/* Generated by re2c 0.9.2 on Mon Apr 19 03:45:41 2004 */
#line 1 "scanner.re"
/* $Id$ */
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <unistd.h>
#include "scanner.h"
#include "parser.h"
#include "y.tab.h"

extern YYSTYPE yylval;

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define	BSIZE	8192

#define	YYCTYPE		char
#define	YYCURSOR	cursor
#define	YYLIMIT		lim
#define	YYMARKER	ptr
#define	YYFILL(n)	{cursor = fill(cursor);}

#define	RETURN(i)	{cur = cursor; return i;}


Scanner::Scanner(int i) : in(i),
	bot(NULL), tok(NULL), ptr(NULL), cur(NULL), pos(NULL), lim(NULL),
	top(NULL), eof(NULL), tchar(0), tline(0), cline(1) {
    ;
}

char *Scanner::fill(char *cursor){
    if(!eof){
	uint cnt = tok - bot;
	if(cnt){
	    memcpy(bot, tok, lim - tok);
	    tok = bot;
	    ptr -= cnt;
	    cursor -= cnt;
	    pos -= cnt;
	    lim -= cnt;
	}
	if((top - lim) < BSIZE){
	    char *buf = new char[(lim - bot) + BSIZE];
	    memcpy(buf, tok, lim - tok);
	    tok = buf;
	    ptr = &buf[ptr - bot];
	    cursor = &buf[cursor - bot];
	    pos = &buf[pos - bot];
	    lim = &buf[lim - bot];
	    top = &lim[BSIZE];
	    delete [] bot;
	    bot = buf;
	}
	if((cnt = read(in, (char*) lim, BSIZE)) != BSIZE){
	    eof = &lim[cnt]; *eof++ = '\n';
	}
	lim += cnt;
    }
    return cursor;
}

#line 73 "scanner.re"


int Scanner::echo(ostream &out){
    char *cursor = cur;

    // Catch EOF
    if (eof && cursor == eof)
    	return 0;

    tok = cursor;
echo:

#line 7 "re2c-output.c"
{
	YYCTYPE yych;
	unsigned int yyaccept;
	goto yy0;
yy1:	++YYCURSOR;
yy0:
	if((YYLIMIT - YYCURSOR) < 7) YYFILL(7);
	yych = *YYCURSOR;
	if(yych == '\n')	goto yy4;
	if(yych != '/')	goto yy6;
	goto yy2;
yy2:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == '*')	goto yy7;
	goto yy3;
yy3:
#line 92 "scanner.re"
{ goto echo; }
#line 26 "re2c-output.c"
yy4:	yych = *++YYCURSOR;
	goto yy5;
yy5:
#line 88 "scanner.re"
{ if(cursor == eof) RETURN(0);
				  out.write((const char*)(tok), (const char*)(cursor) - (const char*)(tok));
				  tok = pos = cursor; cline++;
				  goto echo; }
#line 35 "re2c-output.c"
yy6:	yych = *++YYCURSOR;
	goto yy3;
yy7:	yych = *++YYCURSOR;
	if(yych == '!')	goto yy9;
	goto yy8;
yy8:	YYCURSOR = YYMARKER;
	switch(yyaccept){
	case 0:	goto yy3;
	}
yy9:	yych = *++YYCURSOR;
	if(yych != 'r')	goto yy8;
	goto yy10;
yy10:	yych = *++YYCURSOR;
	if(yych != 'e')	goto yy8;
	goto yy11;
yy11:	yych = *++YYCURSOR;
	if(yych != '2')	goto yy8;
	goto yy12;
yy12:	yych = *++YYCURSOR;
	if(yych != 'c')	goto yy8;
	goto yy13;
yy13:	yych = *++YYCURSOR;
	goto yy14;
yy14:
#line 85 "scanner.re"
{ out.write((const char*)(tok), (const char*)(&cursor[-7]) - (const char*)(tok));
				  tok = cursor;
				  RETURN(1); }
#line 64 "re2c-output.c"
}
#line 93 "scanner.re"

}


int Scanner::scan(){
    char *cursor = cur;
    uint depth;

scan:
    tchar = cursor - pos;
    tline = cline;
    tok = cursor;

#line 68 "re2c-output.c"
{
	YYCTYPE yych;
	unsigned int yyaccept;
	goto yy15;
yy16:	++YYCURSOR;
yy15:
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if(yych <= '/'){
		if(yych <= '"'){
			if(yych <= '\n'){
				if(yych <= '\b')	goto yy37;
				if(yych <= '\t')	goto yy33;
				goto yy35;
			} else {
				if(yych == ' ')	goto yy33;
				if(yych <= '!')	goto yy37;
				goto yy23;
			}
		} else {
			if(yych <= ')'){
				if(yych <= '&')	goto yy37;
				if(yych <= '\'')	goto yy25;
				goto yy29;
			} else {
				if(yych <= '*')	goto yy21;
				if(yych <= '+')	goto yy30;
				if(yych <= '.')	goto yy37;
				goto yy19;
			}
		}
	} else {
		if(yych <= '@'){
			if(yych <= '<'){
				if(yych == ';')	goto yy29;
				goto yy37;
			} else {
				if(yych <= '=')	goto yy29;
				if(yych == '?')	goto yy30;
				goto yy37;
			}
		} else {
			if(yych <= '`'){
				if(yych <= 'Z')	goto yy31;
				if(yych <= '[')	goto yy27;
				if(yych <= '\\')	goto yy29;
				goto yy37;
			} else {
				if(yych <= 'z')	goto yy31;
				if(yych <= '{')	goto yy17;
				if(yych <= '|')	goto yy29;
				goto yy37;
			}
		}
	}
yy17:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych <= '/')	goto yy18;
	if(yych <= '9')	goto yy62;
	goto yy18;
yy18:
#line 106 "scanner.re"
{ depth = 1;
				  goto code;
				}
#line 134 "re2c-output.c"
yy19:	yych = *++YYCURSOR;
	if(yych == '*')	goto yy60;
	goto yy20;
yy20:
#line 130 "scanner.re"
{ RETURN(*tok); }
#line 141 "re2c-output.c"
yy21:	yych = *++YYCURSOR;
	if(yych == '/')	goto yy58;
	goto yy22;
yy22:
#line 132 "scanner.re"
{ yylval.op = *tok;
				  RETURN(CLOSE); }
#line 149 "re2c-output.c"
yy23:	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych != '\n')	goto yy54;
	goto yy24;
yy24:
#line 123 "scanner.re"
{ fatal("bad string"); }
#line 157 "re2c-output.c"
yy25:	yyaccept = 2;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych != '\n')	goto yy49;
	goto yy26;
yy26:
#line 158 "scanner.re"
{ cerr << "unexpected character: " << *tok << endl;
				  goto scan;
				}
#line 167 "re2c-output.c"
yy27:	yyaccept = 3;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych != '\n')	goto yy43;
	goto yy28;
yy28:
#line 128 "scanner.re"
{ fatal("bad character constant"); }
#line 175 "re2c-output.c"
yy29:	yych = *++YYCURSOR;
	goto yy20;
yy30:	yych = *++YYCURSOR;
	goto yy22;
yy31:	yych = *++YYCURSOR;
	goto yy41;
yy32:
#line 147 "scanner.re"
{ cur = cursor;
				  yylval.symbol = Symbol::find(token());
				  return ID; }
#line 187 "re2c-output.c"
yy33:	yych = *++YYCURSOR;
	goto yy39;
yy34:
#line 151 "scanner.re"
{ goto scan; }
#line 193 "re2c-output.c"
yy35:	yych = *++YYCURSOR;
	goto yy36;
yy36:
#line 153 "scanner.re"
{ if(cursor == eof) RETURN(0);
				  pos = cursor; cline++;
				  goto scan;
	    			}
#line 202 "re2c-output.c"
yy37:	yych = *++YYCURSOR;
	goto yy26;
yy38:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy39;
yy39:	if(yych == '\t')	goto yy38;
	if(yych == ' ')	goto yy38;
	goto yy34;
yy40:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy41;
yy41:	if(yych <= '@'){
		if(yych <= '/')	goto yy32;
		if(yych <= '9')	goto yy40;
		goto yy32;
	} else {
		if(yych <= 'Z')	goto yy40;
		if(yych <= '`')	goto yy32;
		if(yych <= 'z')	goto yy40;
		goto yy32;
	}
yy42:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy43;
yy43:	if(yych <= '['){
		if(yych != '\n')	goto yy42;
		goto yy44;
	} else {
		if(yych <= '\\')	goto yy45;
		if(yych <= ']')	goto yy46;
		goto yy42;
	}
yy44:	YYCURSOR = YYMARKER;
	switch(yyaccept){
	case 0:	goto yy18;
	case 1:	goto yy24;
	case 3:	goto yy28;
	case 2:	goto yy26;
	}
yy45:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if(yych == '\n')	goto yy44;
	goto yy42;
yy46:	yych = *++YYCURSOR;
	goto yy47;
yy47:
#line 125 "scanner.re"
{ cur = cursor;
				  yylval.regexp = ranToRE(token());
				  return RANGE; }
#line 257 "re2c-output.c"
yy48:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy49;
yy49:	if(yych <= '&'){
		if(yych == '\n')	goto yy44;
		goto yy48;
	} else {
		if(yych <= '\'')	goto yy51;
		if(yych != '\\')	goto yy48;
		goto yy50;
	}
yy50:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if(yych == '\n')	goto yy44;
	goto yy48;
yy51:	yych = *++YYCURSOR;
	goto yy52;
yy52:
#line 119 "scanner.re"
{ cur = cursor;
				  yylval.regexp = strToCaseInsensitiveRE(token());
				  return STRING; }
#line 282 "re2c-output.c"
yy53:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy54;
yy54:	if(yych <= '!'){
		if(yych == '\n')	goto yy44;
		goto yy53;
	} else {
		if(yych <= '"')	goto yy56;
		if(yych != '\\')	goto yy53;
		goto yy55;
	}
yy55:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if(yych == '\n')	goto yy44;
	goto yy53;
yy56:	yych = *++YYCURSOR;
	goto yy57;
yy57:
#line 115 "scanner.re"
{ cur = cursor;
				  yylval.regexp = strToRE(token());
				  return STRING; }
#line 307 "re2c-output.c"
yy58:	yych = *++YYCURSOR;
	goto yy59;
yy59:
#line 112 "scanner.re"
{ tok = cursor;
				  RETURN(0); }
#line 314 "re2c-output.c"
yy60:	yych = *++YYCURSOR;
	goto yy61;
yy61:
#line 109 "scanner.re"
{ depth = 1;
				  goto comment; }
#line 321 "re2c-output.c"
yy62:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy63;
yy63:	if(yych <= '/'){
		if(yych == ',')	goto yy66;
		goto yy44;
	} else {
		if(yych <= '9')	goto yy62;
		if(yych != '}')	goto yy44;
		goto yy64;
	}
yy64:	yych = *++YYCURSOR;
	goto yy65;
yy65:
#line 135 "scanner.re"
{ yylval.extop.minsize = atoi((char *)tok+1);
				  yylval.extop.maxsize = atoi((char *)tok+1);
				  RETURN(CLOSESIZE); }
#line 341 "re2c-output.c"
yy66:	yych = *++YYCURSOR;
	if(yych != '}')	goto yy70;
	goto yy67;
yy67:	yych = *++YYCURSOR;
	goto yy68;
yy68:
#line 143 "scanner.re"
{ yylval.extop.minsize = atoi((char *)tok+1);
				  yylval.extop.maxsize = -1;
				  RETURN(CLOSESIZE); }
#line 352 "re2c-output.c"
yy69:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy70;
yy70:	if(yych <= '/')	goto yy44;
	if(yych <= '9')	goto yy69;
	if(yych != '}')	goto yy44;
	goto yy71;
yy71:	yych = *++YYCURSOR;
	goto yy72;
yy72:
#line 139 "scanner.re"
{ yylval.extop.minsize = atoi((char *)tok+1);
				  yylval.extop.maxsize = MAX(yylval.extop.minsize,atoi(strchr((char *)tok, ',')+1));
				  RETURN(CLOSESIZE); }
#line 368 "re2c-output.c"
}
#line 161 "scanner.re"


code:

#line 372 "re2c-output.c"
{
	YYCTYPE yych;
	unsigned int yyaccept;
	goto yy73;
yy74:	++YYCURSOR;
yy73:
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if(yych <= '&'){
		if(yych <= '\n'){
			if(yych <= '\t')	goto yy81;
			goto yy79;
		} else {
			if(yych == '"')	goto yy83;
			goto yy81;
		}
	} else {
		if(yych <= '{'){
			if(yych <= '\'')	goto yy84;
			if(yych <= 'z')	goto yy81;
			goto yy77;
		} else {
			if(yych != '}')	goto yy81;
			goto yy75;
		}
	}
yy75:	yych = *++YYCURSOR;
	goto yy76;
yy76:
#line 165 "scanner.re"
{ if(--depth == 0){
					cur = cursor;
					yylval.token = new Token(token(), tline);
					return CODE;
				  }
				  goto code; }
#line 409 "re2c-output.c"
yy77:	yych = *++YYCURSOR;
	goto yy78;
yy78:
#line 171 "scanner.re"
{ ++depth;
				  goto code; }
#line 416 "re2c-output.c"
yy79:	yych = *++YYCURSOR;
	goto yy80;
yy80:
#line 173 "scanner.re"
{ if(cursor == eof) fatal("missing '}'");
				  pos = cursor; cline++;
				  goto code;
				}
#line 425 "re2c-output.c"
yy81:	yych = *++YYCURSOR;
	goto yy82;
yy82:
#line 177 "scanner.re"
{ goto code; }
#line 431 "re2c-output.c"
yy83:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == '\n')	goto yy82;
	goto yy90;
yy84:	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if(yych == '\n')	goto yy82;
	goto yy86;
yy85:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy86;
yy86:	if(yych <= '&'){
		if(yych != '\n')	goto yy85;
		goto yy87;
	} else {
		if(yych <= '\'')	goto yy81;
		if(yych == '\\')	goto yy88;
		goto yy85;
	}
yy87:	YYCURSOR = YYMARKER;
	switch(yyaccept){
	case 0:	goto yy82;
	}
yy88:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if(yych == '\n')	goto yy87;
	goto yy85;
yy89:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	goto yy90;
yy90:	if(yych <= '!'){
		if(yych == '\n')	goto yy87;
		goto yy89;
	} else {
		if(yych <= '"')	goto yy81;
		if(yych != '\\')	goto yy89;
		goto yy91;
	}
yy91:	++YYCURSOR;
	if(YYLIMIT == YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if(yych == '\n')	goto yy87;
	goto yy89;
}
#line 178 "scanner.re"


comment:

#line 481 "re2c-output.c"
{
	YYCTYPE yych;
	unsigned int yyaccept;
	goto yy92;
yy93:	++YYCURSOR;
yy92:
	if((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if(yych <= ')'){
		if(yych == '\n')	goto yy97;
		goto yy99;
	} else {
		if(yych <= '*')	goto yy94;
		if(yych == '/')	goto yy96;
		goto yy99;
	}
yy94:	yych = *++YYCURSOR;
	if(yych == '/')	goto yy102;
	goto yy95;
yy95:
#line 192 "scanner.re"
{ goto comment; }
#line 504 "re2c-output.c"
yy96:	yych = *++YYCURSOR;
	if(yych == '*')	goto yy100;
	goto yy95;
yy97:	yych = *++YYCURSOR;
	goto yy98;
yy98:
#line 188 "scanner.re"
{ if(cursor == eof) RETURN(0);
				  tok = pos = cursor; cline++;
				  goto comment;
				}
#line 516 "re2c-output.c"
yy99:	yych = *++YYCURSOR;
	goto yy95;
yy100:	yych = *++YYCURSOR;
	goto yy101;
yy101:
#line 186 "scanner.re"
{ ++depth;
				  goto comment; }
#line 525 "re2c-output.c"
yy102:	yych = *++YYCURSOR;
	goto yy103;
yy103:
#line 182 "scanner.re"
{ if(--depth == 0)
					goto scan;
				    else
					goto comment; }
#line 534 "re2c-output.c"
}
#line 193 "scanner.re"

}

void Scanner::fatal(char *msg){
    cerr << "line " << tline << ", column " << (tchar + 1) << ": "
	<< msg << endl;
    exit(1);
}
