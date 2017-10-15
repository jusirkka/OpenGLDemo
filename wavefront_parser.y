%{

#include "modelstore.h"

int wavefront_lex(Demo::WF::ValueType*, Demo::WF::LocationType*, yyscan_t);

using Demo::WF::TripletIndex;
using Demo::GL::ModelStore;

#define HANDLE_ERROR(item, errnum) {models->createError(item, errnum); YYERROR;}



%}

%locations
%define parse.trace

%define api.prefix {wavefront_}
%define api.pure full
%define api.value.type {Demo::WF::ValueType}

%parse-param {Demo::GL::ModelStore* models}
%parse-param {yyscan_t scanner}
%lex-param {yyscan_t scanner}

%token UNK END UNSUPP CSTYPE DEG SURF PARM SURFEND
%token VERTEX TEXCOORD NORMAL FACE

%token <v_triplet> VERT VERT_NORM VERT_TEX VERT_TEX_NORM
%token <v_int> INT
%token <v_float> FLOAT
%token <v_string> VARNAME TYPENAME

%type <v_triplets> verts verts_norms verts_texes verts_texes_norms ints
%type <v_ints> controlpoints
%type <v_float> numeric
%type <v_floats> numerics


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
    statement END
    ;

statement:
    unsupported
    |
    supported
    ;

unsupported:
    UNSUPP dummy_argument_list
    ;


dummy_argument_list:
    dummy_argument
    |
    dummy_argument_list dummy_argument
    ;

dummy_argument:
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
    |
    parametricdef
    |
    poldeg
    |
    surfbegin
    |
    surfparameter
    |
    surfend
    ;


vertex:
    VERTEX numeric numeric numeric
        {
            models->appendVertex($2, $3, $4);
        }
    |
    VERTEX numeric numeric numeric numeric
        {
            models->appendVertex($2, $3, $4, $5);
        }
    ;

numeric:
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
    NORMAL numeric numeric numeric
        {
            models->appendNormal($2, $3, $4);
        }
    ;

texcoord:
    TEXCOORD numeric
        {
            // unsupported
        }
    |
    TEXCOORD numeric numeric numeric
        {
            // unsupported
        }
    |
    TEXCOORD numeric numeric
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
            $$.append(TripletIndex($1[0], $1[1], $1[2]));
        }
    |
    verts VERT
        {
            $$.append(TripletIndex($2[0], $2[1], $2[2]));
        }
    ;

ints:
    INT
        {
            $$.clear();
            $$.append(TripletIndex($1, 0, 0));
        }
    |
    ints INT
        {
            $$.append(TripletIndex($2, 0, 0));
        }
    ;

verts_norms:
    VERT_NORM
        {
            $$.clear();
            $$.append(TripletIndex($1[0], $1[1], $1[2]));
        }
    |
    verts_norms VERT_NORM
        {
            $$.append(TripletIndex($2[0], $2[1], $2[2]));
        }
    ;

verts_texes:
    VERT_TEX
        {
            $$.clear();
            $$.append(TripletIndex($1[0], $1[1], $1[2]));
        }
    |
    verts_texes VERT_TEX
        {
            $$.append(TripletIndex($2[0], $2[1], $2[2]));
        }
    ;

verts_texes_norms:
    VERT_TEX_NORM
        {
            $$.clear();
            $$.append(TripletIndex($1[0], $1[1], $1[2]));
        }
    |
    verts_texes_norms VERT_TEX_NORM
        {
            $$.append(TripletIndex($2[0], $2[1], $2[2]));
        }
    ;

parametricdef:
    CSTYPE TYPENAME
        {
            if (models->inPatchDef()) {
                HANDLE_ERROR("cstype", ModelStore::Error::InSurfDef);
            }
            models->setPatchType($2);
        }
    ;


surfparameter:
    PARM VARNAME numerics
        {
            if (!models->inPatchDef()) {
                HANDLE_ERROR("parm", ModelStore::Error::SurfDefRequired);
            }
            models->setPatchKnots($2, $3);
        }
    ;

numerics:
    numeric
        {
            $$.clear();
            $$.append($1);
        }
    |
    numerics numeric
        {
            $$.append($2);
        }
    ;

poldeg:
    DEG INT
        {
            // unsupported
        }
    |
    DEG INT INT
        {
            if (models->inPatchDef()) {
                HANDLE_ERROR("deg", ModelStore::Error::InSurfDef);
            }
            models->setPatchRank($2, $3);
        }
    ;

surfbegin:
    SURF numeric numeric numeric numeric controlpoints
        {
            if (!models->checkPatchState()) {
                HANDLE_ERROR("surf", ModelStore::Error::StateNotComplete);
            }
            models->beginPatch($2, $3, $4, $5, $6);
        }
    ;

controlpoints:
    INT
        {
            $$.clear();
            $$.append($1);
        }
    |
    controlpoints INT
    {
        $$.append($2);
    }
    ;

surfend:
    SURFEND
        {
            models->endPatch();
        }
    ;


