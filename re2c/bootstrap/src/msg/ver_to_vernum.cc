/* Generated by re2c 1.1.1 on Wed Feb 13 07:15:30 2019 */
#line 1 "../src/msg/ver_to_vernum.re"
#include "src/util/c99_stdint.h"
#include <string>

#include "config.h"
#include "src/msg/ver_to_vernum.h"

namespace re2c {

std::string ver_to_vernum(const char *ver)
{
#define YYCTYPE char
    const char *YYCURSOR = ver, *YYMARKER,
        *p1, *p2, *p3, *e1, *e2, *e3;

#line 15 "../src/msg/ver_to_vernum.re"

const char *yyt1;
const char *yyt2;
const char *yyt3;
const char *yyt4;
const char *yyt5;
const char *yyt6;


#line 28 "src/msg/ver_to_vernum.cc"
{
	YYCTYPE yych;
	static const unsigned char yybm[] = {
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
		  0,   0,   0,   0,   0,   0,   0,   0, 
	};
	yych = *YYCURSOR;
	if (yych <= '/') goto yy2;
	if (yych <= '9') {
		yyt1 = YYCURSOR;
		goto yy4;
	}
yy2:
	++YYCURSOR;
yy3:
#line 49 "../src/msg/ver_to_vernum.re"
	{ return std::string(); }
#line 76 "src/msg/ver_to_vernum.cc"
yy4:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= '.') {
		if (yych <= 0x00) {
			yyt3 = yyt4 = yyt5 = yyt6 = NULL;
			yyt2 = YYCURSOR;
			goto yy5;
		}
		if (yych <= '-') goto yy3;
		yyt3 = yyt4 = yyt5 = yyt6 = NULL;
		yyt2 = YYCURSOR;
		goto yy7;
	} else {
		if (yych <= '/') goto yy3;
		if (yych <= '9') goto yy9;
		goto yy3;
	}
yy5:
	++YYCURSOR;
	p1 = yyt1;
	e1 = yyt2;
	p2 = yyt3;
	e2 = yyt4;
	p3 = yyt5;
	e3 = yyt6;
#line 31 "../src/msg/ver_to_vernum.re"
	{

        static const uint32_t PARTS = 3, WIDTH = 2, SIZE = PARTS * WIDTH;
        char buffer[SIZE], *q, *q0;
        const char * bounds[] = {p1, e1, p2, e2, p3, e3}, *p, *p0;

        for (uint32_t i = 0; i < PARTS; ++i) {
            q0 = buffer + 2 * i;
            q  = buffer + 2 * (i + 1);
            p0 = bounds[2 * i];
            p  = bounds[2 * i + 1];
            while (p > p0) *--q = *--p;
            while (q > q0) *--q = '0';
        }

        return std::string(buffer, SIZE);
    }
#line 120 "src/msg/ver_to_vernum.cc"
yy7:
	yych = *++YYCURSOR;
	if (yych <= 0x00) goto yy8;
	if (yych <= '/') goto yy12;
	if (yych <= '9') {
		yyt3 = YYCURSOR;
		goto yy10;
	}
	goto yy12;
yy8:
	YYCURSOR = YYMARKER;
	goto yy3;
yy9:
	yych = *++YYCURSOR;
	if (yych <= 0x00) {
		yyt3 = yyt4 = yyt5 = yyt6 = NULL;
		yyt2 = YYCURSOR;
		goto yy5;
	}
	if (yych == '.') {
		yyt3 = yyt4 = yyt5 = yyt6 = NULL;
		yyt2 = YYCURSOR;
		goto yy7;
	}
	goto yy8;
yy10:
	yych = *++YYCURSOR;
	if (yych <= '.') {
		if (yych <= 0x00) {
			yyt5 = yyt6 = NULL;
			yyt4 = YYCURSOR;
			goto yy5;
		}
		if (yych <= '-') goto yy8;
		yyt5 = yyt6 = NULL;
		yyt4 = YYCURSOR;
		goto yy13;
	} else {
		if (yych <= '/') goto yy8;
		if (yych <= '9') goto yy14;
		goto yy8;
	}
yy11:
	yych = *++YYCURSOR;
yy12:
	if (yybm[0+yych] & 128) {
		goto yy11;
	}
	if (yych <= 0x00) goto yy5;
	goto yy8;
yy13:
	yych = *++YYCURSOR;
	if (yych <= 0x00) goto yy8;
	if (yych <= '/') goto yy12;
	if (yych <= '9') {
		yyt5 = YYCURSOR;
		goto yy15;
	}
	goto yy12;
yy14:
	yych = *++YYCURSOR;
	if (yych <= 0x00) {
		yyt5 = yyt6 = NULL;
		yyt4 = YYCURSOR;
		goto yy5;
	}
	if (yych == '.') {
		yyt5 = yyt6 = NULL;
		yyt4 = YYCURSOR;
		goto yy13;
	}
	goto yy8;
yy15:
	yych = *++YYCURSOR;
	if (yych <= '.') {
		if (yych <= 0x00) {
			yyt6 = YYCURSOR;
			goto yy5;
		}
		if (yych <= '-') goto yy8;
		yyt6 = YYCURSOR;
	} else {
		if (yych <= '/') goto yy8;
		if (yych <= '9') goto yy17;
		goto yy8;
	}
yy16:
	yych = *++YYCURSOR;
	if (yybm[0+yych] & 128) {
		goto yy11;
	}
	if (yych <= '/') goto yy8;
	if (yych <= '9') goto yy18;
	goto yy8;
yy17:
	yych = *++YYCURSOR;
	if (yych <= 0x00) {
		yyt6 = YYCURSOR;
		goto yy5;
	}
	if (yych == '.') {
		yyt6 = YYCURSOR;
		goto yy16;
	}
	goto yy8;
yy18:
	yych = *++YYCURSOR;
	if (yych <= '.') {
		if (yych <= 0x00) goto yy5;
		if (yych <= '-') goto yy8;
		goto yy16;
	} else {
		if (yych <= '/') goto yy8;
		if (yych >= ':') goto yy8;
	}
	yych = *++YYCURSOR;
	if (yych <= 0x00) goto yy5;
	if (yych == '.') goto yy16;
	goto yy8;
}
#line 50 "../src/msg/ver_to_vernum.re"

#undef YYCTYPE
}

} // namespace re2c
