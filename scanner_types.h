#ifndef SCANNER_TYPES_H
#define SCANNER_TYPES_H

#define YYLTYPE_IS_DECLARED 1
#define YYSTYPE_IS_DECLARED 1

/* copy of YYSTYPE in grammar.y */
typedef union YYSTYPE
{
    int int_value;
    double real_value;
    char char_value;
    char string_value[1024];
} YYSTYPE;

typedef struct YYLTYPE
{
  int row;
  int col;
  int pos;
  int prev_col;
  int prev_pos;
} YYLTYPE;

extern YYLTYPE yylloc;


#endif // SCANNER_TYPES_H
