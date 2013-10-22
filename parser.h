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


#ifndef DEMO_PARSER_H
#define DEMO_PARSER_H

#include "symbol.h"
#include "variable.h"
#include "function.h"

#include "math3d.h"

#include <QString>
#include <QVector>
#include <QMap>
#include <QVariant>
#include <QtDebug>

namespace Demo {

class GLWidget;
class Runner;

class Parser: public QObject {

    Q_OBJECT

public:

    typedef QVector<unsigned int> CodeStack;
    typedef QVector<QVariant> ValueStack;

    class Assignment {

    public:

        Assignment(const QString& v, const CodeStack& c, const ValueStack& i):
            var(v),
            code(c),
            immed(i) {}

        Assignment(const Assignment& a) {
            var = a.var;
            code = CodeStack(a.code);
            immed = ValueStack(a.immed);
        }

        QString var;
        CodeStack code;
        ValueStack immed;
    };

    typedef QMap<QString, Symbol*> SymbolMap;
    typedef QList<Variable*> VariableList;
    typedef QList<Function*> FunctionList;
    typedef QList<Assignment> AssignmentList;

public slots:

    void shared_deleted(const QStringList&);

public:

    // GL interface
    static bool ParseIt(const QString&);
    static Runner* CreateRunner();

    // grammar interface
    static void AddSymbol(Symbol*);
    static void AddSharedVariable(Variable*);
    static const SymbolMap& Symbols();
    static void SetCode(const QString&);
    static void PushBack(unsigned op, unsigned lrtype, int inc);
    static void SetJump();
    static void InitJump();
    static void PushBackImmed(int);
    static void PushBackImmed(Math3D::Real);
    static void PushBackImmed(const QVariant&);

    ~Parser();

public:

    // codes
    enum {
        cImmed, cAdd, cSub, cMul, cDiv, cEqual, cNEqual, cLess, cLessOrEq,
        cGreater, cGreaterOrEq, cAnd, cOr, cNot, cFun, cVar, cTake, cNeg,
        cGuard, cBOr, cBAnd
    };

    // LR types
    enum {
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


private:

    Parser();

    Parser(const Parser&); // Not implemented
    Parser &operator=(const Parser&); // Not implemented

    static Parser& instance();

    // GL interface
    void init();
    Runner* createRunner();

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
};


} // namespace Demo


#endif // DEMO_PARSER_H
