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
#include "statement.h"

#include <QVector>
#include <QMap>
#include <QStack>
#include <QtDebug>


namespace Demo {

class Scope;

namespace GL {

class Widget;
class Runner;


class CompileError {

public:
    CompileError(QString msg, int pos)
        : emsg(std::move(msg))
        , epos(pos)
    {}

    CompileError():
        emsg(""),
        epos(0)
    {}

    const QString msg() const {return emsg;}
    int pos() const {return epos;}

private:

    QString emsg;
    int epos;

};



class Compiler: public QObject, public Parser {

    Q_OBJECT

public:


    using FunctionVector = QVector<Demo::Function*>;
    using StatementVector = QVector<Demo::Statement::Statement*>;
    using CodeStack = Demo::Statement::Statement::CodeStack;
    using ValueStack = Demo::Statement::Statement::ValueStack;
    using IndexStack = QStack<int>;

public:

    Compiler(const QString& name, Scope* globalScope, QObject* parent = nullptr);

    // GL interface
    void compile(const QString& script);
    bool ready() const;
    void run();

    // grammar interface
    void setCode(const QString& name) override;
    void pushBack(unsigned op, unsigned lrtype, int inc) override;
    void setJump() override;
    void initJump() override;
    void pushBackImmed(int constVal) override;
    void pushBackImmed(Math3D::Real constVal) override;
    void pushBackImmed(const QVariant& constVal) override;
    void createError(const QString& item, Error err) override;
    bool createCompletion(const IdentifierType& id, unsigned completionMask) override;
    void addVariable(Variable* v) override;
    bool hasSymbol(const QString& sym) const override;
    Symbol* symbol(const QString& sym) const override;
    bool isImported(const Variable* var) const override;
    bool isExported(const QString& v, const QString& script) const override;
    void addImported(const QString& v, const QString& script) override;
    bool isScript(const QString& name) const override;
    void addSubscript(const QString& name) override;
    void beginWhile() override;
    void beginIf() override;
    bool endWhile() override;
    bool endIf() override;
    bool addElse() override;

    const QStringList& subscripts() const;
    const VariableMap& exports() const;

    ~Compiler() override;


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
    int checkControls();

private:

    StatementVector mStatements;
    VariableMap mVariables;
    VariableMap mExports;
    SymbolMap mSymbols;
    CodeStack mCurrent;
    ValueStack mCurrImmed;
    IndexStack mWhiles;
    IndexStack mConds;
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
