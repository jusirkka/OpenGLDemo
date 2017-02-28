%{


extern "C"
{
    #include <stdio.h>

    int wavefront_parse(void);
    int wavefront_lex(void);
    void wavefront_error(const char *);

    int wavefront_wrap(void) {return 1;}

}



#include "modelstore.h"

using namespace GL;

%}

%debug
%locations

%union
{
    float v_float;
    int v_int;
    int p_triplet[3];
    TripletList* p_triplet_list;
}


%token UNK ENDLINE UNSUPP
%token VERTEX TEXCOORD NORMAL FACE

%token <p_triplet> VERT VERT_NORM VERT_TEX VERT_TEX_NORM
%token <v_int> INT
%token <v_float> FLOAT

%type <v_float> num
%type <p_triplet_list> verts verts_norms verts_texes verts_texes_norms ints
%destructor {delete ($$);} verts verts_norms verts_texes verts_texes_norms ints


// Grammar follows


%%

input:
    lines
    ;

lines:
    line
    |
    lines line
    ;

line:
    command ENDLINE
    ;

command:
    unsupported
    |
    supported
    ;

unsupported:
    UNSUPP arguments
    ;


arguments:
    argument
    |
    arguments argument
    ;

argument:
    FLOAT
    |
    INT
    |
    UNK
    ;


supported:
    vertex
    |
    normal
    |
    texcoord
    |
    face
    ;


vertex:
    VERTEX num num num
        {
            ModelStore::AppendVertex($2, $3, $4);
        }
    |
    VERTEX num num num num
        {
            ModelStore::AppendVertex($2, $3, $4);
        }
    ;

num:
    FLOAT
        {
            $$ = $1;
        }
    |
    INT
        {
            $$ = $1;
        }
    ;

normal:
    NORMAL num num num
        {
            ModelStore::AppendNormal($2, $3, $4);
        }
    ;

texcoord:
    TEXCOORD num
        {
            // unsupported
        }
    |
    TEXCOORD num num num
        {
            // unsupported
        }
    |
    TEXCOORD num num
        {
            ModelStore::AppendTex($2, $3);
        }
    ;

face:
    FACE ints
        {
            ModelStore::AppendFace(*$2);
        }
    |
    FACE verts
        {
            ModelStore::AppendFace(*$2);
        }
    |
    FACE verts_norms
        {
            ModelStore::AppendFace(*$2);
        }
    |
    FACE verts_texes
        {
            ModelStore::AppendFace(*$2);
        }
    |
    FACE verts_texes_norms
        {
            ModelStore::AppendFace(*$2);
        }

    ;

verts:
    VERT
        {
            $$ = new TripletList;
            $$->append(Triplet($1[0], $1[1], $1[2]));
        }
    |
    verts VERT
        {
            $$->append(Triplet($2[0], $2[1], $2[2]));
        }
    ;

ints:
    INT
        {
            $$ = new TripletList;
            $$->append(Triplet($1, 0, 0));
        }
    |
    ints INT
        {
            $$->append(Triplet($2, 0, 0));
        }
    ;

verts_norms:
    VERT_NORM
        {
            $$ = new TripletList;
            $$->append(Triplet($1[0], $1[1], $1[2]));
        }
    |
    verts_norms VERT_NORM
        {
            $$->append(Triplet($2[0], $2[1], $2[2]));
        }
    ;

verts_texes:
    VERT_TEX
        {
            $$ = new TripletList;
            $$->append(Triplet($1[0], $1[1], $1[2]));
        }
    |
    verts_texes VERT_TEX
        {
            $$->append(Triplet($2[0], $2[1], $2[2]));
        }
    ;

verts_texes_norms:
    VERT_TEX_NORM
        {
            $$ = new TripletList;
            $$->append(Triplet($1[0], $1[1], $1[2]));
        }
    |
    verts_texes_norms VERT_TEX_NORM
        {
            $$->append(Triplet($2[0], $2[1], $2[2]));
        }
    ;

