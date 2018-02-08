#ifndef GL_LANG_PARSER_INTERFACE_H
#define GL_LANG_PARSER_INTERFACE_H


#include "symbol.h"
#include "variable.h"
#include "function.h"

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

class ValueType {
public:
    bool v_bool;
    Math3D::Integer v_int;
    Math3D::Real v_real;
    QChar v_char;
    QString v_string;
    IdentifierType v_identifier;
    QStringList v_string_list;
    Symbol::TypeVector v_int_list;
};




class Parser {


public:

    enum Error {
        declared, notdeclared, notvariable, assincompatible, incompatibletypes,
        expectedinteger, expectedintegerorreal, notfunction, wrongargs, incompatibleargs,
        notvarorconst, expectedmatrix, expectedvectorormatrix, wrongcomps, duplicate,
        notavariable, notasharedvariable, notarealvariable, notavectorvariable,
        notamatrixvariable, notatextvariable, notaintegervariable, expectedarithmetictype,
        nottext, assimported, notimported, scriptnotfound, expectedtextadd, roguestatement,
        numerrors
    };

    static const unsigned CompleteFunctions = 0x01;
    static const unsigned CompleteVariables = 0x02;
    static const unsigned CompleteConstants = 0x04;
    static const unsigned CompleteReserved  = 0x08;
    static const unsigned CompleteAll =       0x0f;
    static const unsigned CompleteFVC =       0x07;
    static const unsigned CompleteFVR =       0x0b;

    virtual void setCode(const QString& name) = 0;
    virtual void pushBack(unsigned op, unsigned lrtype, int inc) = 0;
    virtual void setJump() = 0;
    virtual void initJump() = 0;
    virtual void pushBackImmed(int constVal) = 0;
    virtual void pushBackImmed(Math3D::Real constVal) = 0;
    virtual void pushBackImmed(const QVariant& constVal) = 0;
    virtual void createError(const QString& item, Error err) = 0;
    virtual bool createCompletion(const IdentifierType& id, unsigned completionMask) = 0;
    virtual void addVariable(Variable* v) = 0;
    virtual bool hasSymbol(const QString& sym) const = 0;
    virtual Symbol* symbol(const QString& sym) const = 0;
    virtual bool isImported(const Variable* var) const = 0;
    virtual bool isExported(const QString& v, const QString& script) const = 0;
    virtual void addImported(const QString& v, const QString& script) = 0;
    virtual bool isScript(const QString& name) const = 0;
    virtual void addSubscript(const QString& name) = 0;
    virtual void beginWhile() = 0;
    virtual void beginIf() = 0;
    virtual bool endWhile() = 0;
    virtual bool endIf() = 0;
    virtual bool addElse() = 0;


    // codes
    enum Codes {
        cImmed, cAdd, cSub, cMul, cDiv, cEqual, cNEqual, cLess, cLessOrEq,
        cGreater, cGreaterOrEq, cAnd, cOr, cNot, cFun, cVar, cTake, cNeg,
        cGuard, cBOr, cBAnd
    };

    // LR types
    enum LRTypes {
        cII, cIS, cIV, cIM, cIT,
        cSI, cSS, cSV, cSM, cST,
        cVI, cVS, cVV, cVM, cVT,
        cMI, cMS, cMV, cMM, cMT,
        cTI, cTS, cTV, cTM, cTT
    };
};


}} // namespace Demo::GL


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
