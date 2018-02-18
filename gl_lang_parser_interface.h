#ifndef GL_LANG_PARSER_INTERFACE_H
#define GL_LANG_PARSER_INTERFACE_H


#include "symbol.h"
#include "variable.h"
#include "function.h"
#include "operation.h"

#include "math3d.h"

#include <QString>
#include <QVariant>

#define GL_LANG_LTYPE Demo::GL::LocationType
#define GL_LANG_STYPE Demo::GL::ValueType

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
using yyscan_t = void *;
#endif

namespace Demo {

class Scope;

namespace GL {

class Widget;

class LocationType {
public:
    LocationType()
        : row(0)
        , col(0)
        , pos(0)
        , prev_col(0)
        , prev_pos(0) {}

    int row;
    int col;
    int pos;
    int prev_col;
    int prev_pos;
};

class IdentifierType {
public:
    QString name;
    int pos; // for autocompletion
};


class NamedTypeList {
public:
    QStringList names;
    QVector<Type*> types;
};

class ArrayItemType {
public:
    int size;
    const Type* type;
};

class RefPathType {
public:
    QVector<const Type*> types;
    QVector<const Operation*> operations;
    QVector<int> immeds;
};

class RefPathItemType {
public:
    const Type* type;
    const Operation* operation;
    int immed;
};

class ValueType {
public:
    bool v_bool;
    Math3D::Integer v_int;
    Math3D::Real v_real;
    QChar v_char;
    QString v_string;
    IdentifierType v_identifier;
    QStringList v_string_list;
    const Type* v_type;
    Type* v_new_type;
    const Operation* v_oper;
    Type::List v_type_list;
    NamedTypeList v_named_type_list;
    ArrayItemType v_array_item;
    RefPathItemType v_ref_path_item;
    RefPathType v_ref_path;
};




class Parser {


public:

    static const unsigned CF = 0x01;
    static const unsigned CV = 0x02;
    static const unsigned CC = 0x04;
    static const unsigned CR = 0x08;
    static const unsigned CT = 0x10;

    virtual void assignment() = 0;
    virtual void pushBack(unsigned op, unsigned lrtype, int inc) = 0;
    virtual void setJump() = 0;
    virtual void initJump() = 0;
    virtual void finalizeJumps() = 0;
    virtual void pushBackImmed(int constVal) = 0;
    virtual void pushBackImmed(Math3D::Real constVal) = 0;
    virtual void pushBackImmed(const QVariant& constVal) = 0;
    virtual void setImmed(int index, int val) = 0;
    virtual int getImmed() const = 0;
    virtual void createError(const QString& item, QString detail) = 0;
    virtual bool createCompletion(const IdentifierType& id, unsigned completionMask) = 0;
    virtual void addSymbol(Symbol* s) = 0;
    virtual bool hasSymbol(const QString& sym) const = 0;
    virtual Symbol* symbol(const QString& sym) const = 0;
    virtual bool isImported(const Variable* var) const = 0;
    virtual bool isExported(const QString& v, const QString& script) const = 0;
    virtual void addImported(const QString& v, const QString& script) = 0;
    virtual bool isScript(const QString& name) const = 0;
    virtual void addSubscript(const QString& name) = 0;
    virtual void binit(const Type* t) = 0;
    virtual void beginWhile() = 0;
    virtual void beginIf() = 0;
    virtual bool endWhile() = 0;
    virtual bool endIf() = 0;
    virtual bool addElse() = 0;
    virtual bool addElsif() = 0;


    // codes
    enum Codes {
        cImmed, cAdd, cSub, cMul, cDiv, cEqual, cNEqual, cLess, cLessOrEq,
        cGreater, cGreaterOrEq, cAnd, cOr, cNot, cFun, cVar, cNeg,
        cGuard, cJump, cBOr, cBAnd, cList, cVarPath, cImmedPath, cAss, cAssPath
    };

    // LR types
    enum LRTypes {
        cII, cIS, cIV, cIM, cIT,
        cSI, cSS, cSV, cSM, cST,
        cVI, cVS, cVV, cVM, cVT,
        cMI, cMS, cMV, cMM, cMT,
        cTI, cTS, cTV, cTM, cTT
    };


    static int LRType(const Type* left, const Type* right);
    static const Operation* Op(int token);
    static const Type* Integer();
    static const Type* Real();
    static const Type* Text();
    static const Type* Vector();
    static const Type* Matrix();
};


}} // namespace Demo::GL

// extra tokens
#define TOKEN_MINUS -1
#define TOKEN_PLUS -2
#define TOKEN_TAKE -3

#define YYLLOC_DEFAULT(Current, Rhs, N) do if (N) {\
    (Current).row = YYRHSLOC (Rhs, 1).row;\
    (Current).col = YYRHSLOC (Rhs, 1).col;\
    (Current).prev_col = YYRHSLOC (Rhs, 1).prev_col;\
    (Current).pos = YYRHSLOC (Rhs, 1).pos;\
    (Current).prev_pos = YYRHSLOC (Rhs, 1).prev_pos;\
    } else {\
    (Current).row = YYRHSLOC (Rhs, 0).row;\
    (Current).col = YYRHSLOC (Rhs, 0).col;\
    (Current).prev_col = YYRHSLOC (Rhs, 0).prev_col;\
    (Current).pos = YYRHSLOC (Rhs, 0).pos;\
    (Current).prev_pos = YYRHSLOC (Rhs, 0).prev_pos;\
    } while (0)

void gl_lang_error(Demo::GL::LocationType*, Demo::GL::Parser*, yyscan_t, const char*);


#endif // GL_LANG_PARSER_H
