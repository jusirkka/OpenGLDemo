%{

#include "modelstore.h"

int wavefront_lex(Demo::WF::ValueType*, Demo::WF::LocationType*, yyscan_t);

using Demo::WF::Triplet;

%}

%locations
%define parse.trace

%define api.prefix {wavefront_}
%define api.pure full
%define api.value.type {Demo::WF::ValueType}

%parse-param {Demo::GL::ModelStore* models}
%parse-param {yyscan_t scanner}
%lex-param {yyscan_t scanner}

%token UNK ENDLINE UNSUPP
%token VERTEX TEXCOORD NORMAL FACE

%token <v_triplet> VERT VERT_NORM VERT_TEX VERT_TEX_NORM
%token <v_int> INT
%token <v_float> FLOAT

%type <v_float> num
%type <v_triplet_list> verts verts_norms verts_texes verts_texes_norms ints


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
            models->appendVertex($2, $3, $4);
        }
    |
    VERTEX num num num num
        {
            models->appendVertex($2, $3, $4);
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
            models->appendNormal($2, $3, $4);
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
            models->appendTex($2, $3);
        }
    ;

face:
    FACE ints
        {
            models->appendFace($2);
        }
    |
    FACE verts
        {
            models->appendFace($2);
        }
    |
    FACE verts_norms
        {
            models->appendFace($2);
        }
    |
    FACE verts_texes
        {
            models->appendFace($2);
        }
    |
    FACE verts_texes_norms
        {
            models->appendFace($2);
        }

    ;

verts:
    VERT
        {
            $$.clear();
            $$.append(Triplet($1[0], $1[1], $1[2]));
        }
    |
    verts VERT
        {
            $$.append(Triplet($2[0], $2[1], $2[2]));
        }
    ;

ints:
    INT
        {
            $$.clear();
            $$.append(Triplet($1, 0, 0));
        }
    |
    ints INT
        {
            $$.append(Triplet($2, 0, 0));
        }
    ;

verts_norms:
    VERT_NORM
        {
            $$.clear();
            $$.append(Triplet($1[0], $1[1], $1[2]));
        }
    |
    verts_norms VERT_NORM
        {
            $$.append(Triplet($2[0], $2[1], $2[2]));
        }
    ;

verts_texes:
    VERT_TEX
        {
            $$.clear();
            $$.append(Triplet($1[0], $1[1], $1[2]));
        }
    |
    verts_texes VERT_TEX
        {
            $$.append(Triplet($2[0], $2[1], $2[2]));
        }
    ;

verts_texes_norms:
    VERT_TEX_NORM
        {
            $$.clear();
            $$.append(Triplet($1[0], $1[1], $1[2]));
        }
    |
    verts_texes_norms VERT_TEX_NORM
        {
            $$.append(Triplet($2[0], $2[1], $2[2]));
        }
    ;

