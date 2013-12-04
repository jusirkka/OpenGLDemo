#ifndef OBJ_SCANNER_TYPES_H
#define OBJ_SCANNER_TYPES_H



#define YYLTYPE_IS_DECLARED 1
#define YYSTYPE_IS_DECLARED 1


/* copy of YYSTYPE in obj_grammar.y */
typedef union YYSTYPE
{
    int v_int;
    float v_float;
    int p_triplet[3];

} YYSTYPE;

typedef struct YYLTYPE
{
  int row;
  int col;
  int pos;
  int prev_col;
  int prev_pos;
} YYLTYPE;

extern YYLTYPE g_olloc;


#endif // OBJ_SCANNER_TYPES_H
