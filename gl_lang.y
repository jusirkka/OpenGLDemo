%{


extern "C"
{
    #include <stdio.h>

    int gl_lang_parse(void);
    int gl_lang_lex(void);
    void gl_lang_error(const char *);

    int gl_lang_wrap(void) {return 1;}

}


#include "parser.h"
#include "variable.h"
#include "constant.h"
#include "function.h"

#include <QtDebug>
#include <QStringList>
#include <QtGlobal>

// Error handling

enum {
    demo_err_declared,
    demo_err_notdeclared,
    demo_err_notvariable,
    demo_err_assincompatible,
    demo_err_incompatibletypes,
    demo_err_expectedinteger,
    demo_err_expectedintegerorreal,
    demo_err_notfunction,
    demo_err_wrongargs,
    demo_err_incompatibleargs,
    demo_err_notvarorconst,
    demo_err_expectedmatrix,
    demo_err_expectedvectorormatrix,
    demo_err_wrongcomps,
    demo_err_duplicate,
    demo_err_notavariable,
    demo_err_notasharedvariable,
    demo_err_notarealvariable,
    demo_err_notavectorvariable,
    demo_err_notamatrixvariable,
    demo_err_notatextvariable,
    demo_err_notaintegervariable,
    demo_err_expectedarithmetictype,
    demo_err_nottext,
};

static const char* explanations[] = {
    "%s has been already declared.",
    "%s has not been declared.",
    "%s is not a variable.",
    "incompatible types in assignment to %s.",
    "incompatible types in %s expression.",
    "expected integer in %s expression.",
    "expected integer or real in %s expression.",
    "%s is not a function.",
    "wrong number of arguments in %s.",
    "incompatible arguments in %s.",
    "%s is not a variable or constant.",
    "%s is not a matrix.",
    "%s is not a vector or matrix.",
    "wrong number of components in %s expression.",
    "duplicate declaration of %s.",
    "symbol %s is not a variable",
    "symbol %s is not a shared variable",
    "type of variable %s is not Real",
    "type of variable %s is not Vector",
    "type of variable %s is not Matrix",
    "type of variable %s is not Text",
    "type of variable %s is not Natural",
    "expected arithmetic type in %s expression",
    "%s expects a text expression",
};

char buffer[256];

#define HANDLE_ERROR(item, errnum) \
    {sprintf(buffer, explanations[errnum], item); \
    yyerror(buffer); \
    YYERROR;}

const char* opname(int);
unsigned int operation(int);
unsigned int lrtype(int, int);

using namespace Demo;

%}

%debug
%locations

%union
{
    int int_value;
    Math3D::Real real_value;
    char char_value;
    char string_value[1024];
    QStringList* p_string_list;
    Symbol::TypeList* p_int_list;
    QString* p_string;
}

%type <p_string_list> identifiers shared_identifiers
%destructor {delete ($$);} identifiers shared_identifiers

%type <p_int_list> parameters arglist
%destructor {delete ($$);} parameters arglist

%type <p_string> text chars
%destructor {delete($$);} text chars

%type <int_value> rhs simple_rhs cond_rhs cond_rhs_seq guard
%type <int_value> expression terms factors factor statement
%type <int_value> rel_op eq_op add_op sign and_op or_op
%type <int_value> literal paren_or_variable paren_or_variable_comp function_call



%token <int_value> INT
%token <real_value> FLOAT
%token <char_value> CHAR

%token <string_value> ID

%token VECTOR MATRIX TEXT NATURAL SHARED REAL EXECUTE

%nonassoc <int_value> '<' '>' EQ NE LE GE
%left <int_value> '+' '-' OR BOR
%left <int_value> '*' '/' AND BAND
%left <int_value> NEG '!'

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
    declaration SEP
    |
    assignment SEP
    |
    statement SEP
    ;

declaration:
    REAL identifiers
        {
            foreach (QString var, *($2)) {
                Parser::AddSymbol(new Var::Local::Real(var));
            }
        }
    |
    VECTOR identifiers
        {
            foreach (QString var, *($2)) {
                Parser::AddSymbol(new Var::Local::Vector(var));
            }
        }
    |
    MATRIX identifiers
        {
            foreach (QString var, *($2)) {
                Parser::AddSymbol(new Var::Local::Matrix(var));
            }
        }
    |
    TEXT identifiers
        {
            foreach (QString var, *($2)) {
                Parser::AddSymbol(new Var::Local::Text(var));
            }
        }
    |
    NATURAL identifiers
        {
            foreach (QString var, *($2)) {
                Parser::AddSymbol(new Var::Local::Natural(var));
            }
        }
    |
    SHARED REAL shared_identifiers
        {
            foreach (QString var, *($3)) {
                if (Parser::Symbols().contains(var)) {
                    if (Parser::Symbols()[var]->type() != Symbol::Real) {
                        HANDLE_ERROR(var.toUtf8().constData(), demo_err_notarealvariable);
                    }
                    dynamic_cast<Variable*>(Parser::Symbols()[var])->setUsed(true);
                } else {
                    Parser::AddSymbol(new Var::Shared::Real(var));
                }
            }
        }
    |
    SHARED VECTOR shared_identifiers
        {
            foreach (QString var, *($3)) {
                if (Parser::Symbols().contains(var)) {
                    if (Parser::Symbols()[var]->type() != Symbol::Vector) {
                        HANDLE_ERROR(var.toUtf8().constData(), demo_err_notavectorvariable);
                    }
                    dynamic_cast<Variable*>(Parser::Symbols()[var])->setUsed(true);
                } else {
                    Parser::AddSymbol(new Var::Shared::Vector(var));
                }
            }
        }
    |
    SHARED MATRIX shared_identifiers
        {
            foreach (QString var, *($3)) {
                if (Parser::Symbols().contains(var)) {
                    if (Parser::Symbols()[var]->type() != Symbol::Matrix) {
                        HANDLE_ERROR(var.toUtf8().constData(), demo_err_notamatrixvariable);
                    }
                    dynamic_cast<Variable*>(Parser::Symbols()[var])->setUsed(true);
                } else {
                    Parser::AddSymbol(new Var::Shared::Matrix(var));
                }
            }
        }
    |
    SHARED TEXT shared_identifiers
        {
            foreach (QString var, *($3)) {
                if (Parser::Symbols().contains(var)) {
                    if (Parser::Symbols()[var]->type() != Symbol::Text) {
                        HANDLE_ERROR(var.toUtf8().constData(), demo_err_notatextvariable);
                    }
                    dynamic_cast<Variable*>(Parser::Symbols()[var])->setUsed(true);
                } else {
                    Parser::AddSymbol(new Var::Shared::Text(var));
                }
            }
        }
    |
    SHARED NATURAL shared_identifiers
        {
            foreach (QString var, *($3)) {
                if (Parser::Symbols().contains(var)) {
                    if (Parser::Symbols()[var]->type() != Symbol::Integer) {
                        HANDLE_ERROR(var.toUtf8().constData(), demo_err_notaintegervariable);
                    }
                    dynamic_cast<Variable*>(Parser::Symbols()[var])->setUsed(true);
                } else {
                    Parser::AddSymbol(new Var::Shared::Natural(var));
                }
            }
        }
    ;


assignment:
    ID rhs
        {
            if (!Parser::Symbols().contains(QString($1))) {
                HANDLE_ERROR($1, demo_err_notdeclared);
            }
            Variable* var = dynamic_cast<Variable*>(Parser::Symbols()[QString($1)]);
            if (!var) {
                HANDLE_ERROR($1, demo_err_notvariable);
            }
            if (var->type() == $2 || (var->type() == Symbol::Real && $2 == Symbol::Integer)) {
                Parser::SetCode(var->name());
            } else {
                HANDLE_ERROR($1, demo_err_assincompatible);
            }
        }
    ;


statement:
    ID  parameters
        {
            if (!Parser::Symbols().contains(QString($1))) {
                HANDLE_ERROR($1, demo_err_notdeclared);
            }
            Function* fun = dynamic_cast<Function*>(Parser::Symbols()[QString($1)]);
            if (!fun) {
                HANDLE_ERROR($1, demo_err_notfunction);
            }
            $$ = fun->type();
            if (fun->argTypes().size() != $2->size()) {
                HANDLE_ERROR($1, demo_err_wrongargs);
            }
            // check the argument types
            for (int i = 0; i < fun->argTypes().size(); ++i) {
                int te = (*$2)[i]; int ta = fun->argTypes()[i];
                if (ta == te) continue;
                if (ta == Symbol::Real && te == Symbol::Integer) continue;
                HANDLE_ERROR($1, demo_err_incompatibleargs);
            }
            // qDebug() << "Code:" << opname(Parser::cFun) << fun->name();
            Parser::PushBack(Parser::cFun, 0, 1 - $2->size());
            Parser::PushBack(fun->index(), 0, 0);

            Parser::SetJump();
            Parser::SetCode("gl_result");
        }
    |
    EXECUTE expression
        {
            Function* emitter = dynamic_cast<Function*>(Parser::Symbols()["gl_emitter"]);
            if ($2 != Symbol::Text) {
                HANDLE_ERROR("Execute", demo_err_nottext);
            }
            Parser::PushBack(Parser::cImmed, 0, 1);
            Parser::PushBackImmed(Parser::Name());
            Parser::PushBack(Parser::cFun, 0, -1);
            Parser::PushBack(emitter->index(), 0, 0);

            Parser::SetJump();
            Parser::SetCode("gl_result");
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
                HANDLE_ERROR("assignment", demo_err_incompatibletypes);
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
            Parser::SetJump();
        }
    ;

guard:
    '|' expression
        {
            if ($2 != Symbol::Integer) {
                HANDLE_ERROR("guard", demo_err_expectedinteger);
            }
            $$ = $2;
            // qDebug() << "Code: GUARD";
            Parser::PushBack(Parser::cGuard, 0, 0);
            // reserve space for code and immed jumps
            Parser::PushBack(0, 0, 0);
            Parser::PushBack(0, 0, 0);
            Parser::InitJump();
        }
    ;



identifiers:
    ID
        {
            if (Parser::Symbols().contains(QString($1))) {
                HANDLE_ERROR($1, demo_err_declared);
            }
            $$ = new QStringList;
            $$->append(QString($1));
        }
    |
    identifiers ',' ID
        {
            if (Parser::Symbols().contains(QString($3))) {
                HANDLE_ERROR($3, demo_err_declared);
            }
            if ($$->contains(QString($3))) {
                HANDLE_ERROR($3, demo_err_duplicate);
            }
            $$->append(QString($3));
        }
    ;


shared_identifiers:
    ID
        {
            if (Parser::Symbols().contains(QString($1))) {
                Symbol* sym = Parser::Symbols()[QString($1)];
                Variable* var = dynamic_cast<Variable*>(sym);
                if (!var) {
                    HANDLE_ERROR($1, demo_err_notavariable);
                }
                if (!var->shared()) {
                    HANDLE_ERROR($1, demo_err_notasharedvariable);
                }
            }
            $$ = new QStringList;
            $$->append(QString($1));
        }
    |
    shared_identifiers ',' ID
        {
            if (Parser::Symbols().contains(QString($3))) {
                Symbol* sym = Parser::Symbols()[QString($3)];
                Variable* var = dynamic_cast<Variable*>(sym);
                if (!var) {
                    HANDLE_ERROR($3, demo_err_notavariable);
                }
                if (!var->shared()) {
                    HANDLE_ERROR($3, demo_err_notasharedvariable);
                }
            }
            if ($$->contains(QString($3))) {
                HANDLE_ERROR($3, demo_err_duplicate);
            }
            $$->append(QString($3));
        }
    ;

expression:
    terms
        {$$ = $1;}
    |
    terms rel_op terms
        {
            if (($1 == Symbol::Text || $3 == Symbol::Text)) {
                HANDLE_ERROR(opname($2), demo_err_expectedintegerorreal);
            }
            if (($1 != Symbol::Integer || $3 != Symbol::Integer) &&
                ($1 != Symbol::Real || $3 != Symbol::Integer) &&
                ($1 != Symbol::Integer || $3 != Symbol::Real) &&
                ($1 != Symbol::Real || $3 != Symbol::Real)) {
                HANDLE_ERROR(opname($2), demo_err_expectedintegerorreal);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            Parser::PushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    terms eq_op terms
        {
            if (($1 != $3) &&
                ($1 != Symbol::Integer || $3 != Symbol::Real) &&
                ($1 != Symbol::Real || $3 != Symbol::Integer)) {
                HANDLE_ERROR(opname($2), demo_err_incompatibletypes);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            Parser::PushBack(operation($2), lrtype($1, $3), -1);
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
                HANDLE_ERROR(opname($2), demo_err_expectedinteger);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            Parser::PushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    terms add_op factors
        {
            if (($1 == Symbol::Text || $3 == Symbol::Text)) {
                HANDLE_ERROR(opname($2), demo_err_expectedarithmetictype);
            }
            if (($1 != $3) &&
                ($1 != Symbol::Integer || $3 != Symbol::Real) &&
                ($1 != Symbol::Real || $3 != Symbol::Integer)) {
                HANDLE_ERROR(opname($2), demo_err_incompatibletypes);
            }
            if ($3 == Symbol::Real) {
                $$ = $3;
            } else {
                $$ = $1;
            }
            // qDebug() << "Code:" << opname($2);
            Parser::PushBack(operation($2), lrtype($1, $3), -1);
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
                HANDLE_ERROR(opname($2), demo_err_expectedinteger);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            Parser::PushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    factors '*' factor
        {
            if (($1 == Symbol::Text || $3 == Symbol::Text)) {
                HANDLE_ERROR(opname($2), demo_err_expectedarithmetictype);
            }
            if (($1 == Symbol::Vector && $3 == Symbol::Vector) ||
                ($1 == Symbol::Vector && $3 == Symbol::Matrix)) {
                HANDLE_ERROR(opname($2), demo_err_incompatibletypes);
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
            Parser::PushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    factors '/' factor
        {
            if (($1 == Symbol::Text || $3 == Symbol::Text)) {
                HANDLE_ERROR(opname($2), demo_err_expectedarithmetictype);
            }
            if ($1 == Symbol::Vector || $3 == Symbol::Vector ||
                $1 == Symbol::Matrix || $3 == Symbol::Matrix) {
                HANDLE_ERROR(opname($2), demo_err_incompatibletypes);
            }
            if ($1 == Symbol::Real || $3 == Symbol::Real) {
                $$ = Symbol::Real;
            } else {
                $$ = Symbol::Integer;
            }
            // qDebug() << "Code:" << opname($2);
            Parser::PushBack(operation($2), lrtype($1, $3), -1);
        }
    ;

and_op: BAND | AND
;


factor:
    '!' factor
        {
            if ($2 != Symbol::Integer) {
                HANDLE_ERROR(opname($1), demo_err_expectedinteger);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($1);
            Parser::PushBack(operation($1), 0, 0);
        }
    |
    sign factor
        {
            $$ = $2;
            // qDebug() << "Code: (unary)" << opname($1);
            if ($1 == '-') {
                Parser::PushBack(Parser::cNeg, lrtype(Symbol::Integer, $2), 0);
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
                HANDLE_ERROR("[]", demo_err_expectedinteger);
            }
            if ($1 != Symbol::Matrix && $1 != Symbol::Vector) {
                HANDLE_ERROR("expression", demo_err_expectedvectorormatrix);
            }
            // qDebug() << "Code: TAKE";
            Parser::PushBack(Parser::cTake, lrtype($1, Symbol::Integer), -1);
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
            Parser::PushBack(Parser::cImmed, 0, 1);
            Parser::PushBackImmed($1);
        }
    |
    FLOAT
        {
            $$ = Symbol::Real;
            // qDebug() << "Code: SCA" << $1;
            Parser::PushBack(Parser::cImmed, 0, 1);
            Parser::PushBackImmed($1);
        }
    |
    BEGINSTRING text ENDSTRING
        {
            $$ = Symbol::Text;
            // qDebug() << "Code: TXT" << *$2;
            Parser::PushBack(Parser::cImmed, 0, 1);
            Parser::PushBackImmed(*$2);
        }
    ;


text:
    /* empty */
        {$$ = new QString("");}
    |
    chars
        {$$ = $1;}
    ;

chars:
    CHAR
        {
            $$ = new QString($1);
        }
    |
    chars CHAR
        {$$->append($2);}
    ;



paren_or_variable:
    '(' expression ')'
        {
            $$ = $2;
        }
    |
    ID
        {
            if (!Parser::Symbols().contains(QString($1))) {
                HANDLE_ERROR($1, demo_err_notdeclared);
            }
            Symbol* sym = Parser::Symbols()[QString($1)];
            Variable* var = dynamic_cast<Variable*>(sym);
            if (var) {
                // qDebug() << "Code:" << opname(Parser::cVar) << sym->name();
                Parser::PushBack(Parser::cVar, 0, 1);
                Parser::PushBack(var->index(), 0, 0);
            } else { // constant
                Constant* con = dynamic_cast<Constant*>(sym);
                if (con) {
                    // qDebug() << "Code:" << opname(Parser::cImmed) << sym->name();
                    Parser::PushBack(Parser::cImmed, 0, 1);
                    Parser::PushBackImmed(con->value());
                } else {
                    HANDLE_ERROR($1, demo_err_notvarorconst);
                }
            }
            $$ = sym->type();
        }
    ;

function_call:
    ID '(' parameters ')'
        {
            if (!Parser::Symbols().contains(QString($1))) {
                HANDLE_ERROR($1, demo_err_notdeclared);
            }
            Function* fun = dynamic_cast<Function*>(Parser::Symbols()[QString($1)]);
            if (!fun) {
                HANDLE_ERROR($1, demo_err_notfunction);
            }
            $$ = fun->type();
            if (fun->argTypes().size() != $3->size()) {
                HANDLE_ERROR($1, demo_err_wrongargs);
            }
            // check the argument types
            for (int i = 0; i < fun->argTypes().size(); ++i) {
                int te = (*$3)[i]; int ta = fun->argTypes()[i];
                if (ta == te) continue;
                if (ta == Symbol::Real && te == Symbol::Integer) continue;
                HANDLE_ERROR($1, demo_err_incompatibleargs);
            }
            // qDebug() << "Code:" << opname(Parser::cFun) << fun->name();
            Parser::PushBack(Parser::cFun, 0, 1 - $3->size());
            Parser::PushBack(fun->index(), 0, 0);
        }
    ;


parameters:
    /* empty */
        {$$ = new Symbol::TypeList;}
    |
    arglist
        {$$ = $1;}
    ;

arglist:
    expression
        {
            $$ = new Symbol::TypeList;
            $$->append($1);
        }
    |
    arglist ',' expression
        {$$->append($3);}
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
        {Parser::cVar, "VAR"},
        {Parser::cImmed, "CON"},
        {Parser::cFun, "FUN"},
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
        {'<', Parser::cLess},
        {'>', Parser::cGreater},
        {'+', Parser::cAdd},
        {'-', Parser::cSub},
        {'*', Parser::cMul},
        {'/', Parser::cDiv},
        {'!', Parser::cNot},
        {BOR, Parser::cBOr},
        {BAND, Parser::cBAnd},
        {EQ, Parser::cEqual},
        {NE, Parser::cNEqual},
        {LE, Parser::cLessOrEq},
        {GE, Parser::cGreaterOrEq},
        {OR, Parser::cOr},
        {AND, Parser::cAnd},
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
        {Symbol::Integer, Symbol::Integer, Parser::cII},
        {Symbol::Integer, Symbol::Real, Parser::cIS},
        {Symbol::Integer, Symbol::Vector, Parser::cIV},
        {Symbol::Integer, Symbol::Matrix, Parser::cIM},
        {Symbol::Integer, Symbol::Text, Parser::cIT},
        {Symbol::Real, Symbol::Integer, Parser::cSI},
        {Symbol::Real, Symbol::Real, Parser::cSS},
        {Symbol::Real, Symbol::Vector, Parser::cSV},
        {Symbol::Real, Symbol::Matrix, Parser::cSM},
        {Symbol::Real, Symbol::Text, Parser::cST},
        {Symbol::Vector, Symbol::Integer, Parser::cVI},
        {Symbol::Vector, Symbol::Real, Parser::cVS},
        {Symbol::Vector, Symbol::Vector, Parser::cVV},
        {Symbol::Vector, Symbol::Matrix, Parser::cVM},
        {Symbol::Vector, Symbol::Text, Parser::cVT},
        {Symbol::Matrix, Symbol::Integer, Parser::cMI},
        {Symbol::Matrix, Symbol::Real, Parser::cMS},
        {Symbol::Matrix, Symbol::Vector, Parser::cMV},
        {Symbol::Matrix, Symbol::Matrix, Parser::cMM},
        {Symbol::Matrix, Symbol::Text, Parser::cMT},
        {Symbol::Text, Symbol::Integer, Parser::cTI},
        {Symbol::Text, Symbol::Real, Parser::cTS},
        {Symbol::Text, Symbol::Vector, Parser::cTV},
        {Symbol::Text, Symbol::Matrix, Parser::cTM},
        {Symbol::Text, Symbol::Text, Parser::cTT},
        {-1, -1, 0}
    };
    for (int i = 0; ops[i].l != -1; ++i) {
        if (ops[i].l == l && ops[i].r == r) return ops[i].lr;
    }
    Q_ASSERT(0);
    return 0;
}
