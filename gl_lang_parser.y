%{


#include "gl_lang_parser_interface.h"
#include "constant.h"

int gl_lang_lex(Demo::GL::ValueType*, Demo::GL::LocationType*, yyscan_t);

const char* opname(int);
unsigned int operation(int);
unsigned int lrtype(int, int);

using namespace Demo;
using Demo::GL::Parser;

#define HANDLE_ERROR(item, errnum) {parser->createError(item, errnum); YYERROR;}
#define HANDLE_COMPLETION(var, mask) {if (parser->createCompletion(var, mask)) YYERROR;}

%}

%locations
%define parse.trace

%define api.prefix {gl_lang_}
%define api.pure full
%define api.value.type {Demo::GL::ValueType}

%parse-param {Demo::GL::Parser* parser}
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

%token <v_identifier> ID

%token VECTOR MATRIX TEXT NATURAL SHARED REAL EXECUTE FROM IMPORT WHILE ENDWHILE IF ELSE ENDIF

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
    declaration_and_assignment SEP
    |
    statement SEP
    ;


import_from:
    FROM BEGINSTRING text ENDSTRING IMPORT variables
        {
            for (auto& name: $6) {
                if (parser->isExported(name, $3)) {
                    parser->addImported(name, $3);
                } else {
                    HANDLE_ERROR(name, Parser::notimported);
                }
            }
        }
    ;


declaration:
    shared typename variables
        {
            for (auto& name: $3) {
                parser->addVariable(Var::Create($2, name, $1));
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
            if (parser->hasSymbol($1.name)) {
                HANDLE_ERROR($1.name, Parser::declared);
            }
            $$.clear();
            $$.append($1.name);
        }
    |
    variables ',' ID
        {
            if (parser->hasSymbol($3.name)) {
                HANDLE_ERROR($3.name, Parser::declared);
            }
            if ($$.contains($3.name)) {
                HANDLE_ERROR($3.name, Parser::duplicate);
            }
            $$.append($3.name);
        }
    ;



assignment:
    ID rhs
        {
            HANDLE_COMPLETION($1, Parser::CompleteVariables);
            if (!parser->hasSymbol($1.name)) {
                HANDLE_ERROR($1.name, Parser::notdeclared);
            }
            Variable* var = dynamic_cast<Variable*>(parser->symbol($1.name));
            if (!var) {
                HANDLE_ERROR($1.name, Parser::notvariable);
            }
            if (parser->isImported(var)) {
                HANDLE_ERROR($1.name, Parser::assimported);
            }
            if (var->type() == $2 || (var->type() == Symbol::Real && $2 == Symbol::Integer)) {
                parser->setCode(var->name());
            } else {
                HANDLE_ERROR($1.name, Parser::assincompatible);
            }
        }
    ;

declaration_and_assignment:
    shared typename ID rhs
        {
            if (parser->hasSymbol($3.name)) {
                HANDLE_ERROR($3.name, Parser::declared);
            }
            Variable* var = Var::Create($2, $3.name, $1);
            if (var->type() == $4 || (var->type() == Symbol::Real && $4 == Symbol::Integer)) {
                parser->addVariable(var);
                parser->setCode(var->name());
            } else {
                HANDLE_ERROR($3.name, Parser::assincompatible);
            }
        }
    ;

statement:
    ID parameters
        {
            if ($2.isEmpty()) {
                HANDLE_COMPLETION($1, Parser::CompleteFVR);
            } else {
                HANDLE_COMPLETION($1, Parser::CompleteFunctions);
            }
            if (!parser->hasSymbol($1.name)) {
                HANDLE_ERROR($1.name, Parser::notdeclared);
            }
            Function* fun = dynamic_cast<Function*>(parser->symbol($1.name));
            if (!fun) {
                HANDLE_ERROR($1.name, Parser::notfunction);
            }
            $$ = fun->type();
            if (fun->argTypes().size() != $2.size()) {
                HANDLE_ERROR($1.name, Parser::wrongargs);
            }
            // check the argument types
            for (int i = 0; i < fun->argTypes().size(); ++i) {
                int te = $2[i]; int ta = fun->argTypes()[i];
                if (ta == te) continue;
                if (ta == Symbol::Real && te == Symbol::Integer) continue;
                HANDLE_ERROR($1.name, Parser::incompatibleargs);
            }
            // qDebug() << "Code:" << opname(Parser::cFun) << fun->name() << fun->index();
            parser->pushBack(Parser::cFun, 0, 1 - $2.size());
            parser->pushBack(fun->index(), 0, 0);

            parser->setJump();
            parser->setCode("gl_result");
        }
    |
    EXECUTE BEGINSTRING text ENDSTRING
        {
            if (parser->isScript($3)) {
                parser->addSubscript($3);
            } else {
                HANDLE_ERROR($3, Parser::scriptnotfound);
            }
            parser->pushBack(Parser::cImmed, 0, 1);
            parser->pushBackImmed($3);

            Function* dispatcher = dynamic_cast<Function*>(parser->symbol("dispatch"));
            parser->pushBack(Parser::cFun, 0, 0);
            parser->pushBack(dispatcher->index(), 0, 0);

            parser->setJump();
            parser->setCode("gl_result");
        }
    |
    WHILE expression
        {
            if ($2 != Symbol::Integer) {
                HANDLE_ERROR("While", Parser::expectedinteger);
            }
            parser->beginWhile();
        }
    |
    ENDWHILE
        {
           if (!parser->endWhile()) {
               HANDLE_ERROR("Endwhile", Parser::roguestatement);
           }
        }
    |
    IF expression
        {
            if ($2 != Symbol::Integer) {
                HANDLE_ERROR("If", Parser::expectedinteger);
            }
            parser->beginIf();
        }
    |
    ELSE
        {
            if (!parser->addElse()) {
                HANDLE_ERROR("Else", Parser::roguestatement);
            }
        }
    |
    ENDIF
        {
           if (!parser->endIf()) {
               HANDLE_ERROR("Endif", Parser::roguestatement);
           }
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
                HANDLE_ERROR("assignment", Parser::incompatibletypes);
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
            parser->setJump();
        }
    ;

guard:
    '@' expression
        {
            if ($2 != Symbol::Integer) {
                HANDLE_ERROR("guard", Parser::expectedinteger);
            }
            $$ = $2;
            // qDebug() << "Code: GUARD";
            parser->pushBack(Parser::cGuard, 0, 0);
            // reserve space for code and immed jumps
            parser->pushBack(0, 0, 0);
            parser->pushBack(0, 0, 0);
            parser->initJump();
        }
    ;






expression:
    terms
        {$$ = $1;}
    |
    terms rel_op terms
        {
            if (($1 == Symbol::Text || $3 == Symbol::Text)) {
                HANDLE_ERROR(opname($2), Parser::expectedintegerorreal);
            }
            if (($1 != Symbol::Integer || $3 != Symbol::Integer) &&
                ($1 != Symbol::Real || $3 != Symbol::Integer) &&
                ($1 != Symbol::Integer || $3 != Symbol::Real) &&
                ($1 != Symbol::Real || $3 != Symbol::Real)) {
                HANDLE_ERROR(opname($2), Parser::expectedintegerorreal);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            parser->pushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    terms eq_op terms
        {
            if (($1 != $3) &&
                ($1 != Symbol::Integer || $3 != Symbol::Real) &&
                ($1 != Symbol::Real || $3 != Symbol::Integer)) {
                HANDLE_ERROR(opname($2), Parser::incompatibletypes);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            parser->pushBack(operation($2), lrtype($1, $3), -1);
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
                HANDLE_ERROR(opname($2), Parser::expectedinteger);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            parser->pushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    terms add_op factors
        {
            if ($1 == Symbol::Text && $3 == Symbol::Text) {
                if ($2 != '+') {
                    HANDLE_ERROR(opname($2), Parser::expectedtextadd);
                }
            } else if ($1 == Symbol::Text || $3 == Symbol::Text) {
                HANDLE_ERROR(opname($2), Parser::expectedarithmetictype);
            }
            if (($1 != $3) &&
                ($1 != Symbol::Integer || $3 != Symbol::Real) &&
                ($1 != Symbol::Real || $3 != Symbol::Integer)) {
                HANDLE_ERROR(opname($2), Parser::incompatibletypes);
            }
            if ($1 == Symbol::Real || $3 == Symbol::Real) {
                $$ = Symbol::Real;
            } else {
                $$ = $1;
            }
            // qDebug() << "Code:" << opname($2);
            parser->pushBack(operation($2), lrtype($1, $3), -1);
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
                HANDLE_ERROR(opname($2), Parser::expectedinteger);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($2);
            parser->pushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    factors '*' factor
        {
            if (($1 == Symbol::Text || $3 == Symbol::Text)) {
                HANDLE_ERROR(opname($2), Parser::expectedarithmetictype);
            }
            if (($1 == Symbol::Vector && $3 == Symbol::Vector) ||
                ($1 == Symbol::Vector && $3 == Symbol::Matrix)) {
                HANDLE_ERROR(opname($2), Parser::incompatibletypes);
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
            parser->pushBack(operation($2), lrtype($1, $3), -1);
        }
    |
    factors '/' factor
        {
            if (($1 == Symbol::Text || $3 == Symbol::Text)) {
                HANDLE_ERROR(opname($2), Parser::expectedarithmetictype);
            }
            if ($1 == Symbol::Vector || $3 == Symbol::Vector ||
                $1 == Symbol::Matrix || $3 == Symbol::Matrix) {
                HANDLE_ERROR(opname($2), Parser::incompatibletypes);
            }
            if ($1 == Symbol::Real || $3 == Symbol::Real) {
                $$ = Symbol::Real;
            } else {
                $$ = Symbol::Integer;
            }
            // qDebug() << "Code:" << opname($2);
            parser->pushBack(operation($2), lrtype($1, $3), -1);
        }
    ;

and_op: BAND | AND
;


factor:
    '!' factor
        {
            if ($2 != Symbol::Integer) {
                HANDLE_ERROR(opname($1), Parser::expectedinteger);
            }
            $$ = Symbol::Integer;
            // qDebug() << "Code:" << opname($1);
            parser->pushBack(operation($1), 0, 0);
        }
    |
    sign factor
        {
            $$ = $2;
            // qDebug() << "Code: (unary)" << opname($1);
            if ($1 == '-') {
                parser->pushBack(Parser::cNeg, lrtype(Symbol::Integer, $2), 0);
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
                HANDLE_ERROR("[]", Parser::expectedinteger);
            }
            if ($1 != Symbol::Matrix && $1 != Symbol::Vector) {
                HANDLE_ERROR("expression", Parser::expectedvectorormatrix);
            }
            // qDebug() << "Code: TAKE";
            parser->pushBack(Parser::cTake, lrtype($1, Symbol::Integer), -1);
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
            parser->pushBack(Parser::cImmed, 0, 1);
            parser->pushBackImmed($1);
        }
    |
    FLOAT
        {
            $$ = Symbol::Real;
            // qDebug() << "Code: SCA" << $1;
            parser->pushBack(Parser::cImmed, 0, 1);
            parser->pushBackImmed($1);
        }
    |
    BEGINSTRING text ENDSTRING
        {
            $$ = Symbol::Text;
            // qDebug() << "Code: TXT" << $2;
            parser->pushBack(Parser::cImmed, 0, 1);
            parser->pushBackImmed($2);
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
            HANDLE_COMPLETION($1, Parser::CompleteFVC);
            if (!parser->hasSymbol($1.name)) {
                HANDLE_ERROR($1.name, Parser::notdeclared);
            }
            Symbol* sym = parser->symbol($1.name);
            Variable* var = dynamic_cast<Variable*>(sym);
            if (var) {
                // qDebug() << "Code:" << opname(Parser::cVar) << sym->name();
                parser->pushBack(Parser::cVar, 0, 1);
                parser->pushBack(var->index(), 0, 0);
            } else { // constant
                Constant* con = dynamic_cast<Constant*>(sym);
                if (con) {
                    // qDebug() << "Code:" << opname(Parser::cImmed) << sym->name();
                    parser->pushBack(Parser::cImmed, 0, 1);
                    parser->pushBackImmed(con->value());
                } else {
                    HANDLE_ERROR($1.name, Parser::notvarorconst);
                }
            }
            $$ = sym->type();
        }
    ;

function_call:
    ID '(' parameters ')'
        {
            HANDLE_COMPLETION($1, Parser::CompleteFunctions);
            if (!parser->hasSymbol($1.name)) {
                HANDLE_ERROR($1.name, Parser::notdeclared);
            }
            Function* fun = dynamic_cast<Function*>(parser->symbol($1.name));
            if (!fun) {
                HANDLE_ERROR($1.name, Parser::notfunction);
            }
            $$ = fun->type();
            if (fun->argTypes().size() != $3.size()) {
                HANDLE_ERROR($1.name, Parser::wrongargs);
            }
            // check the argument types
            for (int i = 0; i < fun->argTypes().size(); ++i) {
                int te = $3[i]; int ta = fun->argTypes()[i];
                if (ta == te) continue;
                if (ta == Symbol::Real && te == Symbol::Integer) continue;
                HANDLE_ERROR($1.name, Parser::incompatibleargs);
            }
            // qDebug() << "Code:" << opname(Parser::cFun) << fun->name();
            parser->pushBack(Parser::cFun, 0, 1 - $3.size());
            parser->pushBack(fun->index(), 0, 0);
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
