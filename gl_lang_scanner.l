%{
#include "gl_lang_compiler.h"
#include "gl_lang_parser.h"
#ifndef YYSTYPE
#define YYSTYPE GL_LANG_STYPE
#endif
#ifndef YYLTYPE
#define YYLTYPE GL_LANG_LTYPE
#endif

#define YY_USER_ACTION  \
    yylloc->col = yylloc->prev_col; yylloc->prev_col += yyleng; yylloc->pos = yylloc->prev_pos; yylloc->prev_pos += yyleng;

#define YY_USER_INIT  \
    yylloc->prev_col = 0; yylloc->row = 0; yylloc->prev_pos = 0;

#define my_init_line \
    yylloc->row++; yylloc->prev_col = 0;

#define YY_DECL int gl_lang_lex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, yyscan_t yyscanner)

%}

%option reentrant
%option bison-locations
%option noyywrap
%option prefix="gl_lang_"


dig             [0-9]
unum            {dig}+
num             [-+]?{dig}+
float_1         {unum}\.({unum})?([Ee]{num})?
float_2         \.{unum}([Ee]{num})?
float_3         {unum}[Ee]{num}
float           {float_1}|{float_2}|{float_3}

singlechar      [\*\/+\-\^=<>\!,\(\)\[\]]

id              [A-Za-z_][A-Za-z0-9_]*

real            Real
matrix          Matrix
vector          Vector
natural         Natural
text            Text
shared          Shared
execute         Execute
from            From
import          import

%x INSTRING

%%


{float}                 yylval->v_real = strtod(yytext, NULL); return FLOAT;
{unum}                  yylval->v_int = (int) strtol(yytext, NULL, 0); return INT;

"=="            yylval->v_int = EQ; return EQ;
"!="            yylval->v_int = NE; return NE;
"<="            yylval->v_int = LE; return LE;
">="            yylval->v_int = GE; return GE;
"||"            yylval->v_int = OR; return OR;
"&&"            yylval->v_int = AND; return AND;
"|"             yylval->v_int = BOR; return BOR;
"&"             yylval->v_int = BAND; return BAND;


{real}          return REAL;
{vector}        return VECTOR;
{matrix}        return MATRIX;
{natural}       return NATURAL;
{text}          return TEXT;
{shared}        return SHARED;
{execute}       return EXECUTE;
{from}          return FROM;
{import}        return IMPORT;

{id}            {
                    yylval->v_string = yytext;
                    return ID;
                }

{singlechar}    yylval->v_int = *yytext; return *yytext;



"//".*\n       my_init_line; // eat up comment line

\n             my_init_line; return SEP;


\"             {
                    BEGIN(INSTRING);
                    return BEGINSTRING;
               }


[ \t]+         // eat up whitespace
.             return UNK;

<INSTRING>\"            {
                            BEGIN(0);
                            return ENDSTRING;
                        }

<INSTRING>\n            {
                            my_init_line;
                            yylval->v_char = *yytext;
                            return CHAR;
                        }

<INSTRING>.             yylval->v_char = *yytext; return CHAR;

%%

