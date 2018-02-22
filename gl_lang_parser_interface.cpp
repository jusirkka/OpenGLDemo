#include "gl_lang_parser_interface.h"
#include "gl_lang_parser.h"

using namespace Demo;

void gl_lang_error(GL::LocationType*, GL::Parser* parser, yyscan_t, const char* msg) {
    parser->createError(msg, QString("%1"));
}

int GL::Parser::LRType(const Type* left, const Type* right) {
    static struct {int l; int r; int lr;} ops[] = {
        {Type::Integer, Type::Integer, Parser::cII},
        {Type::Integer, Type::Real, Parser::cIS},
        {Type::Integer, Type::Vector, Parser::cIV},
        {Type::Integer, Type::Matrix, Parser::cIM},
        {Type::Integer, Type::Text, Parser::cIT},
        {Type::Real, Type::Integer, Parser::cSI},
        {Type::Real, Type::Real, Parser::cSS},
        {Type::Real, Type::Vector, Parser::cSV},
        {Type::Real, Type::Matrix, Parser::cSM},
        {Type::Real, Type::Text, Parser::cST},
        {Type::Vector, Type::Integer, Parser::cVI},
        {Type::Vector, Type::Real, Parser::cVS},
        {Type::Vector, Type::Vector, Parser::cVV},
        {Type::Vector, Type::Matrix, Parser::cVM},
        {Type::Vector, Type::Text, Parser::cVT},
        {Type::Matrix, Type::Integer, Parser::cMI},
        {Type::Matrix, Type::Real, Parser::cMS},
        {Type::Matrix, Type::Vector, Parser::cMV},
        {Type::Matrix, Type::Matrix, Parser::cMM},
        {Type::Matrix, Type::Text, Parser::cMT},
        {Type::Text, Type::Integer, Parser::cTI},
        {Type::Text, Type::Real, Parser::cTS},
        {Type::Text, Type::Vector, Parser::cTV},
        {Type::Text, Type::Matrix, Parser::cTM},
        {Type::Text, Type::Text, Parser::cTT},
        {-1, -1, 0}
    };
    int l = left->id();
    int r = right->id();
    for (int i = 0; ops[i].l != -1; ++i) {
        if (ops[i].l == l && ops[i].r == r) return ops[i].lr;
    }
    Q_ASSERT(0);
    return 0;
}

const Operation* GL::Parser::Op(int token) {
    static QMap<int, const Operation*> ops;
    if (ops.isEmpty()) {
        // qCDebug(OGL) << "init";
        ops['<'] = new RelOp("<", Parser::cLess);
        ops['>'] = new RelOp(">", Parser::cGreater);
        ops[LE] = new RelOp("<=", Parser::cLessOrEq);
        ops[GE] = new RelOp(">=", Parser::cGreaterOrEq);
        ops[EQ] = new EqOp("==", Parser::cEqual);
        ops[NE] = new EqOp("!=", Parser::cNEqual);
        ops[BOR] = new AndOrOp("|", Parser::cBOr);
        ops[BAND] = new AndOrOp("&", Parser::cBAnd);
        ops[OR] = new AndOrOp("||", Parser::cOr);
        ops[AND] = new AndOrOp("&&", Parser::cAnd);
        ops['+'] = new AddOp("+", Parser::cAdd);
        ops['-'] = new AddOp("-", Parser::cSub);
        ops['*'] = new MulOp("*", Parser::cMul);
        ops['/'] = new DivOp("/", Parser::cDiv);
        ops['!'] = new NegOp("!", Parser::cNot);
        ops[TOKEN_MINUS] = new SignOp("-", Parser::cNeg);
        ops[TOKEN_PLUS] = new SignOp("+", -1); // not used in byte code
        ops[TOKEN_TAKE] = new TakeOp("TAKE", -1);
        ops['.'] = new MemberOp(".", -1);
    }
    return ops[token];
}

const Type* GL::Parser::Integer() {
    static Type* t = new Integer_T;
    return t;
}

const Type* GL::Parser::Real() {
    static Type* t = new Real_T;
    return t;
}

const Type* GL::Parser::Text() {
    static Type* t = new Text_T;
    return t;
}

const Type* GL::Parser::Vector() {
    static Type* t = new Vector_T;
    return t;
}

const Type* GL::Parser::Matrix() {
    static Type* t = new Matrix_T;
    return t;
}

