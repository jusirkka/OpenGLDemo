%{

#include "modelstore.h"
#include "wavefront_parser.h"
#ifndef YYSTYPE
#define YYSTYPE WAVEFRONT_STYPE
#endif
#ifndef YYLTYPE
#define YYLTYPE WAVEFRONT_LTYPE
#endif

#define YY_USER_ACTION  \
    yylloc->col = yylloc->prev_col; yylloc->prev_col += yyleng; yylloc->pos = yylloc->prev_pos; yylloc->prev_pos += yyleng;

#define YY_USER_INIT  \
    yylloc->prev_col = 0; yylloc->row = 0; yylloc->prev_pos = 0;

#define my_init_line \
    yylloc->row++; yylloc->prev_col = 0;

#define YY_DECL int wavefront_lex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, yyscan_t yyscanner)

%}

%option reentrant
%option bison-locations
%option noyywrap
%option prefix="wavefront_"



dig             [0-9]
unum            {dig}+
num             [-+]?{dig}+
float_1         {num}\.({unum})?([Ee]{num})?
float_2         \.{unum}([Ee]{num})?
float_3         {num}[Ee]{num}

triple_v        {unum}\/\/
triple_vt       {unum}\/{unum}\/
triple_vn       {unum}\/\/{unum}
triple_vtn      {unum}\/{unum}\/{unum}


space           [ \t]*
sep             [ \t]

vertex          ^v{sep}
normal          ^vn{sep}
texcoord        ^vt{sep}
face            ^f{sep}

unsupp1         vp|cstype|deg|bmat|step|p|l|curv|curv2|surf|parm|trom|hole|scrv|sp
unsupp2         end|con|g|s|mg|o|bevel|c_interp|d_interp|lod|usemtl|mtllib|shadow_obj
unsupp3         trace_obj|ctech|stech|call|csh

unsupp          ^({unsupp1}|{unsupp2}|{unsupp3}){sep}



%%


{texcoord}              return TEXCOORD;
{normal}                return NORMAL;
{vertex}                return VERTEX;
{face}                  return FACE;
{unsupp}                return UNSUPP;

{triple_v}              {
                            char* first = index(yytext, '/');
                            yylval->v_triplet[0] = strtof(yytext, &first);
                            yylval->v_triplet[1] = 0;
                            yylval->v_triplet[2] = 0;
                            return VERT;
                        }

{triple_vt}             {
                            char* first = index(yytext, '/');
                            char* last = rindex(yytext, '/');
                            yylval->v_triplet[0] = strtof(yytext, &first);
                            yylval->v_triplet[1] = strtof(first + 1, &last);
                            yylval->v_triplet[2] = 0;
                            return VERT_TEX;
                        }

{triple_vn}             {
                            char* first = index(yytext, '/');
                            char* last = rindex(yytext, '/');
                            yylval->v_triplet[0] = strtof(yytext, &first);
                            yylval->v_triplet[1] = 0;
                            yylval->v_triplet[2] = strtof(last + 1, NULL);
                            return VERT_NORM;
                        }

{triple_vtn}            {
                            char* first = index(yytext, '/');
                            char* last = rindex(yytext, '/');
                            yylval->v_triplet[0] = strtof(yytext, &first);
                            yylval->v_triplet[1] = strtof(first + 1, &last);
                            yylval->v_triplet[2] = strtof(last + 1, NULL);
                            return VERT_TEX_NORM;
                        }

{float_1}                yylval->v_float = strtod(yytext, NULL); return FLOAT;
{float_2}                yylval->v_float = strtod(yytext, NULL); return FLOAT;
{float_3}                yylval->v_float = strtod(yytext, NULL); return FLOAT;
{num}                  yylval->v_int = (int) strtol(yytext, NULL, 0); return INT;




^{space}\n            my_init_line; // eat up empty line
^{space}"#".*\n       my_init_line; // eat up comment line


"#".*\n            {
                        my_init_line;
                        return ENDLINE;
                   }


\n            {
                    my_init_line;
                    return ENDLINE;
               }



{sep}+         // eat up whitespace

.             return UNK;


%%

