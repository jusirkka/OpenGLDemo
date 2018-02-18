%{


#include "gl_lang_parser_interface.h"
#include "constant.h"
#include "typedef.h"
#include "operation.h"

int gl_lang_lex(Demo::GL::ValueType*, Demo::GL::LocationType*, yyscan_t);


using namespace Demo;
using Demo::GL::Parser;

#define CF Parser::CF
#define CV Parser::CV
#define CC Parser::CC
#define CT Parser::CT
#define CR Parser::CR

#define HANDLE_ERROR(item, detail) {parser->createError(item, detail); YYERROR;}
#define HANDLE_COMPLETION(var, mask) {if (parser->createCompletion(var, mask)) YYERROR;}

#define DECLARED_MSG QStringLiteral("%1 has been already declared.")
#define NOT_DECLARED_MSG QStringLiteral("%1 has not been declared.")
#define NOT_IMPORTED_MSG QStringLiteral("variable %1 has not been exported.")
#define NOT_TYPE_MSG QStringLiteral("%1 is not a type.")
#define DUPLICATE_MSG QStringLiteral("duplicate declaration of %1.")
#define NOT_VARIABLE_MSG QStringLiteral("%1 is not a variable.")
#define ASS_IMPORTED_MSG QStringLiteral("cannot assign to imported variable %1.")
#define ASS_INCOMPATIBLE_MSG QStringLiteral("incompatible types in assignment to %1.")
#define NOT_FUNCTION_MSG QStringLiteral("%1 is not a function.")
#define WRONG_ARGS_MSG QStringLiteral("wrong number of arguments in %1.")
#define INCOMPATIBLE_ARGS_MSG QStringLiteral("incompatible arguments in %1.")
#define SCRIPT_NOT_FOUND_MSG QStringLiteral(R"(script "%1" not found)")
#define NOT_INTEGER_MSG QStringLiteral("expected integer in %1 expression.")
#define ROGUE_STATEMENT_MSG QStringLiteral("unmatching %1")
#define INCOMPATIBLE_TYPES_MSG QStringLiteral("incompatible types in %1 expression.")
#define NOT_VAR_CONST_MSG QStringLiteral("%1 is not a variable or constant.")
#define TOO_MANY_VARS_MSG QStringLiteral("too many variables in %1.")

%}

%locations
%define parse.trace

%define api.prefix {gl_lang_}
%define api.pure full
%define api.value.type {Demo::GL::ValueType}

%parse-param {Demo::GL::Parser* parser}
%parse-param {yyscan_t scanner}
%lex-param {yyscan_t scanner}


%type <v_string_list> symbols variables memberseq

%type <v_type_list> parameters arglist recorditems

%type <v_string> text chars

%type <v_type> rhs simple_rhs cond_rhs cond_rhs_seq guard
%type <v_type> expression terms factors factor statement
%type <v_oper> comp_op add_op mul_op unary_op member_op
%type <v_new_type> typedef membertype typespec
%type <v_named_type_list> member members
%type <v_array_item> arrayitems arrayseq
%type <v_ref_path> refpath
%type <v_ref_path_item> refpathitem


%token <v_int> INT
%token <v_real> FLOAT
%token <v_char> CHAR

%token <v_identifier> ID

%token SHARED EXECUTE FROM IMPORT WHILE ENDWHILE IF ELSE ELSIF ENDIF ARRAY OF
%token UNK BEGINSTRING ENDSTRING SEP TYPE VAR RECORD

%token <v_int> '.'
%token <v_int> '<' '>' EQ NE LE GE
%token <v_int> '+' '-' OR BOR
%token <v_int> '*' '/' AND BAND
%token <v_int> NEG '!'

%%

input: elements;

elements: element | elements element;

element: SEP | import_from SEP | typedecl SEP | declaration SEP | assignment SEP;
element: declaration_and_assignment SEP | statement SEP;


import_from: FROM BEGINSTRING text ENDSTRING symbols {
  for (auto& name: $5) {
    if (parser->isExported(name, $3)) {
      parser->addImported(name, $3);
    } else {
      HANDLE_ERROR(name, NOT_IMPORTED_MSG);
    }
  }
};


symbols: IMPORT ID {
  if (parser->hasSymbol($2.name)) {
    HANDLE_ERROR($2.name, DECLARED_MSG);
  }
  $$.clear();
  $$.append($2.name);
};

symbols: symbols ',' ID {
  if (parser->hasSymbol($3.name)) {
    HANDLE_ERROR($3.name, DECLARED_MSG);
  }
  if ($$.contains($3.name)) {
    HANDLE_ERROR($3.name, DUPLICATE_MSG);
  }
  $$.append($3.name);
};

typedecl: TYPE ID '=' typedef {
  if (parser->hasSymbol($2.name)) {
    delete $4;
    HANDLE_ERROR($2.name, DECLARED_MSG);
  }
  parser->addSymbol(new Typedef($2.name, $4));
};

typedef: ARRAY OF typedef {
  $$ = new ArrayType($3);
};

typedef: RECORD '(' members ')' {
  $$ = new RecordType($3.names, $3.types);
};

typedef: ID {
  HANDLE_COMPLETION($1, CR | CT);
  if (!parser->hasSymbol($1.name)) {
    HANDLE_ERROR($1.name, NOT_DECLARED_MSG);
  }
  auto typ = dynamic_cast<Typedef*>(parser->symbol($1.name));
  if (!typ) {
    HANDLE_ERROR($1.name, NOT_TYPE_MSG);
  }
  $$ = typ->type()->clone();
};

members: member {$$ = $1;};

members: members ',' member {
  for (auto name: $3.names) {
    if ($$.names.contains(name)) {
      qDeleteAll($$.types);
      qDeleteAll($3.types);
      HANDLE_ERROR(name, DUPLICATE_MSG);
    }
  }
  $$.names += $3.names;
  $$.types += $3.types;
};

member: memberseq membertype {
  $$.names.clear();
  $$.types.clear();
  for (auto name: $1) {
    $$.names << name;
    $$.types << $2->clone();
  }
  delete $2;
};

memberseq: ID {
  $$.clear();
  $$.append($1.name);
};

memberseq: memberseq ',' ID {
  if ($$.contains($3.name)) {
    HANDLE_ERROR($3.name, DUPLICATE_MSG);
  }
  $$.append($3.name);
};

membertype: ':' typedef {$$ = $2;};


declaration: variables typespec {
  for (auto name: $1) {
      parser->addSymbol(new LocalVar(name, $2->clone()));
  }
  delete $2;
};

declaration: SHARED variables typespec {
  for (auto name: $2) {
      parser->addSymbol(new SharedVar(name, $3->clone()));
  }
  delete $3;
};


variables: VAR ID {
  if (parser->hasSymbol($2.name)) {
    HANDLE_ERROR($2.name, DECLARED_MSG);
  }
  $$.clear();
  $$.append($2.name);
};

variables: variables ',' ID {
  if (parser->hasSymbol($3.name)) {
    HANDLE_ERROR($3.name, DECLARED_MSG);
  }
  if ($$.contains($3.name)) {
    HANDLE_ERROR($3.name, DUPLICATE_MSG);
  }
  $$.append($3.name);
};

typespec: ':' typedef {$$ = $2;};

assignment: ID rhs {
  if (!parser->hasSymbol($1.name)) {
    HANDLE_ERROR($1.name, NOT_DECLARED_MSG);
  }
  auto var = dynamic_cast<Variable*>(parser->symbol($1.name));
  if (!var) {
    HANDLE_ERROR($1.name, NOT_VARIABLE_MSG);
  }
  if (parser->isImported(var)) {
    HANDLE_ERROR($1.name, ASS_IMPORTED_MSG);
  }
  if (var->type()->assignable($2)) {
    parser->pushBack(Parser::cAss, 0, 0);
    parser->pushBack(var->index(), 0, 0);
    parser->assignment();
  } else {
    HANDLE_ERROR($1.name, ASS_INCOMPATIBLE_MSG);
  }
};

assignment: ID refpath rhs {
  if (!parser->hasSymbol($1.name)) {
    HANDLE_ERROR($1.name, NOT_DECLARED_MSG);
  }
  auto var = dynamic_cast<Variable*>(parser->symbol($1.name));
  if (!var) {
    HANDLE_ERROR($1.name, NOT_VARIABLE_MSG);
  }
  if (parser->isImported(var)) {
    HANDLE_ERROR($1.name, ASS_IMPORTED_MSG);
  }

  const Type* left = var->type();
  int pathSize = $2.types.size();
  do {
    const Type* right = $2.types.takeFirst();
    const Operation* op = $2.operations.takeFirst();
    try {
      op->check(left, right);
      auto rec = dynamic_cast<const RecordType*>(left);
      auto sel = dynamic_cast<const Selector*>(right);
      if (rec && sel) parser->setImmed($2.immeds.takeFirst(), rec->index(sel->name()));
      left = op->type(left, right);
    } catch (OpError e) {
      HANDLE_ERROR(op->name(), e.msg());
    }
  } while (!$2.types.isEmpty());

  if (left->assignable($3)) {
    parser->pushBack(Parser::cAssPath, 0, - pathSize);
    parser->pushBack(var->index(), 0, 0);
    parser->pushBack(pathSize, 0, 0);
    parser->assignment();
  } else {
    HANDLE_ERROR($1.name, ASS_INCOMPATIBLE_MSG);
  }
};

declaration_and_assignment: variables typespec rhs {
  if ($1.size() != 1) {
    HANDLE_ERROR("assignment", TOO_MANY_VARS_MSG);
  }
  QString name = $1.first();
  if ($2->assignable($3)) {
    parser->addSymbol(new LocalVar(name, $2));
    auto v = dynamic_cast<Variable*>(parser->symbol(name));
    parser->pushBack(Parser::cAss, 0, 0);
    parser->pushBack(v->index(), 0, 0);
    parser->assignment();
  } else {
    HANDLE_ERROR(name, ASS_INCOMPATIBLE_MSG);
  }
};

declaration_and_assignment: SHARED variables typespec rhs {
  if ($2.size() != 1) {
    HANDLE_ERROR("assignment", TOO_MANY_VARS_MSG);
  }
  QString name = $2.first();
  if ($3->assignable($4)) {
    parser->addSymbol(new SharedVar(name, $3));
    auto v = dynamic_cast<Variable*>(parser->symbol(name));
    parser->pushBack(Parser::cAss, 0, 0);
    parser->pushBack(v->index(), 0, 0);
    parser->assignment();
  } else {
    HANDLE_ERROR(name, ASS_INCOMPATIBLE_MSG);
  }
};

statement: ID parameters {
  if ($2.isEmpty()) {
    HANDLE_COMPLETION($1, CR|CV|CF);
  } else {
    HANDLE_COMPLETION($1, CF);
  }
  if (!parser->hasSymbol($1.name)) {
    HANDLE_ERROR($1.name, NOT_DECLARED_MSG);
  }
  auto fun = dynamic_cast<Function*>(parser->symbol($1.name));
  if (!fun) {
    HANDLE_ERROR($1.name, NOT_FUNCTION_MSG);
  }
  $$ = fun->type();
  if (fun->argTypes().size() != $2.size()) {
    HANDLE_ERROR($1.name, WRONG_ARGS_MSG);
  }
  // check the argument types
  for (int i = 0; i < fun->argTypes().size(); ++i) {
    if (fun->argTypes()[i]->id() == 0 && $2[i]->id() != 0) continue; // NullType: accept all types
    if (fun->argTypes()[i]->assignable($2[i])) continue;
    HANDLE_ERROR($1.name, INCOMPATIBLE_ARGS_MSG);
  }
  // qDebug() << "Code:" << opname(Parser::cFun) << fun->name() << fun->index();
  parser->pushBack(Parser::cFun, 0, 1 - $2.size());
  parser->pushBack(fun->index(), 0, 0);
  auto v = dynamic_cast<Variable*>(parser->symbol("gl_result"));
  parser->pushBack(Parser::cAss, 0, 0);
  parser->pushBack(v->index(), 0, 0);
  parser->assignment();
};

statement: EXECUTE BEGINSTRING text ENDSTRING {
  if (parser->isScript($3)) {
    parser->addSubscript($3);
  } else {
    HANDLE_ERROR($3, SCRIPT_NOT_FOUND_MSG);
  }
  parser->pushBack(Parser::cImmed, 0, 1);
  parser->pushBackImmed($3);

  auto dispatcher = dynamic_cast<Function*>(parser->symbol("dispatch"));
  parser->pushBack(Parser::cFun, 0, 0);
  parser->pushBack(dispatcher->index(), 0, 0);

  auto v = dynamic_cast<Variable*>(parser->symbol("gl_result"));
  parser->pushBack(Parser::cAss, 0, 0);
  parser->pushBack(v->index(), 0, 0);
  parser->assignment();
};


statement: WHILE expression {
  if ($2->id() != Type::Integer) {
    HANDLE_ERROR("While", NOT_INTEGER_MSG);
  }
  parser->beginWhile();
};

statement: ENDWHILE {
  if (!parser->endWhile()) {
    HANDLE_ERROR("Endwhile", ROGUE_STATEMENT_MSG);
  }
};

statement: IF expression {
  if ($2->id() != Type::Integer) {
    HANDLE_ERROR("If", NOT_INTEGER_MSG);
  }
  parser->beginIf();
};

statement: ELSE {
  if (!parser->addElse()) {
    HANDLE_ERROR("Else", ROGUE_STATEMENT_MSG);
  }
};

statement: ELSIF expression {
  if ($2->id() != Type::Integer) {
    HANDLE_ERROR("Elsif", NOT_INTEGER_MSG);
  }
  if (!parser->addElsif()) {
    HANDLE_ERROR("Elsif", ROGUE_STATEMENT_MSG);
  }
};

statement: ENDIF {
  if (!parser->endIf()) {
    HANDLE_ERROR("Endif", ROGUE_STATEMENT_MSG);
  }
};

rhs: simple_rhs;

rhs: cond_rhs_seq {
  $$ = $1;
  parser->finalizeJumps();
};

cond_rhs_seq: cond_rhs {$$ = $1;};
cond_rhs_seq: cond_rhs_seq cond_rhs {
  if (!$1->assignable($2) && !$2->assignable($1)) {
    HANDLE_ERROR("assignment", INCOMPATIBLE_TYPES_MSG);
  }
  if ($1->assignable($2)) {
    $$ = $1;
  } else {
    $$ = $2;
  }
};

cond_rhs: guard simple_rhs {
  $$ = $2;
  // qDebug() << "Code: JUMP";
  parser->pushBack(Parser::cJump, 0, 0);
  // reserve space for unconditional code and immed jumps:
  // jump to end of guard sequence after executing assignment
  parser->pushBack(0, 0, 0);
  parser->pushBack(0, 0, 0);
  parser->setJump();
};


guard: '@' expression {
  if ($2->id() != Type::Integer) {
    HANDLE_ERROR("guard", NOT_INTEGER_MSG);
  }
  $$ = $2;
  // qDebug() << "Code: GUARD";
  parser->pushBack(Parser::cGuard, 0, 0);
  // reserve space for conditional code and immed jumps:
  // jump to next guard expression if this guard expression is false
  parser->pushBack(0, 0, 0);
  parser->pushBack(0, 0, 0);
  parser->initJump();
};

simple_rhs: '=' expression {$$ = $2;};

expression: terms {$$ = $1;};

expression: terms comp_op terms {
  try {
    $2->check($1, $3);
    // qDebug() << "Code:" << $2->name();
    parser->pushBack($2->code(), Parser::LRType($1, $3), -1);
    $$ = $2->type($1, $3);
  } catch (OpError e) {
    HANDLE_ERROR($2->name(), e.msg());
  }
};


comp_op: '>' {$$ = Parser::Op($1);};
comp_op: '<' {$$ = Parser::Op($1);};
comp_op: GE {$$ = Parser::Op($1);};
comp_op: LE {$$ = Parser::Op($1);};
comp_op: EQ {$$ = Parser::Op($1);};
comp_op: NE {$$ = Parser::Op($1);};


terms: factors {$$ = $1;};

terms: terms add_op factors {
try {
  $2->check($1, $3);
  // qDebug() << "Code:" << $2->name();
  parser->pushBack($2->code(), Parser::LRType($1, $3), -1);
  $$ = $2->type($1, $3);
} catch (OpError e) {
  HANDLE_ERROR($2->name(), e.msg());
}};


add_op: '+' {$$ = Parser::Op($1);};
add_op: '-' {$$ = Parser::Op($1);};
add_op: BOR {$$ = Parser::Op($1);};
add_op: OR {$$ = Parser::Op($1);};



factors: factor {$$ = $1;};

factors: factors mul_op factor {
try {
  $2->check($1, $3);
  // qDebug() << "Code:" << $2->name();
  parser->pushBack($2->code(), Parser::LRType($1, $3), -1);
  $$ = $2->type($1, $3);
} catch (OpError e) {
  HANDLE_ERROR($2->name(), e.msg());
}};


mul_op: '*' {$$ = Parser::Op($1);};
mul_op: '/' {$$ = Parser::Op($1);};
mul_op: BAND {$$ = Parser::Op($1);};
mul_op: AND {$$ = Parser::Op($1);};


factor: unary_op factor {
try {
  $1->check($2);
  // qDebug() << "Code:" << $1->name();
  if ($1->name() != "+") parser->pushBack($1->code(), Parser::LRType(Parser::Integer(), $2), 0);
  $$ = $1->type($2);
} catch (OpError e) {
  HANDLE_ERROR($1->name(), e.msg());
}};


unary_op: '!' {$$ = Parser::Op($1);};
unary_op: '+' {$$ = Parser::Op(TOKEN_PLUS);};
unary_op: '-' {$$ = Parser::Op(TOKEN_MINUS);} %prec NEG;

factor: '(' expression ')' {$$ = $2;};

factor: ID '(' parameters ')' {
  HANDLE_COMPLETION($1, CF | CR);
  if (!parser->hasSymbol($1.name)) {
    HANDLE_ERROR($1.name, NOT_DECLARED_MSG);
  }
  Function* fun = dynamic_cast<Function*>(parser->symbol($1.name));
  if (!fun) {
    HANDLE_ERROR($1.name, NOT_FUNCTION_MSG);
  }
  $$ = fun->type();
  if (fun->argTypes().size() != $3.size()) {
    HANDLE_ERROR($1.name, WRONG_ARGS_MSG);
  }
  // check the argument types
  for (int i = 0; i < fun->argTypes().size(); ++i) {
      if (fun->argTypes()[i]->id() == 0 && $3[i]->id() != 0) continue; // NullType: accept all types
      if (fun->argTypes()[i]->assignable($3[i])) continue;
      HANDLE_ERROR($1.name, INCOMPATIBLE_ARGS_MSG);
  }
  // qDebug() << "Code:" << opname(Parser::cFun) << fun->name();
  parser->pushBack(Parser::cFun, 0, 1 - $3.size());
  parser->pushBack(fun->index(), 0, 0);
};


factor: ID {
  HANDLE_COMPLETION($1, CF | CR | CC | CV);
  if (!parser->hasSymbol($1.name)) {
    HANDLE_ERROR($1.name, NOT_DECLARED_MSG);
  }
  Symbol* sym = parser->symbol($1.name);
  Variable* var = dynamic_cast<Variable*>(sym);
  if (var) {
     parser->pushBack(Parser::cVar, 0, 1);
     parser->pushBack(var->index(), 0, 0);
  } else { // constant
    Constant* con = dynamic_cast<Constant*>(sym);
    if (con) {
      parser->pushBack(Parser::cImmed, 0, 1);
      parser->pushBackImmed(con->value());
    } else {
      HANDLE_ERROR($1.name, NOT_VAR_CONST_MSG);
    }
  }
  $$ = sym->type();
};

factor: ID refpath {
  HANDLE_COMPLETION($1, CV | CC);
  if (!parser->hasSymbol($1.name)) {
    HANDLE_ERROR($1.name, NOT_DECLARED_MSG);
  }
  Symbol* sym = parser->symbol($1.name);

  const Type* left = sym->type();
  int pathSize = $2.types.size();
  do {
    const Type* right = $2.types.takeFirst();
    const Operation* op = $2.operations.takeFirst();
    try {
      op->check(left, right);
      auto rec = dynamic_cast<const RecordType*>(left);
      auto sel = dynamic_cast<const Selector*>(right);
      if (rec && sel) parser->setImmed($2.immeds.takeFirst(), rec->index(sel->name()));
      left = op->type(left, right);
    } catch (OpError e) {
      HANDLE_ERROR(op->name(), e.msg());
    }
  } while (!$2.types.isEmpty());

  $$ = left;

  Variable* var = dynamic_cast<Variable*>(sym);
  if (var) {
     parser->pushBack(Parser::cVarPath, 0, 1 - pathSize);
     parser->pushBack(var->index(), 0, 0);
     parser->pushBack(pathSize, 0, 0);
  } else { // constant
    Constant* con = dynamic_cast<Constant*>(sym);
    if (con) {
      parser->pushBack(Parser::cImmedPath, Parser::LRType(sym->type(), Parser::Integer()), 1 - pathSize);
      parser->pushBackImmed(con->value());
      parser->pushBack(pathSize, 0, 0);
    } else {
      HANDLE_ERROR($1.name, NOT_VAR_CONST_MSG);
    }
  }
};


refpath: refpathitem {
  $$.types.clear();
  $$.operations.clear();
  $$.immeds.clear();
  $$.types << $1.type;
  $$.operations << $1.operation;
  if ($1.immed >= 0) $$.immeds << $1.immed;
};

refpath: refpath refpathitem {
  $$.types << $2.type;
  $$.operations << $2.operation;
  if ($2.immed >= 0) $$.immeds << $2.immed;
};

refpathitem: '[' expression ']' {
  $$.immed = -1;
  $$.type = $2;
  $$.operation = Parser::Op(TOKEN_TAKE);
};

refpathitem: member_op ID {
  parser->pushBack(Parser::cImmed, 0, 1);
  parser->pushBackImmed(0);
  $$.immed = parser->getImmed();
  $$.type = new Selector($2.name);
  parser->binit($$.type);
  $$.operation = $1;
};

member_op: '.' {$$ = Parser::Op($1);};

factor: INT {
  parser->pushBack(Parser::cImmed, 0, 1);
  parser->pushBackImmed($1);
  $$ = Parser::Integer();
};

factor: FLOAT {
  // qDebug() << "Code: SCA" << $1;
  parser->pushBack(Parser::cImmed, 0, 1);
  parser->pushBackImmed($1);
  $$ = Parser::Real();
};


factor: BEGINSTRING text ENDSTRING {
  // qDebug() << "Code: TXT" << $2;
  parser->pushBack(Parser::cImmed, 0, 1);
  parser->pushBackImmed($2);
  $$ = Parser::Text();
};

factor: ARRAY '(' arrayitems ')' {
  parser->pushBack(Parser::cList, 0, 1 - $3.size);
  parser->pushBack($3.size, 0, 0);
  $$ = new ArrayType($3.type->clone());
  parser->binit($$); // to be deleted later
};

arrayitems: %empty {
  $$.size = 0;
  $$.type = new NullType;
  parser->binit($$.type);
};

arrayitems: arrayseq {$$ = $1;};

arrayseq: expression {
  $$.size = 1;
  $$.type = $1;
};

arrayseq: arrayseq ',' expression {
  if (!$$.type->assignable($3) && !$3->assignable($$.type)) {
    HANDLE_ERROR("array", INCOMPATIBLE_TYPES_MSG);
  }
  if ($3->assignable($$.type)) {
    $$.type = $3;
  }
  $$.size += 1;
};

factor: RECORD '(' recorditems ')' {
  parser->pushBack(Parser::cList, 0, 1 - $3.size());
  parser->pushBack($3.size(), 0, 0);
  $$ = new RecordType($3);
  parser->binit($$); // to be deleted later
};

recorditems: expression {$$.clear(); $$ << $1;};
recorditems: recorditems ',' expression {$$ << $3;};


text: %empty {$$ = QString("");};
text: chars {$$ = $1;};

chars: CHAR {$$ = QString($1);};
chars: chars CHAR {$$.append($2);};


parameters: %empty {$$.clear();};
parameters: arglist {$$ = $1;};

arglist: expression {$$.clear(); $$.append($1);};
arglist: arglist ',' expression {$$.append($3);};

%%


