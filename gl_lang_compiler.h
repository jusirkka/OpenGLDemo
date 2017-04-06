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

#include "gl_lang_parser_interface.h"

#include <QVector>
#include <QMap>
#include <QtDebug>


namespace Demo {

class Scope;

namespace GL {

class Widget;
class Runner;


class CompileError {

public:
    CompileError(const QString& msg, int row, int col, int pos):
        emsg(msg),
        erow(row),
        ecol(col),
        epos(pos)
    {}

    CompileError():
        emsg(""),
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



class Compiler: public QObject, public Parser {

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

    Compiler(const QString& name, Scope* globalScope, QObject* parent = 0);

    // GL interface
    void compile(const QString& script);
    bool ready() const;
    void run();

    // grammar interface
    void setCode(const QString& name);
    void pushBack(unsigned op, unsigned lrtype, int inc);
    void setJump();
    void initJump();
    void pushBackImmed(int constVal);
    void pushBackImmed(Math3D::Real constVal);
    void pushBackImmed(const QVariant& constVal);
    void createError(const QString& item, Error err);
    bool createCompletion(const IdentifierType& id, unsigned completionMask);
    void addVariable(Variable* v);
    bool hasSymbol(const QString& sym) const;
    Symbol* symbol(const QString& sym) const;
    bool isImported(const Variable* var) const;
    bool isExported(const QString& v, const QString& script) const;
    void addImported(const QString& v, const QString& script);
    bool isScript(const QString& name) const;
    void addSubscript(const QString& name);

    const QStringList& subscripts() const;
    const VariableMap& exports() const;

    ~Compiler();


public slots:

    void compileLater();

signals:

    void resetting();

private:

    static const char* explanations[];

private:


    Compiler(const Compiler&); // Not implemented
    Compiler &operator=(const Compiler&); // Not implemented

    void reset();

private:

    AssignmentList mAssignments;
    VariableList mVariables;
    VariableMap mExports;
    SymbolMap mSymbols;
    CodeStack mCurrent;
    ValueStack mCurrImmed;
    int mStackSize;
    int mStackPos;
    int mCodeSize;
    int mImmedSize;
    yyscan_t mScanner;
    CompileError mError;
    Runner* mRunner;
    bool mReady;
    bool mRecompile;
    QString mSource;
    Scope* mGlobalScope;
    QStringList mImportScripts;
    QStringList mSubscripts;
};


}} // namespace Demo::GL


#endif
