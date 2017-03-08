%{


#include "gl_lang_compiler.h"
#include "constant.h"

int gl_lang_lex(Demo::GL::ValueType*, Demo::GL::LocationType*, yyscan_t);

const char* opname(int);
unsigned int operation(int);
unsigned int lrtype(int, int);

using namespace Demo;
using Demo::GL::Compiler;

#define HANDLE_ERROR(item, errnum) {compiler->createError(item, errnum); YYERROR;}

%}

%locations
%define parse.trace

%define api.prefix {gl_lang_}
%define api.pure full
%define api.value.type {Demo::GL::ValueType}

%parse-param {Demo::GL::Compiler* compiler}
%parse-param {yyscan_t scanner}
%lex-param {yyscan_t scanner}


%type <v_string_list> variables

%type <v_int_list> parameters arglist

%type <v_string> text chars

%type <v_int> rhs simple_rhs cond_rhs cond_rhs_seq guard
%type <v_int> expression terms factors factor statement
%type <v_int> rel_op eq_op add_op sign and_op or_op
%type <v_int> literal paren_or_variable paren_or_variable_comp function_call
%type <v_bool> shared
%type <v_int> typename



%token <v_int> INT
%token <v_real> FLOAT
%token <v_char> CHAR

%token <v_string> ID

%token VECTOR MATRIX TEXT NATURAL SHARED REAL EXECUTE FROM IMPORT

%nonassoc <v_int> '<' '>' EQ NE LE GE
%left <v_int> '+' '-' OR BOR
%left <v_int> '*' '/' AND BAND
%left <v_int> NEG '!'

%token UNK BEGINSTRING ENDSTRING SEP

// Grammar follows


%%

input:
    elements
    ;

elements:
    element
    |
    elements element
    ;

element:
    SEP
    |
    import_from SEP
    |
    declaration SEP
    |
    assignment SEP
    |
    statement SEP
    ;


import_from:
    FROM BEGINSTRING text ENDSTRING IMPORT variables
        {
            foreach (QString name, $6) {
                qDebug() << $3 << name;
                if (compiler->isExported(name, $3)) {
                    compiler->addImported(name, $3);
                } else {
                    HANDLE_ERROR(name, Compiler::notimported);
                }
            }
        }
    ;


declaration:
    shared typename variables
        {
            foreach (QString name, $3) {
                compiler->addVariable(Var::Create($2, name, $1));
            }
        }
    ;

shared:
    /* empty */
        {$$ = false;}
    |
    SHARED
        {$$ = true;}
    ;

typename:
    NATURAL
       {$$ = Symbol::Integer;}
    |
    REAL
        {$$ = Symbol::Real;}
    |
    VECTOR
        {$$ = Symbol::Vector;}
    |
    MATRIX
        {$$ = Symbol::Matrix;}
    |
    TEXT
        {$$ = Symbol::Text;}
    ;

variables:
    ID
        {
            if (compiler->hasSymbol($1)) {
                HANDLE_ERROR($1, Compiler::declared);
            }
            $$.clear();
            $$.append($1);
        }
    |
    variables ',' ID
        {
            if (compiler->hasSymbol($3)) {
                HANDLE_ERROR($3, Compiler::declared);
            }
            if ($$.contains($3)) {
                HANDLE_ERROR($3, Compiler::duplicate);
            }
            $$.append($3);
        }
    ;



assignment:
    ID rhs
        {
            if (!compiler->hasSymbol($1)) {
                HANDLE_ERROR($1, Compiler::notdeclared);
            }
            Variable* var = dynamic_cast<Variable*>(compiler->symbol($1));
            if (!var) {
                HANDLE_ERROR($1, Compiler::notvariable);
            }
            if (compiler->isImported(var)) {
                HANDLE_ERROR($1, Compiler::assimported);
            }
            if (var->type() == $2 || (var->type() == Symbol::Real && $2 == Symbol::Integer)) {
                compiler->setCode(var->name());
            } else {
                HANDLE_ERROR($1, Compiler::assincompatible);
            }
        }
    ;


statement:
    ID  parameters
        {
            if (!compiler->hasSymbol($1)) {
                HANDLE_ERROR($1, Compiler::notdeclared);
            }
            Function* fun = dynamic_cast<Function*>(compiler->symbol($1));
            if (!fun) {
                HANDLE_ERROR($1, Compiler::notfunction);
            }
            $$ = fun->type();
            if (fun->argTypes().size() != $2.size()) {
                HANDLE_ERROR($1, Compiler::wrongargs);
            }
            // check the argument types
            for (int i = 0; i < fun->argTypes().size(); ++i) {
                int te = $2[i]; int ta = fun->argTypes()[i];
                if (ta == te) continue;
                if (ta == Symbol::Real && te == Symbol::Integer) continue;
                HANDLE_ERROR($1, Compiler::incompatibleargs);
            }
            // qDebug() << "Code:" << opname(Compiler::cFun) << fun->name() << fun->index();
            compiler->pushBack(Compiler::cFun, 0, 1 - $2.size());
            compiler->pushBack(fun->index(), 0, 0);

            compiler->setJump();
            compiler->setCode("gl_result");
        }
    |
    EXECUTE BEGINSTRING text ENDSTRING
        {
            if (compiler->isScript($3)) {
                compiler->addSubscript($3);
            } else {
                HANDLE_ERROR($3, Compiler::scriptnotfound);
            }
            compiler->pushBack(Compiler::cImmed, 0, 1);
            compiler->pushBackImmed($3);

            Function* dispatcher = dynamic_cast<Function*>(compiler->symbol("dispatch"));
            compiler->pushBack(Compiler::cFun, 0, 0);
            compiler->pushBack(dispatcher->index(), 0, 0);

            compiler->setJump();
            compiler->setCode("gl_result");
        }
    ;



rhs: simple_rhs
;

rhs: cond_rhs_seq
;

cond_rhs_seq:
    cond_rhs
        {$$ = $1;}
    |
    cond_rhs_seq cond_rhs
        {
            if (($1 != $2) &&
                ($1 != Symbol::Integer || $2 != Symbol::Real) &&
                ($1 != Symbol::Real || $2 != Symbol::Integer)) {
                HANDLE_ERROR("assignment", Compiler::incompatibletypes);
            }
            if ($2 == Symbol::Real) {
                $$ = $2;
            } else {
                $$ = $1;
            }
        }
    ;

cond_rhs:
    guard simple_rhs
        {$$ = $2;}
    ;

simple_rhs:
    '=' expression
        {
            $$ = $2;
            compiler->setJump();
        }
    ;

guard:
    '|' expression
        {
            if ($2 != Symbol::Integer) {
                HANDLE_ERROR("guard", Compiler::expectedinteger);
            }
            $$ = $2;
            // qDebug() << "Code: GUARD";
            compiler->pushBack(Compiler::cGuard, 0, 0);
            // reserve space for code and immed jumps
            compiler->pushBack(0, 0, 0);
            compiler->pushBack(0, 0, 0);
            compiler->initJump();
        }
    ;






expression:
    terms
        {$$ = $1;}
    |
    terms rel_op terms
        {
            if (($1 == Symbol::Text || $3 == Symbol::Text)) {
                HANDLE_ERROR(opname($2), Compiler::expectedintegerorreal);
            }
            if (($1 != Symbol::Integer || $3 != Symbol::Integer) &&
                ($1 != Symbol::Real || $3 != Symbol::Integer) &&
                ($1 != Symbol::Integer || $3 != Symbol::Real) &&
                ($1 != Symbol::Real || $3 != Symbol::Real)) {
                HANDLE_ERROR(opname($2), Compiler::expectedintegerorreal);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            compiler->pushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    terms eq_op terms
        {
            if (($1 != $3) &&
                ($1 != Symbol::Integer || $3 != Symbol::Real) &&
                ($1 != Symbol::Real || $3 != Symbol::Integer)) {
                HANDLE_ERROR(opname($2), Compiler::incompatibletypes);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            compiler->pushBack(operation($2), lrtype($1, $3), -1);
        }
    ;


rel_op: '>' | GE | '<' | LE
;

eq_op: EQ | NE
;


terms:
    factors
        {$$ = $1;}
    |
    terms or_op factors
        {
            if ($1 != Symbol::Integer || $3 != Symbol::Integer) {
                HANDLE_ERROR(opname($2), Compiler::expectedinteger);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            compiler->pushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    terms add_op factors
        {
            if (($1 == Symbol::Text || $3 == Symbol::Text)) {
                HANDLE_ERROR(opname($2), Compiler::expectedarithmetictype);
            }
            if (($1 != $3) &&
                ($1 != Symbol::Integer || $3 != Symbol::Real) &&
                ($1 != Symbol::Real || $3 != Symbol::Integer)) {
                HANDLE_ERROR(opname($2), Compiler::incompatibletypes);
            }
            if ($3 == Symbol::Real) {
                $$ = $3;
            } else {
                $$ = $1;
            }
            // qDebug() << "Code:" << opname($2);
            compiler->pushBack(operation($2), lrtype($1, $3), -1);
        }
    ;


or_op: BOR | OR
;

add_op: '+' | '-'
;


factors:
    factor
        {$$ = $1;}
    |
    factors and_op factor
        {
            if ($1 != Symbol::Integer || $3 != Symbol::Integer) {
                HANDLE_ERROR(opname($2), Compiler::expectedinteger);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            compiler->pushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    factors '*' factor
        {
            if (($1 == Symbol::Text || $3 == Symbol::Text)) {
                HANDLE_ERROR(opname($2), Compiler::expectedarithmetictype);
            }
            if (($1 == Symbol::Vector && $3 == Symbol::Vector) ||
                ($1 == Symbol::Vector && $3 == Symbol::Matrix)) {
                HANDLE_ERROR(opname($2), Compiler::incompatibletypes);
            }
            if ($1 == Symbol::Vector || $3 == Symbol::Vector) {
                $$ = Symbol::Vector;
            } else if ($1 == Symbol::Matrix || $3 == Symbol::Matrix) {
                $$ = Symbol::Matrix;
            } else if ($1 == Symbol::Real || $3 == Symbol::Real) {
                $$ = Symbol::Real;
            } else {
                $$ = Symbol::Integer;
            }
            // qDebug() << "Code:" << opname($2);
            compiler->pushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    factors '/' factor
        {
            if (($1 == Symbol::Text || $3 == Symbol::Text)) {
                HANDLE_ERROR(opname($2), Compiler::expectedarithmetictype);
            }
            if ($1 == Symbol::Vector || $3 == Symbol::Vector ||
                $1 == Symbol::Matrix || $3 == Symbol::Matrix) {
                HANDLE_ERROR(opname($2), Compiler::incompatibletypes);
            }
            if ($1 == Symbol::Real || $3 == Symbol::Real) {
                $$ = Symbol::Real;
            } else {
                $$ = Symbol::Integer;
            }
            // qDebug() << "Code:" << opname($2);
            compiler->pushBack(operation($2), lrtype($1, $3), -1);
        }
    ;

and_op: BAND | AND
;


factor:
    '!' factor
        {
            if ($2 != Symbol::Integer) {
                HANDLE_ERROR(opname($1), Compiler::expectedinteger);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($1);
            compiler->pushBack(operation($1), 0, 0);
        }
    |
    sign factor
        {
            $$ = $2;
            // qDebug() << "Code: (unary)" << opname($1);
            if ($1 == '-') {
                compiler->pushBack(Compiler::cNeg, lrtype(Symbol::Integer, $2), 0);
            }
        }
    ;

sign: '+' | '-' %prec NEG
;

factor: paren_or_variable_comp
;


paren_or_variable_comp:
    paren_or_variable
    |
    paren_or_variable_comp  '[' expression ']'
        {
            if ($3 != Symbol::Integer) {
                HANDLE_ERROR("[]", Compiler::expectedinteger);
            }
            if ($1 != Symbol::Matrix && $1 != Symbol::Vector) {
                HANDLE_ERROR("expression", Compiler::expectedvectorormatrix);
            }
            // qDebug() << "Code: TAKE";
            compiler->pushBack(Compiler::cTake, lrtype($1, Symbol::Integer), -1);
            if ($1 == Symbol::Matrix) {
                $$ = Symbol::Vector;
            } else {
                $$ = Symbol::Real;
            }
        }
    ;



factor: literal | function_call
;

literal:
    INT
        {
            $$ = Symbol::Integer;
            // qDebug() << "Code: INT" << $1;
            compiler->pushBack(Compiler::cImmed, 0, 1);
            compiler->pushBackImmed($1);
        }
    |
    FLOAT
        {
            $$ = Symbol::Real;
            // qDebug() << "Code: SCA" << $1;
            compiler->pushBack(Compiler::cImmed, 0, 1);
            compiler->pushBackImmed($1);
        }
    |
    BEGINSTRING text ENDSTRING
        {
            $$ = Symbol::Text;
            // qDebug() << "Code: TXT" << *$2;
            compiler->pushBack(Compiler::cImmed, 0, 1);
            compiler->pushBackImmed($2);
        }
    ;


text:
    /* empty */
        {$$ = QString("");}
    |
    chars
        {$$ = $1;}
    ;

chars:
    CHAR
        {
            $$ = QString($1);
        }
    |
    chars CHAR
        {$$.append($2);}
    ;



paren_or_variable:
    '(' expression ')'
        {
            $$ = $2;
        }
    |
    ID
        {
            if (!compiler->hasSymbol($1)) {
                HANDLE_ERROR($1, Compiler::notdeclared);
            }
            Symbol* sym = compiler->symbol($1);
            Variable* var = dynamic_cast<Variable*>(sym);
            if (var) {
                // qDebug() << "Code:" << opname(Compiler::cVar) << sym->name();
                compiler->pushBack(Compiler::cVar, 0, 1);
                compiler->pushBack(var->index(), 0, 0);
            } else { // constant
                Constant* con = dynamic_cast<Constant*>(sym);
                if (con) {
                    // qDebug() << "Code:" << opname(Compiler::cImmed) << sym->name();
                    compiler->pushBack(Compiler::cImmed, 0, 1);
                    compiler->pushBackImmed(con->value());
                } else {
                    HANDLE_ERROR($1, Compiler::notvarorconst);
                }
            }
            $$ = sym->type();
        }
    ;

function_call:
    ID '(' parameters ')'
        {
            if (!compiler->hasSymbol($1)) {
                HANDLE_ERROR($1, Compiler::notdeclared);
            }
            Function* fun = dynamic_cast<Function*>(compiler->symbol($1));
            if (!fun) {
                HANDLE_ERROR($1, Compiler::notfunction);
            }
            $$ = fun->type();
            if (fun->argTypes().size() != $3.size()) {
                HANDLE_ERROR($1, Compiler::wrongargs);
            }
            // check the argument types
            for (int i = 0; i < fun->argTypes().size(); ++i) {
                int te = $3[i]; int ta = fun->argTypes()[i];
                if (ta == te) continue;
                if (ta == Symbol::Real && te == Symbol::Integer) continue;
                HANDLE_ERROR($1, Compiler::incompatibleargs);
            }
            // qDebug() << "Code:" << opname(Compiler::cFun) << fun->name();
            compiler->pushBack(Compiler::cFun, 0, 1 - $3.size());
            compiler->pushBack(fun->index(), 0, 0);
        }
    ;


parameters:
    /* empty */
        {$$.clear();}
    |
    arglist
        {$$ = $1;}
    ;

arglist:
    expression
        {
            $$.clear();
            $$.append($1);
        }
    |
    arglist ',' expression
        {$$.append($3);}
    ;

%%


const char* opname(int op) {

    struct {int op; const char* name;} names[] = {
        {'<', "\"<\""},
        {'>', "\">\""},
        {'+', "\"+\""},
        {'-', "\"-\""},
        {'*', "\"*\""},
        {'/', "\"/\""},
        {'!', "\"!\""},
        {BOR, "\"|\""},
        {BAND, "\"&\""},
        {EQ, "\"==\""},
        {NE, "\"!=\""},
        {LE, "\"<=\""},
        {GE, "\">=\""},
        {OR, "\"||\""},
        {AND, "\"&&\""},
        {Compiler::cVar, "VAR"},
        {Compiler::cImmed, "CON"},
        {Compiler::cFun, "FUN"},
        {-1, ""}
    };
    for (int i = 0; names[i].op != -1; i++) {
        if (names[i].op == op) return names[i].name;
    }
    Q_ASSERT(0);
    return 0;
}


unsigned int operation(int op) {

    struct {int op; unsigned int parserOp;} ops[] = {
        {'<', Compiler::cLess},
        {'>', Compiler::cGreater},
        {'+', Compiler::cAdd},
        {'-', Compiler::cSub},
        {'*', Compiler::cMul},
        {'/', Compiler::cDiv},
        {'!', Compiler::cNot},
        {BOR, Compiler::cBOr},
        {BAND, Compiler::cBAnd},
        {EQ, Compiler::cEqual},
        {NE, Compiler::cNEqual},
        {LE, Compiler::cLessOrEq},
        {GE, Compiler::cGreaterOrEq},
        {OR, Compiler::cOr},
        {AND, Compiler::cAnd},
        {0, 0}
    };
    for (int i = 0; ops[i].op != 0; i++) {
        if (ops[i].op == op) return ops[i].parserOp;
    }
    Q_ASSERT(0);
    return 0;
}


unsigned int lrtype(int l, int r) {

    struct {int l; int r; unsigned int lr;} ops[] = {
        {Symbol::Integer, Symbol::Integer, Compiler::cII},
        {Symbol::Integer, Symbol::Real, Compiler::cIS},
        {Symbol::Integer, Symbol::Vector, Compiler::cIV},
        {Symbol::Integer, Symbol::Matrix, Compiler::cIM},
        {Symbol::Integer, Symbol::Text, Compiler::cIT},
        {Symbol::Real, Symbol::Integer, Compiler::cSI},
        {Symbol::Real, Symbol::Real, Compiler::cSS},
        {Symbol::Real, Symbol::Vector, Compiler::cSV},
        {Symbol::Real, Symbol::Matrix, Compiler::cSM},
        {Symbol::Real, Symbol::Text, Compiler::cST},
        {Symbol::Vector, Symbol::Integer, Compiler::cVI},
        {Symbol::Vector, Symbol::Real, Compiler::cVS},
        {Symbol::Vector, Symbol::Vector, Compiler::cVV},
        {Symbol::Vector, Symbol::Matrix, Compiler::cVM},
        {Symbol::Vector, Symbol::Text, Compiler::cVT},
        {Symbol::Matrix, Symbol::Integer, Compiler::cMI},
        {Symbol::Matrix, Symbol::Real, Compiler::cMS},
        {Symbol::Matrix, Symbol::Vector, Compiler::cMV},
        {Symbol::Matrix, Symbol::Matrix, Compiler::cMM},
        {Symbol::Matrix, Symbol::Text, Compiler::cMT},
        {Symbol::Text, Symbol::Integer, Compiler::cTI},
        {Symbol::Text, Symbol::Real, Compiler::cTS},
        {Symbol::Text, Symbol::Vector, Compiler::cTV},
        {Symbol::Text, Symbol::Matrix, Compiler::cTM},
        {Symbol::Text, Symbol::Text, Compiler::cTT},
        {-1, -1, 0}
    };
    for (int i = 0; ops[i].l != -1; ++i) {
        if (ops[i].l == l && ops[i].r == r) return ops[i].lr;
    }
    Q_ASSERT(0);
    return 0;
}
