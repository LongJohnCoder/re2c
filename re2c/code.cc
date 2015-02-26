/* $Id$ */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>
#include <ctype.h>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "code.h"
#include "globals.h"
#include "dfa.h"
#include "indent.h"
#include "input_api.h"
#include "parser.h"
#include "print.h"
#include "substr.h"

namespace re2c
{

// there must be at least one span in list;  all spans must cover
// same range

template<typename _Ty>
std::string replaceParam(std::string str, const std::string& param, const _Ty& value)
{
	std::ostringstream strValue;

	strValue << value;

	std::string::size_type pos;

	while((pos = str.find(param)) != std::string::npos)
	{
		str.replace(pos, param.length(), strValue.str());
	}

	return str;
}

static void genYYFill(OutputFile & o, uint, uint need)
{
	if (bUseYYFillParam)
	{
		o << mapCodeName["YYFILL"];
		if (!bUseYYFillNaked)
		{
			o << "(" << need << ");";
		}
		o << "\n";
	}
	else
	{
		o << replaceParam(mapCodeName["YYFILL"], yyFillLength, need);
		if (!bUseYYFillNaked)
		{
			o << ";";
		}
		o << "\n";
	}
}

static std::string genGetState()
{
	if (bUseYYGetStateNaked)
	{
		return mapCodeName["YYGETSTATE"];
	}
	else
	{
		return mapCodeName["YYGETSTATE"] + "()";
	}
}

static std::string genGetCondition()
{
	if (bUseYYGetConditionNaked)
	{
		return mapCodeName["YYGETCONDITION"];
	}
	else
	{
		return mapCodeName["YYGETCONDITION"] + "()";
	}
}

static void genSetCondition(OutputFile & o, uint ind, const std::string& newcond)
{
	if (bUseYYSetConditionParam)
	{
		o << indent(ind) << mapCodeName["YYSETCONDITION"] << "(" << condEnumPrefix << newcond << ");\n";
	}
	else
	{
		o << indent(ind) << replaceParam(mapCodeName["YYSETCONDITION"], yySetConditionParam, condEnumPrefix + newcond) << "\n";
	}
}

static std::string space(uint this_label)
{
	int nl = next_label > 999999 ? 6 : next_label > 99999 ? 5 : next_label > 9999 ? 4 : next_label > 999 ? 3 : next_label > 99 ? 2 : next_label > 9 ? 1 : 0;
	int tl = this_label > 999999 ? 6 : this_label > 99999 ? 5 : this_label > 9999 ? 4 : this_label > 999 ? 3 : this_label > 99 ? 2 : this_label > 9 ? 1 : 0;

	return std::string(std::max(1, nl - tl + 1), ' ');
}

void Go::compact()
{
	// arrange so that adjacent spans have different targets
	uint i = 0;

	for (uint j = 1; j < nSpans; ++j)
	{
		if (span[j].to != span[i].to)
		{
			++i;
			span[i].to = span[j].to;
		}

		span[i].ub = span[j].ub;
	}

	nSpans = i + 1;
}

/*
 * Find all spans, that map to the given state. For each of them,
 * find upper adjacent span, that maps to another state (if such
 * span exists, otherwize try lower one).
 * If input contains single span that maps to the given state,
 * then output contains 0 spans.
 */
void Go::unmap(Go *base, const State *x)
{
	nSpans = 0;
	for (uint i = 0; i < base->nSpans; ++i)
	{
		if (base->span[i].to != x)
		{
			if (nSpans > 0 && span[nSpans - 1].to == base->span[i].to)
				span[nSpans - 1].ub = base->span[i].ub;
			else
			{
				span[nSpans].to = base->span[i].to;
				span[nSpans].ub = base->span[i].ub;
				++nSpans;
			}
		}
	}
	if (nSpans > 0)
		span[nSpans - 1].ub = base->span[base->nSpans - 1].ub;
}

static void doGen(const Go *g, const State *s, uint *bm, uint f, uint m)
{
	Span *b = g->span, *e = &b[g->nSpans];
	uint lb = 0;

	for (; b < e; ++b)
	{
		if (b->to == s)
		{
			for (; lb < b->ub && lb < 256; ++lb)
			{
				bm[lb-f] |= m;
			}
		}

		lb = b->ub;
	}
}

static bool matches(const Go *g1, const State *s1, const Go *g2, const State *s2)
{
	Span *b1 = g1->span, *e1 = &b1[g1->nSpans];
	uint lb1 = 0;
	Span *b2 = g2->span, *e2 = &b2[g2->nSpans];
	uint lb2 = 0;

	for (;;)
	{
		for (; b1 < e1 && b1->to != s1; ++b1)
		{
			lb1 = b1->ub;
		}

		for (; b2 < e2 && b2->to != s2; ++b2)
		{
			lb2 = b2->ub;
		}

		if (b1 == e1)
		{
			return b2 == e2;
		}

		if (b2 == e2)
		{
			return false;
		}

		if (lb1 != lb2 || b1->ub != b2->ub)
		{
			return false;
		}

		++b1;
		++b2;
	}
}

BitMap *BitMap::first = NULL;

BitMap::BitMap(const Go *g, const State *x)
	: go(g)
	, on(x)
	, next(first)
	, i(0)
	, m(0)
{
	first = this;
}

BitMap::~BitMap()
{
	delete next;
}

const BitMap *BitMap::find(const Go *g, const State *x)
{
	for (const BitMap *b = first; b; b = b->next)
	{
		if (matches(b->go, b->on, g, x))
		{
			return b;
		}
	}

	return new BitMap(g, x);
}

const BitMap *BitMap::find(const State *x)
{
	for (const BitMap *b = first; b; b = b->next)
	{
		if (b->on == x)
		{
			return b;
		}
	}

	return NULL;
}

void BitMap::gen(OutputFile & o, uint ind, uint lb, uint ub)
{
	if (first && bUsedYYBitmap)
	{
		o << indent(ind) << "static const unsigned char " << mapCodeName["yybm"] << "[] = {";

		uint c = 1, n = ub - lb;
		const BitMap *cb = first;

		while((cb = cb->next) != NULL) {
			++c;
		}
		BitMap *b = first;

		uint *bm = new uint[n];
		
		for (uint i = 0, t = 1; b; i += n, t += 8)
		{
			memset(bm, 0, n * sizeof(uint));

			for (uint m = 0x80; b && m; m >>= 1)
			{
				b->i = i;
				b->m = m;
				doGen(b->go, b->on, bm, lb, m);
				b = const_cast<BitMap*>(b->next);
			}

			if (c > 8)
			{
				o << "\n" << indent(ind+1) << "/* table " << t << " .. " << std::min(c, t+7) << ": " << i << " */";
			}

			for (uint j = 0; j < n; ++j)
			{
				if (j % 8 == 0)
				{
					o << "\n" << indent(ind+1);
				}

				if (yybmHexTable)
				{
					prtHex(o.fragment (), bm[j]);
				}
				else
				{
					o << Setw (3) << (uint)bm[j];
				}
				o  << ", ";
			}
		}

		o << "\n" << indent(ind) << "};\n";
		
		delete[] bm;
	}
}

static void genGoTo(OutputFile & o, uint ind, const State *from, const State *to, bool & readCh)
{
	if (DFlag)
	{
		o << from->label << " -> " << to->label << "\n";
		return;
	}

	if (readCh && from->label + 1 != to->label)
	{
		o << input_api.stmt_peek (ind);
		readCh = false;
	}

	o << indent(ind) << "goto " << labelPrefix << to->label << ";\n";
	vUsedLabels.insert(to->label);
}

static void genIf(OutputFile & o, uint ind, const char *cmp, uint v, bool &readCh)
{
	o << indent(ind) << "if (";
	if (readCh)
	{
		o << "(" << input_api.expr_peek_save () << ")";
		readCh = false;
	}
	else
	{
		o << mapCodeName["yych"];
	}

	o << " " << cmp << " ";
	prtChOrHex(o.fragment (), v);
	o << ") ";
}

static void need(OutputFile & o, uint ind, uint n, bool & readCh, bool bSetMarker)
{
	if (DFlag)
	{
		return;
	}

	uint fillIndex = next_fill_index;

	if (fFlag)
	{
		next_fill_index++;
		if (bUseYYSetStateParam)
		{
			o << indent(ind) << mapCodeName["YYSETSTATE"] << "(" << fillIndex << ");\n";
		}
		else
		{
			o << indent(ind) << replaceParam(mapCodeName["YYSETSTATE"], yySetStateParam, fillIndex) << "\n";
		}
	}

	if (bUseYYFill && n > 0)
	{
		o << indent(ind);
		if (n == 1)
		{
			if (bUseYYFillCheck)
			{
				o << "if (" << input_api.expr_lessthan_one () << ") ";
			}
			genYYFill(o, ind, n);
		}
		else
		{
			if (bUseYYFillCheck)
			{
				o << "if (" << input_api.expr_lessthan (n) << ") ";
			}
			genYYFill(o, ind, n);
		}
	}

	if (fFlag)
	{
		o << mapCodeName["yyFillLabel"] << fillIndex << ":\n";
	}

	if (n > 0)
	{
		if (bSetMarker)
		{
			o << input_api.stmt_backup_peek (ind);
		}
		else
		{
			o << input_api.stmt_peek (ind);
		}
		readCh = false;
	}
}

void Match::emit(Output & output, uint ind, bool &readCh, const std::string&) const
{
	OutputFile & o = output.source;

	if (DFlag)
	{
		return;
	}

	if (state->link)
	{
		o << input_api.stmt_skip (ind);
	}
	else if (!readAhead())
	{
		/* do not read next char if match */
		o << input_api.stmt_skip (ind);
		readCh = true;
	}
	else
	{
		o << input_api.stmt_skip_peek (ind);
		readCh = false;
	}

	if (state->link)
	{
		need(o, ind, state->depth, readCh, false);
	}
}

void Enter::emit(Output & output, uint ind, bool &readCh, const std::string&) const
{
	OutputFile & o = output.source;

	if (state->link)
	{
		o << input_api.stmt_skip (ind);
		if (vUsedLabels.count(label))
		{
			o << labelPrefix << label << ":\n";
		}
		need(o, ind, state->depth, readCh, false);
	}
	else
	{
		/* we shouldn't need 'rule-following' protection here */
		o << input_api.stmt_skip_peek (ind);
		if (vUsedLabels.count(label))
		{
			o << labelPrefix << label << ":\n";
		}
		readCh = false;
	}
}

void Initial::emit(Output & output, uint ind, bool &readCh, const std::string&) const
{
	OutputFile & o = output.source;

	if (!cFlag && !startLabelName.empty())
	{
		o << startLabelName << ":\n";
	}

	if (vUsedLabels.count(label+1))
	{
		if (state->link)
		{
			o << input_api.stmt_skip (ind);
		}
		else
		{
			o << input_api.stmt_skip_peek (ind);
		}
	}

	if (vUsedLabels.count(label))
	{
		o << labelPrefix << label << ":\n";
	}
	else if (!label)
	{
		o << "\n";
	}

	if (dFlag)
	{
		o << indent(ind) << mapCodeName["YYDEBUG"] << "(" << label << ", *" << mapCodeName["YYCURSOR"] << ");" << "\n";
	}

	if (state->link)
	{
		need(o, ind, state->depth, readCh, setMarker && bUsedYYMarker);
	}
	else
	{
		if (setMarker && bUsedYYMarker)
		{
			o << input_api.stmt_backup (ind);
		}
		readCh = false;
	}
}

void Save::emit(Output & output, uint ind, bool &readCh, const std::string&) const
{
	OutputFile & o = output.source;

	if (DFlag)
	{
		return;
	}

	o.insert_yyaccept_selector (ind, selector);

	if (state->link)
	{
		if (bUsedYYMarker)
		{
			o << input_api.stmt_skip_backup (ind);
		}
		need(o, ind, state->depth, readCh, false);
	}
	else
	{
		if (bUsedYYMarker)
		{
			o << input_api.stmt_skip_backup_peek (ind);
		}
		else
		{
			o << input_api.stmt_skip_peek (ind);
		}
		readCh = false;
	}
}

Move::Move(State *s) : Action(s)
{
	;
}

void Move::emit(Output &, uint, bool &, const std::string&) const
{
	;
}

Accept::Accept(State *x, uint n, uint *s, State **r)
		: Action(x), nRules(n), saves(s), rules(r)
{
	;
}

void Accept::genRuleMap()
{
	for (uint i = 0; i < nRules; ++i)
	{
		if (saves[i] != ~0u)
		{
			mapRules[saves[i]] = rules[i];
		}
	}
}

void Accept::emitBinary(OutputFile & o, uint ind, uint l, uint r, bool &readCh) const
{
	if (l < r)
	{
		uint m = (l + r) >> 1;

		o << indent(ind) << "if (" << mapCodeName["yyaccept"] << (r == l+1 ? " == " : " <= ") << m << ") {\n";
		emitBinary(o, ++ind, l, m, readCh);
		o << indent(--ind) << "} else {\n";
		emitBinary(o, ++ind, m + 1, r, readCh);
		o << indent(--ind) << "}\n";
	}
	else
	{
		genGoTo(o, ind, state, mapRules.find(l)->second, readCh);
	}
}

void Accept::emit(Output & output, uint ind, bool &readCh, const std::string &) const
{
	OutputFile & o = output.source;

	if (mapRules.size() > 0)
	{
		bUsedYYMarker = true;
		if (!DFlag)
		{
			o << input_api.stmt_restore (ind);
		}

		if (readCh) // shouldn't be necessary, but might become at some point
		{
			o << input_api.stmt_peek (ind);
			readCh = false;
		}

		if (mapRules.size() > 1)
		{
			o.set_used_yyaccept ();

			if (gFlag && mapRules.size() >= cGotoThreshold)
			{
				o << indent(ind++) << "{\n";
				o << indent(ind++) << "static void *" << mapCodeName["yytarget"] << "[" << mapRules.size() << "] = {\n";
				for (RuleMap::const_iterator it = mapRules.begin(); it != mapRules.end(); ++it)
				{
					o << indent(ind) << "&&" << labelPrefix << it->second->label << ",\n";
					vUsedLabels.insert(it->second->label);
				}
				o << indent(--ind) << "};\n";
				o << indent(ind) << "goto *" << mapCodeName["yytarget"] << "[" << mapCodeName["yyaccept"] << "];\n";
				o << indent(--ind) << "}\n";
			}
			else if (sFlag || (mapRules.size() == 2 && !DFlag))
			{
				emitBinary(o, ind, 0, mapRules.size() - 1, readCh);
			}
			else if (DFlag)
			{
				for (RuleMap::const_iterator it = mapRules.begin(); it != mapRules.end(); ++it)
				{
					o << state->label << " -> " << it->second->label;
					o << " [label=\"yyaccept=" << it->first << "\"]\n";
				}
			}
			else
			{
				o << indent(ind) << "switch (" << mapCodeName["yyaccept"] << ") {\n";

				RuleMap::const_iterator it = mapRules.begin(), end = mapRules.end();
		
				while (it != end)
				{
					RuleMap::const_iterator tmp = it;

					if (++it == end)
					{
						o << indent(ind) << "default:\t";
					}
					else
					{
						o << indent(ind) << "case " << tmp->first << ": \t";
					}

					genGoTo(o, 0, state, tmp->second, readCh);
				}
			
				o << indent(ind) << "}\n";
			}
		}
		else
		{
			// no need to write if statement here since there is only case 0.
			genGoTo(o, ind, state, mapRules.find(0)->second, readCh);
		}
	}
}

Rule::Rule(State *s, RuleOp *r) : Action(s), rule(r)
{
	;
}

void Rule::emit(Output & output, uint ind, bool &, const std::string& condName) const
{
	OutputFile & o = output.source;

	if (DFlag)
	{
		o << state->label << " [label=\"" << sourceFileInfo.fname << ":" << rule->code->line << "\"]\n";
		return;
	}

	uint back = rule->ctx->fixedLength();

	if (back != 0u)
	{
		o << input_api.stmt_restorectx (ind);
	}

	if (rule->code->newcond.length() && condName != rule->code->newcond)
	{
		genSetCondition(o, ind, rule->code->newcond);
	}

	RuleLine rl(*rule);

	if (!yySetupRule.empty() && !rule->code->autogen)
	{
		o << indent(ind) << yySetupRule << "\n";
	}

	o << file_info(rule->code->source, &rl);
	o << indent(ind);
	if (rule->code->autogen)
	{
		o << replaceParam(condGoto, condGotoParam, condPrefix + rule->code->newcond);
	}
	else
	{
		o << rule->code->text;
	}
	o << "\n";
	o.insert_line_info ();
}

static void doLinear(OutputFile & o, uint ind, Span *s, uint n, const State *from, const State *next, bool &readCh, uint mask)
{
	for (;;)
	{
		State *bg = s[0].to;

		while (n >= 3 && s[2].to == bg && (s[1].ub - s[0].ub) == 1)
		{
			if (s[1].to == next && n == 3)
			{
				if (!mask || (s[0].ub > 0x00FF))
				{
					genIf(o, ind, "!=", s[0].ub, readCh);
					genGoTo(o, 0, from, bg, readCh);
				}
				if (next->label != from->label + 1 || DFlag)
				{
					genGoTo(o, ind, from, next, readCh);
				}
				return ;
			}
			else
			{
				if (!mask || (s[0].ub > 0x00FF))
				{
					genIf(o, ind, "==", s[0].ub, readCh);
					genGoTo(o, 0, from, s[1].to, readCh);
				}
			}

			n -= 2;
			s += 2;
		}

		if (n == 1)
		{
			//	    	if(bg != next){
			if (s[0].to->label != from->label + 1 || DFlag)
			{
				genGoTo(o, ind, from, s[0].to, readCh);
			}
			//	    	}
			return ;
		}
		else if (n == 2 && bg == next)
		{
			if (!mask || (s[0].ub > 0x00FF))
			{
				genIf(o, ind, ">=", s[0].ub, readCh);
				genGoTo(o, 0, from, s[1].to, readCh);
			}
			if (next->label != from->label + 1 || DFlag)
			{
				genGoTo(o, ind, from, next, readCh);
			}
			return ;
		}
		else
		{
			if (!mask || ((s[0].ub - 1) > 0x00FF))
			{
				genIf(o, ind, "<=", s[0].ub - 1, readCh);
				genGoTo(o, 0, from, bg, readCh);
			}
			n -= 1;
			s += 1;
		}
	}

	if (next->label != from->label + 1 || DFlag)
	{
		genGoTo(o, ind, from, next, readCh);
	}
}

void Go::genLinear(OutputFile & o, uint ind, const State *from, const State *next, bool &readCh, uint mask) const
{
	doLinear(o, ind, span, nSpans, from, next, readCh, mask);
}

static void printDotCharInterval(OutputFile & o, uint lastPrintableChar, uint chr, const State *from, const State *to, bool multipleIntervals)
{
	o << from->label << " -> " << to->label;
	o << " [label=";

	if (lastPrintableChar != 0)
	{
		--chr; // we are already one char past the end

		// make an interval (e.g. [A-Z])
		if (lastPrintableChar != chr)
		{
			o << "\"[" << (char)lastPrintableChar << "-" << (char)chr << "]\"";

			if (multipleIntervals)
			{
				o << "]\n";
				o << from->label << " -> " << to->label;
				o << " [label=";
				prtChOrHex(o.fragment (), ++chr);
			}
		}
		else
		{
			prtChOrHex(o.fragment (), chr);
		}
	}
	else
	{
		prtChOrHex(o.fragment (), chr);
	}

	o << "]";
}

static bool genCases(OutputFile & o, uint ind, uint lb, Span *s, bool &newLine, uint mask, const State *from, const State *to)
{
	bool used = false;
	uint lastPrintableChar = 0;

	if (!newLine)
	{
		o << "\n";
	}
	newLine = true;
	if (lb < s->ub)
	{
		for (;;)
		{
			if (!mask || lb > 0x00FF)
			{
				if (DFlag)
				{
					if ((lb >= 'A' && lb <= 'Z') || (lb >= 'a' && lb <= 'z') || (lb >= '0' && lb <= '9'))
					{
						if (lastPrintableChar == 0)
						{
							lastPrintableChar = lb;
						}

						if (++lb == s->ub)
						{
							break;
						}
						continue;
					}

					printDotCharInterval(o, lastPrintableChar, lb, from, to, true);
					lastPrintableChar = 0;
				}
				else
				{
					o << indent(ind) << "case ";
					prtChOrHex(o.fragment (), lb);
					o << ":";
					if (dFlag && encoding.is(Enc::EBCDIC))
					{
						const uint c = encoding.decodeUnsafe(lb);
						if (isprint(c))
							o << " /* " << std::string(1, c) << " */";
					}
				}
				newLine = false;
				used = true;
			}

			if (++lb == s->ub)
			{
				break;
			}

			o << "\n";
			newLine = true;
		}
	}

	if (lastPrintableChar != 0)
	{
		printDotCharInterval(o, lastPrintableChar, lb, from, to, false);

		o << "\n";
		newLine = true;
	}

	return used;
}

void Go::genSwitch(OutputFile & o, uint ind, const State *from, const State *next, bool &readCh, uint mask) const
{
	bool newLine = true;

	if ((mask ? wSpans : nSpans) <= 2)
	{
		genLinear(o, ind, from, next, readCh, mask);
	}
	else
	{
		State *def = span[nSpans - 1].to;
		Span **sP = new Span * [nSpans - 1], **r, **s, **t;

		t = &sP[0];

		for (uint i = 0; i < nSpans; ++i)
		{
			if (span[i].to != def)
			{
				*(t++) = &span[i];
			}
		}

		if (!DFlag)
		{
			if (dFlag)
			{
				o << indent(ind) << mapCodeName["YYDEBUG"] << "(-1, " << mapCodeName["yych"] << ");\n";
			}

			if (readCh)
			{
				o << indent(ind) << "switch ((" << input_api.expr_peek_save () << ")) {\n";
				readCh = false;
			}
			else
			{
				o << indent(ind) << "switch (" << mapCodeName["yych"] << ") {\n";
			}
		}

		while (t != &sP[0])
		{
			bool used = false;

			r = s = &sP[0];

			const State *to = (*s)->to;

			if (*s == &span[0])
			{
				used |= genCases(o, ind, 0, *s, newLine, mask, from, to);
			}
			else
			{
				used |= genCases(o, ind, (*s)[ -1].ub, *s, newLine, mask, from, to);
			}

			while (++s < t)
			{
				if ((*s)->to == to)
				{
					used |= genCases(o, ind, (*s)[ -1].ub, *s, newLine, mask, from, to);
				}
				else
				{
					*(r++) = *s;
				}
			}

			if (used && !DFlag)
			{
				genGoTo(o, newLine ? ind+1 : 1, from, to, readCh);
				newLine = true;
			}
			t = r;
		}

		if (DFlag)
		{
			if (!newLine)
			{
				o << "\n";
				newLine = true;
			}

			o << from->label << " -> " << def->label;
			o << " [label=default]\n" ;
		}
		else
		{
			o << indent(ind) << "default:";
			genGoTo(o, 1, from, def, readCh);
			o << indent(ind) << "}\n";
		}

		delete [] sP;
	}
}

static void doBinary(OutputFile & o, uint ind, Span *s, uint n, const State *from, const State *next, bool &readCh, uint mask)
{
	if (n <= 4)
	{
		doLinear(o, ind, s, n, from, next, readCh, mask);
	}
	else
	{
		uint h = n / 2;

		genIf(o, ind, "<=", s[h - 1].ub - 1, readCh);
		o << "{\n";
		doBinary(o, ind+1, &s[0], h, from, next, readCh, mask);
		o << indent(ind) << "} else {\n";
		doBinary(o, ind+1, &s[h], n - h, from, next, readCh, mask);
		o << indent(ind) << "}\n";
	}
}

void Go::genBinary(OutputFile & o, uint ind, const State *from, const State *next, bool &readCh, uint mask) const
{
	if (mask)
	{
		Span * sc = new Span[wSpans];
		
		for (uint i = 0, j = 0; i < nSpans; i++)
		{
			if (span[i].ub > 0xFF)
			{
				sc[j++] = span[i];
			}
		}

		doBinary(o, ind, sc, wSpans, from, next, readCh, mask);

		delete[] sc;
	}
	else
	{
		doBinary(o, ind, span, nSpans, from, next, readCh, mask);
	}
}

void Go::genBase(OutputFile & o, uint ind, const State *from, const State *next, bool &readCh, uint mask) const
{
	if ((mask ? wSpans : nSpans) == 0)
	{
		return ;
	}

	if (!sFlag)
	{
		genSwitch(o, ind, from, next, readCh, mask);
		return ;
	}

	if ((mask ? wSpans : nSpans) > 8)
	{
		Span *bot = &span[0], *top = &span[nSpans - 1];
		uint util;

		if (bot[0].to == top[0].to)
		{
			util = (top[ -1].ub - bot[0].ub) / (nSpans - 2);
		}
		else
		{
			if (bot[0].ub > (top[0].ub - top[ -1].ub))
			{
				util = (top[0].ub - bot[0].ub) / (nSpans - 1);
			}
			else
			{
				util = top[ -1].ub / (nSpans - 1);
			}
		}

		if (util <= 2)
		{
			genSwitch(o, ind, from, next, readCh, mask);
			return ;
		}
	}

	if ((mask ? wSpans : nSpans) > 5)
	{
		genBinary(o, ind, from, next, readCh, mask);
	}
	else
	{
		genLinear(o, ind, from, next, readCh, mask);
	}
}

void Go::genCpGoto(OutputFile & o, uint ind, const State *from, const State *next, bool &readCh) const
{
	std::string sYych;
	
	if (readCh)
	{
		sYych = "(" + input_api.expr_peek_save () + ")";
	}
	else
	{
		sYych = mapCodeName["yych"];
	}

	readCh = false;
	if (encoding.szCodeUnit() > 1)
	{
		o << indent(ind) << "if (" << sYych <<" & ~0xFF) {\n";
		genBase(o, ind+1, from, next, readCh, 1);
		o << indent(ind++) << "} else {\n";
		sYych = mapCodeName["yych"];
	}
	else
	{
		o << indent(ind++) << "{\n";
	}
	o << indent(ind++) << "static void *" << mapCodeName["yytarget"] << "[256] = {\n";
	o << indent(ind);

	uint ch = 0;
	for (uint i = 0; i < lSpans; ++i)
	{
		vUsedLabels.insert(span[i].to->label);
		for(; ch < span[i].ub; ++ch)
		{
			o << "&&" << labelPrefix << span[i].to->label;
			if (ch == 255)
			{
				o << "\n";
				i = lSpans;
				break;
			}
			else if (ch % 8 == 7)
			{
				o << ",\n" << indent(ind);
			}
			else
			{
				o << "," << space(span[i].to->label);
			}
		}
	}
	o << indent(--ind) << "};\n";
	o << indent(ind) << "goto *" << mapCodeName["yytarget"] << "[" << sYych << "];\n";
	o << indent(--ind) << "}\n";
}

void Go::genGoto(OutputFile & o, uint ind, const State *from, const State *next, bool &readCh)
{
	if ((gFlag || (encoding.szCodeUnit() > 1)) && wSpans == ~0u)
	{
		uint nBitmaps = 0;
		std::set<uint> vTargets;
		wSpans = 0;
		lSpans = 1;
		dSpans = 0;
		for (uint i = 0; i < nSpans; ++i)
		{
			if (span[i].ub > 0xFF)
			{
				wSpans++;
			}
			if (span[i].ub < 0x100 || (encoding.szCodeUnit() <= 1))
			{
				lSpans++;

				State *to = span[i].to;
	
				if (to && to->isBase)
				{
					const BitMap *b = BitMap::find(to);
	
					if (b && matches(b->go, b->on, this, to))
					{
						nBitmaps++;
					}
					else
					{
						dSpans++;
						vTargets.insert(to->label);
					}
				}
				else
				{
					dSpans++;
					vTargets.insert(to->label);
				}
			}
		}
		lTargets = vTargets.size() >> nBitmaps;
	}

	if (gFlag && (lTargets >= cGotoThreshold || dSpans >= cGotoThreshold))
	{
		genCpGoto(o, ind, from, next, readCh);
		return;
	}
	else if (bFlag)
	{
		for (uint i = 0; i < nSpans; ++i)
		{
			State *to = span[i].to;

			if (to && to->isBase)
			{
				const BitMap *b = BitMap::find(to);
				std::string sYych;

				if (b && matches(b->go, b->on, this, to))
				{
					Go go;
					go.span = new Span[nSpans];
					go.unmap(this, to);
					if (readCh)
					{
						sYych = "(" + input_api.expr_peek_save () + ")";
					}
					else
					{
						sYych = mapCodeName["yych"];
					}
					readCh = false;
					if (encoding.szCodeUnit() > 1)
					{
						o << indent(ind) << "if (" << sYych << " & ~0xFF) {\n";
						sYych = mapCodeName["yych"];
						genBase(o, ind+1, from, next, readCh, 1);
						o << indent(ind) << "} else ";
					}
					else
					{
						o << indent(ind);
					}
					bUsedYYBitmap = true;
					o << "if (" << mapCodeName["yybm"] << "[" << b->i << "+" << sYych << "] & ";
					if (yybmHexTable)
					{
						prtHex(o.fragment (), b->m);
					}
					else
					{
						o << (uint) b->m;
					}
					o << ") {\n";
					genGoTo(o, ind+1, from, to, readCh);
					o << indent(ind) << "}\n";
					go.genBase(o, ind, from, next, readCh, 0);
					delete [] go.span;
					return ;
				}
			}
		}
	}

	genBase(o, ind, from, next, readCh, 0);
}

void State::emit(Output & output, uint ind, bool &readCh, const std::string& condName) const
{
	OutputFile & o = output.source;

	if (vUsedLabels.count(label))
	{
		o << labelPrefix << label << ":\n";
	}
	if (dFlag && !action->isInitial())
	{
		o << indent(ind) << mapCodeName["YYDEBUG"] << "(" << label << ", " << input_api.expr_peek () << ");\n";
	}
	if (isPreCtxt)
	{
		o << input_api.stmt_backupctx (ind);
	}
	action->emit(output, ind, readCh, condName);
}

static uint merge(Span *x0, State *fg, State *bg)
{
	Span *x = x0, *f = fg->go.span, *b = bg->go.span;
	uint nf = fg->go.nSpans, nb = bg->go.nSpans;
	State *prev = NULL, *to;
	// NB: we assume both spans are for same range

	for (;;)
	{
		if (f->ub == b->ub)
		{
			to = f->to == b->to ? bg : f->to;

			if (to == prev)
			{
				--x;
			}
			else
			{
				x->to = prev = to;
			}

			x->ub = f->ub;
			++x;
			++f;
			--nf;
			++b;
			--nb;

			if (nf == 0 && nb == 0)
			{
				return x - x0;
			}
		}

		while (f->ub < b->ub)
		{
			to = f->to == b->to ? bg : f->to;

			if (to == prev)
			{
				--x;
			}
			else
			{
				x->to = prev = to;
			}

			x->ub = f->ub;
			++x;
			++f;
			--nf;
		}

		while (b->ub < f->ub)
		{
			to = b->to == f->to ? bg : f->to;

			if (to == prev)
			{
				--x;
			}
			else
			{
				x->to = prev = to;
			}

			x->ub = b->ub;
			++x;
			++b;
			--nb;
		}
	}
}

static const uint cInfinity = ~0u;

class SCC
{

public:
	State	**top, **stk;

public:
	SCC(uint);
	~SCC();
	void traverse(State*);

#ifdef PEDANTIC
private:
	SCC(const SCC& oth)
		: top(oth.top)
		, stk(oth.stk)
	{
	}
	SCC& operator = (const SCC& oth)
	{
		new(this) SCC(oth);
		return *this;
	}
#endif
};

SCC::SCC(uint size)
	: top(new State * [size])
	, stk(top)
{
}

SCC::~SCC()
{
	delete [] stk;
}

void SCC::traverse(State *x)
{
	*top = x;
	uint k = ++top - stk;
	x->depth = k;

	for (uint i = 0; i < x->go.nSpans; ++i)
	{
		State *y = x->go.span[i].to;

		if (y)
		{
			if (y->depth == 0)
			{
				traverse(y);
			}

			if (y->depth < x->depth)
			{
				x->depth = y->depth;
			}
		}
	}

	if (x->depth == k)
	{
		do
		{
			(*--top)->depth = cInfinity;
			(*top)->link = x;
		}
		while (*top != x);
	}
}

static bool state_is_in_non_trivial_SCC(const State* s)
{
	
	// does not link to self
	if (s->link != s)
	{
		return true;
	}
	
	// or exists i: (s->go.spans[i].to->link == s)
	//
	// Note: (s->go.spans[i].to == s) is allowed, corresponds to s
	// looping back to itself.
	//
	for (uint i = 0; i < s->go.nSpans; ++i)
	{
		const State* t = s->go.span[i].to;
	
		if (t && t->link == s)
		{
			return true;
		}
	}
	// otherwise no
	return false;
}

static uint maxDist(State *s)
{
	if (s->depth != cInfinity)
	{
		// Already calculated, just return result.
    	return s->depth;
	}
	uint mm = 0;

	for (uint i = 0; i < s->go.nSpans; ++i)
	{
		State *t = s->go.span[i].to;

		if (t)
		{
			uint m = 1;

			if (!t->link) // marked as non-key state
			{
				if (t->depth == cInfinity)
				{
					t->depth = maxDist(t);
				}
				m += t->depth;
			}

			if (m > mm)
			{
				mm = m;
			}
		}
	}

	s->depth = mm;
	return mm;
}

static void calcDepth(State *head)
{
	State* s;

	// mark non-key states by s->link = NULL ;
	for (s = head; s; s = s->next)
	{
		if (s != head && !state_is_in_non_trivial_SCC(s))
		{
			s->link = NULL;
		}
		//else: key state, leave alone
	}
	
	for (s = head; s; s = s->next)
	{
		s->depth = cInfinity;
	}

	// calculate max number of transitions before guarantied to reach
	// a key state.
	for (s = head; s; s = s->next)
	{
		maxDist(s);
	}
}

void DFA::findSCCs()
{
	SCC scc(nStates);
	State *s;

	for (s = head; s; s = s->next)
	{
		s->depth = 0;
		s->link = NULL;
	}

	for (s = head; s; s = s->next)
	{
		if (!s->depth)
		{
			scc.traverse(s);
		}
	}

	calcDepth(head);
}

void DFA::split(State *s)
{
	State *move = new State;
	(void) new Move(move);
	addState(&s->next, move);
	move->link = s->link;
	move->rule = s->rule;
	move->go = s->go;
	s->rule = NULL;
	s->go.nSpans = 1;
	s->go.span = new Span[1];
	s->go.span[0].ub = ubChar;
	s->go.span[0].to = move;
}

void DFA::findBaseState()
{
	Span *span = new Span[ubChar - lbChar];

	for (State *s = head; s; s = s->next)
	{
		if (!s->link)
		{
			for (uint i = 0; i < s->go.nSpans; ++i)
			{
				State *to = s->go.span[i].to;

				if (to && to->isBase)
				{
					to = to->go.span[0].to;
					uint nSpans = merge(span, s, to);

					if (nSpans < s->go.nSpans)
					{
						delete [] s->go.span;
						s->go.nSpans = nSpans;
						s->go.span = new Span[nSpans];
						memcpy(s->go.span, span, nSpans*sizeof(Span));
					}

					break;
				}
			}
		}
	}

	delete [] span;
}

void DFA::prepare(uint & max_fill)
{
	State *s;
	uint i;

	bUsedYYBitmap = false;

	findSCCs();
	head->link = head;

	uint nRules = 0;

	for (s = head; s; s = s->next)
	{
		s->depth = maxDist(s);
		if (max_fill < s->depth)
		{
			max_fill = s->depth;
		}
		if (s->rule && s->rule->accept >= nRules)
		{
			nRules = s->rule->accept + 1;
		}
	}

	uint nSaves = 0;
	saves = new uint[nRules];
	memset(saves, ~0, (nRules)*sizeof(*saves));

	// mark backtracking points
	bSaveOnHead = false;

	for (s = head; s; s = s->next)
	{
		if (s->rule)
		{
			for (i = 0; i < s->go.nSpans; ++i)
			{
				if (s->go.span[i].to && !s->go.span[i].to->rule)
				{
					delete s->action;
					s->action = NULL;

					if (saves[s->rule->accept] == ~0u)
					{
						saves[s->rule->accept] = nSaves++;
					}

					bSaveOnHead |= s == head;
					(void) new Save(s, saves[s->rule->accept]); // sets s->action
				}
			}
		}
	}

	// insert actions
	rules = new State * [nRules];

	memset(rules, 0, (nRules)*sizeof(*rules));

	State *accept = NULL;
	Accept *accfixup = NULL;

	for (s = head; s; s = s->next)
	{
		State * ow;

		if (!s->rule)
		{
			ow = accept;
		}
		else
		{
			if (!rules[s->rule->accept])
			{
				State *n = new State;
				(void) new Rule(n, s->rule);
				rules[s->rule->accept] = n;
				addState(&s->next, n);
			}

			ow = rules[s->rule->accept];
		}

		for (i = 0; i < s->go.nSpans; ++i)
		{
			if (!s->go.span[i].to)
			{
				if (!ow)
				{
					ow = accept = new State;
					accfixup = new Accept(accept, nRules, saves, rules);
					addState(&s->next, accept);
				}

				s->go.span[i].to = ow;
			}
		}
	}
	
	if (accfixup)
	{
		accfixup->genRuleMap();
	}

	// split ``base'' states into two parts
	for (s = head; s; s = s->next)
	{
		s->isBase = false;

		if (s->link)
		{
			for (i = 0; i < s->go.nSpans; ++i)
			{
				if (s->go.span[i].to == s)
				{
					s->isBase = true;
					split(s);

					if (bFlag)
					{
						BitMap::find(&s->next->go, s);
					}

					s = s->next;
					break;
				}
			}
		}
	}

	// find ``base'' state, if possible
	findBaseState();

	delete head->action;
	head->action = NULL;
}


void DFA::emit(Output & output, uint& ind, const RegExpMap* specMap, const std::string& condName, bool isLastCond, bool& bPrologBrace)
{
	OutputFile & o = output.source;
	bool bProlog = (!cFlag || !bWroteCondCheck);

	// In -c mode, the prolog needs its own label separate from start_label.
	// prolog_label is before the condition branch (GenCondGoto). It is
	// equivalent to startLabelName.
	// start_label corresponds to current condition.
	// NOTE: prolog_label must be yy0 because of the !getstate:re2c handling
	// in scanner.re
	uint prolog_label = next_label;
	if (bProlog && cFlag)
	{
		next_label++;
	}

	uint start_label = next_label;

	(void) new Initial(head, next_label++, bSaveOnHead);

	if (bUseStartLabel)
	{
		if (startLabelName.empty())
		{
			vUsedLabels.insert(prolog_label);
		}
	}

	State *s;

	for (s = head; s; s = s->next)
	{
		s->label = next_label++;
	}

	// Save 'next_fill_index' and compute information about code generation
	// while writing to null device.
	uint save_fill_index = next_fill_index;
	Output null_dev (NULL, NULL);

	for (s = head; s; s = s->next)
	{
		bool readCh = false;
		s->emit(null_dev, ind, readCh, condName);
		s->go.genGoto(null_dev.source, ind, s, s->next, readCh);
	}
	if (last_fill_index < next_fill_index)
	{
		last_fill_index = next_fill_index;
	}
	next_fill_index = save_fill_index;

	// Generate prolog
	if (bProlog)
	{
		o << "\n";
		o.insert_line_info ();

		if (DFlag)
		{
			bPrologBrace = true;
			o << "digraph re2c {\n";
		}
		else if ((!fFlag && o.get_used_yyaccept ())
		||  (!fFlag && bEmitYYCh)
		||  (bFlag && !cFlag && BitMap::first)
		||  (cFlag && !bWroteCondCheck && gFlag && !specMap->empty())
		||  (fFlag && !bWroteGetState && gFlag)
		)
		{
			bPrologBrace = true;
			o << indent(ind++) << "{\n";
		}
		else if (ind == 0)
		{
			ind = 1;
		}

		if (!fFlag && !DFlag)
		{
			if (bEmitYYCh)
			{
				o << indent(ind) << mapCodeName["YYCTYPE"] << " " << mapCodeName["yych"] << ";\n";
			}
			o.insert_yyaccept_init (ind);
		}
		else
		{
			o << "\n";
		}
	}
	if (bFlag && !cFlag && BitMap::first)
	{
		BitMap::gen(o, ind, lbChar, ubChar <= 256 ? ubChar : 256);
	}
	if (bProlog)
	{
		genCondTable(o, ind, *specMap);
		o.insert_state_goto (ind, prolog_label);
		if (cFlag && !DFlag)
		{
			if (vUsedLabels.count(prolog_label))
			{
				o << labelPrefix << prolog_label << ":\n";
			}
			if (!startLabelName.empty())
			{
				o << startLabelName << ":\n";
			}
		}
		genCondGoto(o, ind, *specMap);
	}

	if (cFlag && !condName.empty())
	{
		if (condDivider.length())
		{
			o << replaceParam(condDivider, condDividerParam, condName) << "\n";
		}

		if (DFlag)
		{
			o << condName << " -> " << (start_label+1) << "\n";
		}
		else
		{
			o << condPrefix << condName << ":\n";
		}
	}
	if (cFlag && bFlag && BitMap::first)
	{
		o << indent(ind++) << "{\n";
		BitMap::gen(o, ind, lbChar, ubChar <= 256 ? ubChar : 256);
	}

	// The start_label is not always the first to be emitted, so we may have to jump. c.f. Initial::emit()
	if (vUsedLabels.count(start_label+1))
	{
		vUsedLabels.insert(start_label);
		o << indent(ind) << "goto " << labelPrefix << start_label << ";\n";
	}

	// Generate code
	for (s = head; s; s = s->next)
	{
		bool readCh = false;
		s->emit(output, ind, readCh, condName);
		s->go.genGoto(o, ind, s, s->next, readCh);
	}

	if (cFlag && bFlag && BitMap::first)
	{
		o << indent(--ind) << "}\n";
	}
	// Generate epilog
	if ((!cFlag || isLastCond) && bPrologBrace)
	{
		o << indent(--ind) << "}\n";
	}

	// Cleanup
	if (BitMap::first)
	{
		delete BitMap::first;
		BitMap::first = NULL;
	}

	bUseStartLabel = false;
}

static void output_state_goto_sub (OutputFragment & o, uint ind, uint start_label, int cMin, int cMax)
{
	if (cMin == cMax)
	{
		if (cMin == -1)
		{
			o << indent(ind) << "goto " << labelPrefix << start_label << ";\n";
		}
		else
		{
			o << indent(ind) << "goto " << mapCodeName["yyFillLabel"] << cMin << ";\n";
		}
	}
	else
	{
		int cMid = cMin + ((cMax - cMin + 1) / 2);

		o << indent(ind) << "if (" << genGetState() << " < " << cMid << ") {\n";
		output_state_goto_sub (o, ind + 1, start_label, cMin, cMid - 1);
		o << indent(ind) << "} else {\n";
		output_state_goto_sub (o, ind + 1, start_label, cMid, cMax);
		o << indent(ind) << "}\n";
	}
}

void output_state_goto (OutputFragment & o, uint start_label)
{
	uint ind = o.indent;
	if (gFlag)
	{
		o << indent(ind++) << "static void *" << mapCodeName["yystable"] << "[" << "] = {\n";

		for (size_t i=0; i<last_fill_index; ++i)
		{
			o << indent(ind) << "&&" << mapCodeName["yyFillLabel"] << i << ",\n";
		}

		o << indent(--ind) << "};\n";
		o << "\n";

		o << indent(ind) << "if (" << genGetState();
		if (bUseStateAbort)
		{
			o << " == -1) {\n";
		}
		else
		{
			o << " < 0) {\n";
		}
		o << indent(++ind) << "goto " << labelPrefix << start_label << ";\n";
		if (bUseStateAbort)
		{
			o << indent(--ind) << "} else if (" << genGetState() << " < -1) {\n";
			o << indent(++ind) << "abort();\n";
		}
		o << indent(--ind) << "}\n";

		o << indent(ind) << "goto *" << mapCodeName["yystable"] << "[" << genGetState() << "];\n";

	}
	else if (bFlag)
	{
		output_state_goto_sub (o, ind, start_label, -1, last_fill_index-1);
		if (bUseStateAbort)
		{
			o << indent(ind) << "abort();\n";
		}
	}
	else
	{
		o << indent(ind) << "switch (" << genGetState() << ") {\n";
		if (bUseStateAbort)
		{
			o << indent(ind) << "default: abort();\n";
			o << indent(ind) << "case -1: goto " << labelPrefix << start_label << ";\n";
		}
		else
		{
			o << indent(ind) << "default: goto " << labelPrefix << start_label << ";\n";
		}

		for (size_t i=0; i<last_fill_index; ++i)
		{
			o << indent(ind) << "case " << i << ": goto " << mapCodeName["yyFillLabel"] << i << ";\n";
		}

		o << indent(ind) << "}\n";
	}
	if (bUseStateNext)
	{
		o << mapCodeName["yyNext"] << ":\n";
	}
}

void genCondTable(OutputFile & o, uint ind, const RegExpMap& specMap)
{
	if (cFlag && !bWroteCondCheck && gFlag && specMap.size())
	{
		RegExpIndices  vCondList(specMap.size());

		for(RegExpMap::const_iterator itSpec = specMap.begin(); itSpec != specMap.end(); ++itSpec)
		{
			vCondList[itSpec->second.first] = itSpec->first;
		}

		o << indent(ind++) << "static void *" << mapCodeName["yyctable"] << "[" << specMap.size() << "] = {\n";

		for(RegExpIndices::const_iterator it = vCondList.begin(); it != vCondList.end(); ++it)
		{
			o << indent(ind) << "&&" << condPrefix << *it << ",\n";
		}
		o << indent(--ind) << "};\n";
	}
}

static void genCondGotoSub(OutputFile & o, uint ind, RegExpIndices& vCondList, uint cMin, uint cMax)
{
	if (cMin == cMax)
	{
		o << indent(ind) << "goto " << condPrefix << vCondList[cMin] << ";\n";
	}
	else
	{
		uint cMid = cMin + ((cMax - cMin + 1) / 2);

		o << indent(ind) << "if (" << genGetCondition() << " < " << cMid << ") {\n";
		genCondGotoSub(o, ind + 1, vCondList, cMin, cMid - 1);
		o << indent(ind) << "} else {\n";
		genCondGotoSub(o, ind + 1, vCondList, cMid, cMax);
		o << indent(ind) << "}\n";
	}
}

void genCondGoto(OutputFile & o, uint ind, const RegExpMap& specMap)
{
	if (cFlag && !bWroteCondCheck && specMap.size())
	{
		if (gFlag)
		{
			o << indent(ind) << "goto *" << mapCodeName["yyctable"] << "[" << genGetCondition() << "];\n";
		}
		else
		{
			if (sFlag)
			{
				RegExpIndices  vCondList(specMap.size());
			
				for(RegExpMap::const_iterator it = specMap.begin(); it != specMap.end(); ++it)
				{
					vCondList[it->second.first] = it->first;
				}
				genCondGotoSub(o, ind, vCondList, 0, vCondList.size() - 1);
			}
			else if (DFlag)
			{
				for(RegExpMap::const_iterator it = specMap.begin(); it != specMap.end(); ++it)
				{
					o << "0 -> " << it->first << " [label=\"state=" << it->first << "\"]\n";
				}
			}
			else
			{
				o << indent(ind) << "switch (" << genGetCondition() << ") {\n";
	
				for(RegExpMap::const_iterator it = specMap.begin(); it != specMap.end(); ++it)
				{
					o << indent(ind) << "case " << condEnumPrefix << it->first << ": goto " << condPrefix << it->first << ";\n";
				}
				o << indent(ind) << "}\n";
			}
		}
		bWroteCondCheck = true;
	}
}

void genTypes(Output & output, const RegExpMap& specMap)
{
	output.types.resize (specMap.size());
	for(RegExpMap::const_iterator itSpecMap = specMap.begin(); itSpecMap != specMap.end(); ++itSpecMap)
	{
		// If an entry is < 0 then we did the 0/empty correction twice.
		assert(itSpecMap->second.first >= 0);
		output.types[itSpecMap->second.first] = itSpecMap->first;
	}
}

void output_yyaccept_init (OutputFragment & o, bool used_yyaccept)
{
	if (used_yyaccept)
	{
		o << indent (o.indent) << "unsigned int " << mapCodeName["yyaccept"] << " = 0;\n";
	}
}

void output_yyaccept_selector (OutputFragment & o, bool used_yyaccept)
{
	if (used_yyaccept)
	{
		o << indent (o.indent) << mapCodeName["yyaccept"] << " = " << o.info.yyaccept_selector << ";\n";
	}
}

void output_yymaxfill (OutputFragment & o, uint max_fill)
{
	o << "#define YYMAXFILL " << max_fill << "\n";
}

void output_line_info (OutputFragment & o, uint line_number, const char * filename)
{
	if (!iFlag)
	{
		o << "#line " << line_number << " \"" << filename << "\"\n";
	}
}

void output_types (OutputFragment & o, const std::vector<std::string> & types)
{
	uint ind = o.indent;
	o << indent (ind++) << "enum " << mapCodeName["YYCONDTYPE"] << " {\n";
	for (unsigned int i = 0; i < types.size (); ++i)
	{
		o << indent (ind) << condEnumPrefix << types[i] << ",\n";
	}
	o << indent (--ind) << "};\n";
}

void output_version_time (OutputFile & o)
{
	o << "/* Generated by re2c " PACKAGE_VERSION;
	if (!bNoGenerationDate)
	{
		o << " on ";
		time_t now = time (NULL);
		o.write (ctime (&now), 24);
	}
	o << "*/" << "\n";
}

void Scanner::config(const Str& cfg, int num)
{
	if (cfg.to_string() == "indent:top")
	{
		if (num < 0)
		{
			fatal("configuration 'indent:top' must be a positive integer");
		}
		topIndent = num;
	}
	else if (cfg.to_string() == "yybm:hex")
	{
		yybmHexTable = num != 0;
	}
	else if (cfg.to_string() == "startlabel")
	{
		bUseStartLabel = num != 0;
		startLabelName = "";
	}
	else if (cfg.to_string() == "state:abort")
	{
		bUseStateAbort = num != 0;
	}
	else if (cfg.to_string() == "state:nextlabel")
	{
		bUseStateNext = num != 0;
	}
	else if (cfg.to_string() == "yyfill:enable")
	{
		bUseYYFill = num != 0;
	}
	else if (cfg.to_string() == "yyfill:parameter")
	{
		bUseYYFillParam = num != 0;
	}
	else if (cfg.to_string() == "yyfill:check")
	{
		bUseYYFillCheck = num != 0;
	}
	else if (cfg.to_string() == "cgoto:threshold")
	{
		cGotoThreshold = num;
	}
	else if (cfg.to_string() == "yych:conversion")
	{
		if (num)
		{
			yychConversion  = "(";
			yychConversion += mapCodeName["YYCTYPE"];
			yychConversion += ")";
		}
		else
		{
			yychConversion  = "";
		}
	}
	else if (cfg.to_string() == "yych:emit")
	{
		bEmitYYCh = num != 0;
	}
	else if (cfg.to_string() == "define:YYFILL:naked")
	{
		bUseYYFillNaked = num != 0;
	}
	else if (cfg.to_string() == "define:YYGETCONDITION:naked")
	{
		bUseYYGetConditionNaked = num != 0;
	}
	else if (cfg.to_string() == "define:YYGETSTATE:naked")
	{
		bUseYYGetStateNaked = num != 0;
	}
	else if (cfg.to_string() == "define:YYSETSTATE:naked")
	{
		bUseYYSetStateNaked = num != 0;
	}
	else if (cfg.to_string() == "flags:e")
	{
		if (num != 0)
		{
			if (!encoding.set(Enc::EBCDIC))
				fatal("Cannot set '-e' switch: please reset '-w', '-x', '-u' and '-8' switches at first.\n");
		}
		else
			encoding.unset(Enc::EBCDIC);
	}
	else if (cfg.to_string() == "flags:u")
	{
		if (num != 0)
		{
			if (!encoding.set(Enc::UTF32))
				fatal("Cannot set '-u' switch: please reset '-e', '-w', '-x' and '-8' switches at first.\n");
		}
		else
			encoding.unset(Enc::UTF32);
	}
	else if (cfg.to_string() == "flags:w")
	{
		if (num != 0)
		{
			if (!encoding.set(Enc::UCS2))
				fatal("Cannot set '-w' switch: please reset '-e', '-x', '-u' and '-8' switches at first.\n");
		}
		else
			encoding.unset(Enc::UCS2);
	}
	else if (cfg.to_string() == "flags:x")
	{
		if (num != 0)
		{
			if (!encoding.set(Enc::UTF16))
				fatal("Cannot set '-x' switch: please reset '-e', '-x', '-u' and '-8' switches at first.\n");
		}
		else
			encoding.unset(Enc::UTF16);
	}
	else if (cfg.to_string() == "flags:8")
	{
		if (num != 0)
		{
			if (!encoding.set(Enc::UTF8))
				fatal("Cannot set '-8' switch: please reset '-e', '-w', '-x' and '-u' switches at first.\n");
		}
		else
			encoding.unset(Enc::UTF8);
	}
	else
	{
		fatalf("unrecognized configuration name '%s' or illegal integer value", cfg.to_string().c_str());
	}
}

static std::set<std::string> mapVariableKeys;
static std::set<std::string> mapDefineKeys;
static std::set<std::string> mapLabelKeys;

void Scanner::config(const Str& cfg, const Str& val)
{
	if (mapDefineKeys.empty())
	{
		mapVariableKeys.insert("variable:yyaccept");
		mapVariableKeys.insert("variable:yybm");
		mapVariableKeys.insert("variable:yych");
		mapVariableKeys.insert("variable:yyctable");
		mapVariableKeys.insert("variable:yystable");
		mapVariableKeys.insert("variable:yytarget");
		mapDefineKeys.insert("define:YYBACKUP");
		mapDefineKeys.insert("define:YYBACKUPCTX");
		mapDefineKeys.insert("define:YYCONDTYPE");
		mapDefineKeys.insert("define:YYCTXMARKER");
		mapDefineKeys.insert("define:YYCTYPE");
		mapDefineKeys.insert("define:YYCURSOR");
		mapDefineKeys.insert("define:YYDEBUG");
		mapDefineKeys.insert("define:YYFILL");
		mapDefineKeys.insert("define:YYGETCONDITION");
		mapDefineKeys.insert("define:YYGETSTATE");
		mapDefineKeys.insert("define:YYLESSTHAN");
		mapDefineKeys.insert("define:YYLIMIT");
		mapDefineKeys.insert("define:YYMARKER");
		mapDefineKeys.insert("define:YYPEEK");
		mapDefineKeys.insert("define:YYRESTORE");
		mapDefineKeys.insert("define:YYRESTORECTX");
		mapDefineKeys.insert("define:YYSETCONDITION");
		mapDefineKeys.insert("define:YYSETSTATE");
		mapDefineKeys.insert("define:YYSKIP");
		mapLabelKeys.insert("label:yyFillLabel");
		mapLabelKeys.insert("label:yyNext");
	}

	std::string strVal;

	if (val.len >= 2 && val.str[0] == val.str[val.len-1] 
	&& (val.str[0] == '"' || val.str[0] == '\''))
	{
		SubStr tmp(val.str + 1, val.len - 2);
		unescape(tmp, strVal);
	}
	else
	{
		strVal = val.to_string();
	}

	if (cfg.to_string() == "indent:string")
	{
		indString = strVal;
	}
	else if (cfg.to_string() == "startlabel")
	{
		startLabelName = strVal;
		bUseStartLabel = !startLabelName.empty();
	}
	else if (cfg.to_string() == "labelprefix")
	{
		labelPrefix = strVal;
	}
	else if (cfg.to_string() == "condprefix")
	{
		condPrefix = strVal;
	}
	else if (cfg.to_string() == "condenumprefix")
	{
		condEnumPrefix = strVal;
	}
	else if (cfg.to_string() == "cond:divider")
	{
		condDivider = strVal;
	}
	else if (cfg.to_string() == "cond:divider@cond")
	{
		condDividerParam = strVal;
	}
	else if (cfg.to_string() == "cond:goto")
	{
		condGoto = strVal;
	}
	else if (cfg.to_string() == "cond:goto@cond")
	{
		condGotoParam = strVal;
	}
	else if (cfg.to_string() == "define:YYFILL@len")
	{
		yyFillLength = strVal;
		bUseYYFillParam = false;
	}
	else if (cfg.to_string() == "define:YYSETCONDITION@cond")
	{
		yySetConditionParam = strVal;
		bUseYYSetConditionParam = false;
	}
	else if (cfg.to_string() == "define:YYSETSTATE@state")
	{
		yySetStateParam = strVal;
		bUseYYSetStateParam = false;
	}
	else if (mapVariableKeys.find(cfg.to_string()) != mapVariableKeys.end())
	{
		if (!mapCodeName.insert(std::make_pair(cfg.to_string().substr(sizeof("variable:") - 1), strVal)).second)
		{
			fatalf("variable '%s' already being used and cannot be changed", cfg.to_string().c_str());
		}
	}
	else if (mapDefineKeys.find(cfg.to_string()) != mapDefineKeys.end())
	{
		if (!mapCodeName.insert(std::make_pair(cfg.to_string().substr(sizeof("define:") - 1), strVal)).second)
		{
			fatalf("define '%s' already being used and cannot be changed", cfg.to_string().c_str());
		}
	}
	else if (mapLabelKeys.find(cfg.to_string()) != mapLabelKeys.end())
	{
		if (!mapCodeName.insert(std::make_pair(cfg.to_string().substr(sizeof("label:") - 1), strVal)).second)
		{
			fatalf("label '%s' already being used and cannot be changed", cfg.to_string().c_str());
		}
	}
	else
	{
		std::string msg = "unrecognized configuration name '";
		msg += cfg.to_string();
		msg += "' or illegal string value";
		fatal(msg.c_str());
	}
}

ScannerState::ScannerState()
	: tok(NULL), ptr(NULL), cur(NULL), pos(NULL), ctx(NULL)
	, bot(NULL), lim(NULL), top(NULL), eof(NULL)
	, tchar(0), tline(0), cline(1), iscfg(0)
	, in_parse(false)
{
}

Scanner::Scanner (FILE * i, OutputFile & o)
	: ScannerState ()
	, in (i)
	, out (o)
{}

char *Scanner::fill(char *cursor, uint need)
{
	if(!eof)
	{
		uint cnt;
		/* Do not get rid of anything when rFlag is active. Otherwise
		 * get rid of everything that was already handedout. */
		if (!rFlag)
		{
			cnt = tok - bot;
			if (cnt)
			{
				memmove(bot, tok, top - tok);
				tok  = bot;
				ptr -= cnt;
				cur -= cnt;
				pos -= cnt;
				lim -= cnt;
				ctx -= cnt;
				cursor -= cnt;
			}
		}
		/* In crease buffer size. */
		if (BSIZE > need)
		{
			need = BSIZE;
		}
		if (static_cast<uint>(top - lim) < need)
		{
			char *buf = new char[(lim - bot) + need];
			if (!buf)
			{
				fatal("Out of memory");
			}
			memcpy(buf, bot, lim - bot);
			tok = &buf[tok - bot];
			ptr = &buf[ptr - bot];
			cur = &buf[cur - bot];
			pos = &buf[pos - bot];
			lim = &buf[lim - bot];
			top = &lim[need];
			ctx = &buf[ctx - bot];
			cursor = &buf[cursor - bot];
			delete [] bot;
			bot = buf;
		}
		/* Append to buffer. */
		cnt = fread (lim, 1, need, in);
		if (cnt != need)
		{
			eof = &lim[cnt];
			*eof++ = '\0';
		}
		lim += cnt;
	}
	return cursor;
}

void Scanner::set_in_parse(bool new_in_parse)
{
	in_parse = new_in_parse;
}

void Scanner::fatal_at(uint line, uint ofs, const char *msg) const
{
	std::cerr << "re2c: error: "
		<< "line " << line << ", column " << (tchar + ofs + 1) << ": "
		<< msg << std::endl;
	exit(1);
}

void Scanner::fatal(uint ofs, const char *msg) const
{
	fatal_at(in_parse ? tline : cline, ofs, msg);
}

void Scanner::fatalf_at(uint line, const char* fmt, ...) const
{
	char szBuf[4096];

	va_list args;
	
	va_start(args, fmt);
	vsnprintf(szBuf, sizeof(szBuf), fmt, args);
	va_end(args);
	
	szBuf[sizeof(szBuf)-1] = '0';
	
	fatal_at(line, 0, szBuf);
}

void Scanner::fatalf(const char *fmt, ...) const
{
	char szBuf[4096];

	va_list args;
	
	va_start(args, fmt);
	vsnprintf(szBuf, sizeof(szBuf), fmt, args);
	va_end(args);
	
	szBuf[sizeof(szBuf)-1] = '0';
	
	fatal(szBuf);
}

Scanner::~Scanner()
{
	if (bot)
	{
		delete [] bot;
	}
}

void Scanner::check_token_length(char *pos, uint len) const
{
	if (pos < bot || pos + len > top)
	{
		fatal("Token exceeds limit");
	}
}

Str Scanner::raw_token(std::string enclosure) const
{
	return Str(std::string(enclosure + token().to_string() + enclosure).c_str());
}

void Scanner::reuse()
{
	next_label = 0;
	next_fill_index = 0;
	bWroteGetState = false;
	bWroteCondCheck = false;
	mapCodeName.clear();
}

void Scanner::restore_state(const ScannerState& state)
{
	int diff = bot - state.bot;
	char *old_bot = bot;
	char *old_lim = lim;
	char *old_top = top;
	char *old_eof = eof;
	*(ScannerState*)this = state;
	if (diff)
	{
		tok -= diff;
		ptr -= diff;
		cur -= diff;
		pos -= diff;
		ctx -= diff;		
		bot = old_bot;
		lim = old_lim;
		top = old_top;
		eof = old_eof;
	}
}

} // end namespace re2c
