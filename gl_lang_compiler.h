// -----------------------------------------------------------------------
//   Copyright (C) 2009 by Jukka Sirkka
//   jukka.sirkka@iki.fi
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the
//   Free Software Foundation, Inc.,
//   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------


#ifndef GL_LANG_COMPILER_H
#define GL_LANG_COMPILER_H

#include "symbol.h"
#include "variable.h"
#include "function.h"

#include "math3d.h"

#include <QString>
#include <QVector>
#include <QMap>
#include <QVariant>
#include <QtDebug>

#define GL_LANG_LTYPE Demo::GL::LocationType
#define GL_LANG_STYPE Demo::GL::ValueType

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

namespace Demo {

class Project;

namespace GL {

class Widget;
class Runner;

class LocationType {
public:
    int row;
    int col;
    int pos;
    int prev_col;
    int prev_pos;
};

class ValueType {
public:
    int v_int;
    Math3D::Real v_real;
    QChar v_char;
    QString v_string;
    QStringList v_string_list;
    Symbol::TypeList v_int_list;
};

class CompileError {

public:
    CompileError(const QString& msg, int row, int col, int pos)
        :emsg(msg),
          erow(row),
          ecol(col),
          epos(pos)
    {}

    CompileError()
        :emsg(""),
          erow(0),
          ecol(0),
          epos(0)
    {}

    const QString msg() const {return emsg;}
    int row() const {return erow;}
    int col() const {return ecol;}
    int pos() const {return epos;}

private:

    QString emsg;
    int erow;
    int ecol;
    int epos;

};

class Compiler: public QObject {

    Q_OBJECT

public:

    typedef QVector<unsigned int> CodeStack;
    typedef QVector<QVariant> ValueStack;

    class Assignment {

    public:

        Assignment(const QString& v, const CodeStack& c, const ValueStack& i, int p):
            var(v),
            code(c),
            immed(i),
            pos(p) {}

        Assignment(const Assignment& a) {
            var = a.var;
            code = CodeStack(a.code);
            immed = ValueStack(a.immed);
            pos = a.pos;
        }

        QString var;
        CodeStack code;
        ValueStack immed;
        int pos;
    };

    typedef QList<Variable*> VariableList;
    typedef QList<Function*> FunctionList;
    typedef QList<Assignment> AssignmentList;

public:

    enum Error {
        declared,
        notdeclared,
        notvariable,
        assincompatible,
        incompatibletypes,
        expectedinteger,
        expectedintegerorreal,
        notfunction,
        wrongargs,
        incompatibleargs,
        notvarorconst,
        expectedmatrix,
        expectedvectorormatrix,
        wrongcomps,
        duplicate,
        notavariable,
        notasharedvariable,
        notarealvariable,
        notavectorvariable,
        notamatrixvariable,
        notatextvariable,
        notaintegervariable,
        expectedarithmetictype,
        nottext,
        numerrors
    };



    Compiler();

    // GL interface
    void compile(const QString& script);
    bool ready() const {return mReady;}
    void run();

    // grammar interface
    const SymbolMap& symbols() const;
    void addSymbol(Symbol* sym, bool used = true);
    void setCode(const QString& name);
    void pushBack(unsigned op, unsigned lrtype, int inc);
    void setJump();
    void initJump();
    void pushBackImmed(int constVal);
    void pushBackImmed(Math3D::Real constVal);
    void pushBackImmed(const QVariant& constVal);
    void createError(const QString& item, Error err);

    ~Compiler();


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

    // code offsets
    static const int FirstVariable = 1000;
    static const int FirstFunction = 2000;


private:

    typedef QMap<QString, int> CountMap;

    static const char* explanations[];

private:


    Compiler(const Compiler&); // Not implemented
    Compiler &operator=(const Compiler&); // Not implemented

    void reset();

    class Dispatcher: public Function {

    public:

        Dispatcher(Project* p);

        const QVariant& execute(const QVector<QVariant>& vals, int start);

        ~Dispatcher() {}

    private:

        Project* mParent;

    };



private:

    SymbolMap mSymbols;
    AssignmentList mAssignments;
    VariableList mVariables;
    FunctionList mFunctions;
    CodeStack mCurrent;
    ValueStack mCurrImmed;
    CountMap mSharedCounts;
    int mStackSize;
    int mStackPos;
    int mCodeSize;
    int mImmedSize;
    yyscan_t mScanner;
    CompileError mError;
    Runner* mRunner;
    bool mReady;
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

void gl_lang_error(Demo::GL::LocationType*, Demo::GL::Compiler*, yyscan_t, const char*);



#endif // DEMO_PARSER_H
