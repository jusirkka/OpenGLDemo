%{
#include <stdlib.h>
#include <ctype.h>

#include "gl_lang_types.h"
#include "gl_lang.h"

#define yylval gl_lang_lval
#define yylloc gl_lang_lloc

#define YY_USER_ACTION  \
    yylloc.col = yylloc.prev_col; yylloc.prev_col += yyleng; yylloc.pos = yylloc.prev_pos; yylloc.prev_pos += yyleng;

#define YY_USER_INIT  \
    yylloc.prev_col = 0; yylloc.row = 0; yylloc.prev_pos = 0;

#define my_init_line \
    yylloc.row++; yylloc.prev_col = 0;

#define YY_DECL int gl_lang_lex (void)


%}



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

%x INSTRING

%%


{float}                 yylval.real_value = strtod(yytext, NULL); return FLOAT;
{unum}                  yylval.int_value = (int) strtol(yytext, NULL, 0); return INT;

"=="            yylval.int_value = EQ; return EQ;
"!="            yylval.int_value = NE; return NE;
"<="            yylval.int_value = LE; return LE;
">="            yylval.int_value = GE; return GE;
"||"            yylval.int_value = OR; return OR;
"&&"            yylval.int_value = AND; return AND;
"|"             yylval.int_value = BOR; return BOR;
"&"             yylval.int_value = BAND; return BAND;


{real}          return REAL;
{vector}        return VECTOR;
{matrix}        return MATRIX;
{natural}       return NATURAL;
{text}          return TEXT;
{shared}        return SHARED;
{execute}       return EXECUTE;

{id}            {
                    strcpy(yylval.string_value, yytext);
                    return ID;
                }

{singlechar}    yylval.int_value = *yytext; return *yytext;



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
                            yylval.char_value = *yytext;
                            return CHAR;
                        }

<INSTRING>.             yylval.char_value = *yytext; return CHAR;

%%

char parser_error_buffer[256];

void gl_lang_error(const char *str) {
    strcpy(parser_error_buffer, str);
/*    fprintf(stderr,"error at line %d, column %d: %s\n",
            yylloc.first_line,
            yylloc.first_column,
            str);*/
}

