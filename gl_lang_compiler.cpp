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

#include <QtGlobal>

#include "gl_lang_compiler.h"
#include "gl_lang_runner.h"

#include "gl_lang_parser.h"
#ifndef YYSTYPE
#define YYSTYPE GL_LANG_STYPE
#endif
#ifndef YYLTYPE
#define YYLTYPE GL_LANG_LTYPE
#endif
#include "gl_lang_scanner.h"

#include "scope.h"
#include "constant.h"


const char* Demo::GL::Compiler::explanations[] = {
    "%1 has been already declared.",
    "%1 has not been declared.",
    "%1 is not a variable.",
    "incompatible types in assignment to %1.",
    "incompatible types in %1 expression.",
    "expected integer in %1 expression.",
    "expected integer or real in %1 expression.",
    "%1 is not a function.",
    "wrong number of arguments in %1.",
    "incompatible arguments in %1.",
    "%1 is not a variable or constant.",
    "%1 is not a matrix.",
    "%1 is not a vector or matrix.",
    "wrong number of components in %1 expression.",
    "duplicate declaration of %1.",
    "symbol %1 is not a variable",
    "symbol %1 is not a shared variable",
    "type of variable %1 is not Real",
    "type of variable %1 is not Vector",
    "type of variable %1 is not Matrix",
    "type of variable %1 is not Text",
    "type of variable %1 is not Natural",
    "expected arithmetic type in %1 expression",
    "%1 expects a text expression",
    "cannot assign to imported variable %1",
    "variable %1 has not been exported",
    R"(script "%1" not found)",
    "%1 is not a supported operation for Text",
    "unmatching %1"
};

using Math3D::Real;
using Math3D::Vector4;
using Math3D::Matrix4;

using namespace Demo::GL;


Compiler::Compiler(const QString &name, Scope* globalScope, QObject *parent):
    QObject(parent),
    mStackSize(0),
    mStackPos(0),
    mCodeSize(0),
    mImmedSize(0),
    mScanner(nullptr),
    mError(),
    mRunner(new Runner(this)),
    mReady(false),
    mRecompile(false),
    mGlobalScope(globalScope) {

    setObjectName(name);
}


// public GL interface



void Compiler::compile(const QString& script) {
    reset();
    gl_lang_lex_init(&mScanner);
    mSource = script;
    // ensure that the source ends with newlines
    YY_BUFFER_STATE buf = gl_lang__scan_string(mSource.append("\n\n").toUtf8().data(), mScanner);
    int err = gl_lang_parse(this, mScanner);

    if (!err) err = checkControls();

    gl_lang__delete_buffer(buf, mScanner);

    if (err) throw mError;

    mRunner->setup(mStatements, mVariables, mGlobalScope->functions());
    mReady = true;
}

int Compiler::checkControls() {
    if (mWhiles.isEmpty() && mConds.isEmpty()) return 0;

    int pos = 0;

    if (!mWhiles.isEmpty()) {
        Statement::Statement* s = mStatements[mWhiles.top()];
        if (s->pos() > pos) pos = s->pos();
    }

    if (!mConds.isEmpty()) {
        Statement::Statement* s = mStatements[mConds.top()];
        if (s->pos() > pos) pos = s->pos();
    }

    mError = CompileError("Unclosed control statement", pos);
    return 1;
}

bool Compiler::ready() const {
    return mReady || mRecompile;
}


void Compiler::run() {
    if (mRecompile) {
        try {
            compile(mSource);
        } catch (CompileError& e){
            throw RunError(QString("Compilation failed: %1").arg(e.msg()), e.pos());
        }
    }
    if (!mReady) {
        throw RunError("Not compiled ", 0);
    }
    // qDebug() << "running" << objectName();
    mRunner->run();
}

void Compiler::createError(const QString &item, Error err) {
    QString detail = item;
    if (err < numerrors && err >= 0) {
        detail = QString(explanations[err]).arg(item);
    }
    LocationType* loc = gl_lang_get_lloc(mScanner);
    mError = CompileError(detail, loc->pos);
}

bool Compiler::createCompletion(const IdentifierType&, unsigned) {
    return false; // noop
}

Compiler::~Compiler() {
    qDeleteAll(mSymbols);
}

void Compiler::compileLater() {
    mRecompile = true;
}

void Compiler::reset() {

    emit resetting();

    mReady = false;
    mRecompile = false;

    mStackSize = 0;
    mStackPos = 0;
    mCodeSize = 0;
    mImmedSize = 0;

    mWhiles.clear();
    mConds.clear();

    mCurrent.clear();
    mCurrImmed.clear();

    mStatements.clear();

    qDeleteAll(mSymbols);
    mSymbols.clear();

    qDeleteAll(mStatements);
    mStatements.clear();

    mVariables.clear();
    mExports.clear();

    mSubscripts.clear();

    for (auto& name: mImportScripts) {
        Compiler* c = mGlobalScope->compiler(name);
        if (c) {
            disconnect(c, SIGNAL(resetting()), this, SLOT(compileLater()));
        }
    }

    mImportScripts.clear();

    addVariable(new Var::Local::Natural("gl_result"));
}


void Compiler::addVariable(Variable* v) {
    // qDebug() << "adding" << objectName() << v->name();
    v->setIndex(mVariables.size() + Scope::VariableOffset);
    mVariables[v->name()] = v;
    mSymbols[v->name()] = v;
    if (v->shared()) {
        // qDebug() << "exporting" << objectName() << v->name();
        mExports[v->name()] = v;
    }
}

bool Compiler::hasSymbol(const QString& sym) const {
    if (mGlobalScope->symbols().contains(sym)) return true;
    return mSymbols.contains(sym);
}

Demo::Symbol* Compiler::symbol(const QString& sym) const {
    if (mGlobalScope->symbols().contains(sym)) return mGlobalScope->symbols()[sym];
    if (mSymbols.contains(sym)) return mSymbols[sym];
    return nullptr;
}

bool Compiler::isImported(const Variable* var) const {
    if (!mSymbols.contains(var->name())) return false;
    // local, imported or exported variable
    if (mExports.contains(var->name())) return false;
    // local (not shared) or imported (shared)
    return var->shared();
}


const QStringList& Compiler::subscripts() const {
    return mSubscripts;
}


bool Compiler::isExported(const QString& name, const QString& script) const {
    if (script == "") { // global scope
        return mGlobalScope->exports().contains(name);
    }
    Compiler* c = mGlobalScope->compiler(script);
    if (!c) return false;
    return c->exports().contains(name);
}

void Compiler::addImported(const QString& name, const QString& script) {
    Variable* v;
    if (script == "") { // global scope
        v = mGlobalScope->exports().value(name)->clone();
    } else {
        v = mGlobalScope->compiler(script)->exports().value(name)->clone();
        if (!mImportScripts.contains(script)) {
            mImportScripts.append(script);
            Compiler* c = mGlobalScope->compiler(script);
            connect(c, SIGNAL(resetting()), this, SLOT(compileLater()));
        }
    }

    v->setIndex(mVariables.size() + Scope::VariableOffset);
    mVariables[v->name()] = v;
    mSymbols[v->name()] = v;

}

const Demo::VariableMap& Compiler::exports() const {
    return mExports;
}

bool Compiler::isScript(const QString& name) const {
    return mGlobalScope->compiler(name) != nullptr;
}

void Compiler::addSubscript(const QString& name) {
    mSubscripts.append(name);
}

void Compiler::setCode(const QString& name) {
    LocationType* loc = gl_lang_get_lloc(mScanner);
    int index = mVariables[name]->index();
    mStatements.append(new Statement::Assignment(index, mCurrent, mCurrImmed, mStackSize, loc->pos));
    mCurrent.clear();
    mCurrImmed.clear();
    mCodeSize = 0;
    mImmedSize = 0;
    mStackPos = 0;
    // qDebug() << name << ": assignment: Byte code ready. Stack size =" << mStackSize;
    mStackSize = 0;
}


void Compiler::beginWhile() {
    mWhiles.push(mStatements.size()); // index of the statement
    LocationType* loc = gl_lang_get_lloc(mScanner);
    mStatements.append(new Statement::CondJump(mCurrent, mCurrImmed, mStackSize, loc->pos));

    mCurrent.clear();
    mCurrImmed.clear();
    mCodeSize = 0;
    mImmedSize = 0;
    mStackPos = 0;
    mStackSize = 0;
}

bool Compiler::endWhile() {
    if (mWhiles.isEmpty()) return false;

    int condIndex = mWhiles.pop();
    int myIndex = mStatements.size();

    auto cond = dynamic_cast<Statement::BaseJump*>(mStatements[condIndex]);
    cond->setJump(myIndex - condIndex + 1);

    LocationType* loc = gl_lang_get_lloc(mScanner);
    mStatements.append(new Statement::Jump(loc->pos, condIndex - myIndex));

    return true;
}

void Compiler::beginIf() {
    mConds.push(mStatements.size()); // index of the statement
    LocationType* loc = gl_lang_get_lloc(mScanner);
    mStatements.append(new Statement::CondJump(mCurrent, mCurrImmed, mStackSize, loc->pos));

    mCurrent.clear();
    mCurrImmed.clear();
    mCodeSize = 0;
    mImmedSize = 0;
    mStackPos = 0;
    // qDebug() << name << ": condjump: Byte code ready. Stack size =" << mStackSize;
    mStackSize = 0;
}


bool Compiler::endIf() {
    if (mConds.isEmpty()) return false;
    int jumpIndex = mConds.pop();
    int myIndex = mStatements.size();
    auto jump = dynamic_cast<Statement::BaseJump*>(mStatements[jumpIndex]);
    jump->setJump(myIndex - jumpIndex);
    return true;
}

bool Compiler::addElse() {
    if (mConds.isEmpty()) return false;

    int condIndex = mConds.pop();
    int myIndex = mStatements.size();
    mConds.push(myIndex);

    auto cond = dynamic_cast<Statement::BaseJump*>(mStatements[condIndex]);
    cond->setJump(myIndex - condIndex + 1);

    LocationType* loc = gl_lang_get_lloc(mScanner);
    // add 4 to go to the end of line
    mStatements.append(new Statement::Jump(loc->pos + 4));
    return true;
}


void Compiler::pushBack(unsigned op, unsigned lrtype, int inc) {
    mCurrent.append((op & 0xfff) | ((lrtype & 0xff) << 12));
    mStackPos += inc;
    if (mStackSize < mStackPos) mStackSize = mStackPos;
    // qDebug() << "push back: stack pos = " << mStackPos << "code size = " << mCurrent.size();
}

void Compiler::setJump() {
    mStackPos = 0;
    if (mCodeSize) {
        mCurrent[mCodeSize - 2] = mCurrent.size() - mCodeSize;
        mCurrent[mCodeSize - 1] = mCurrImmed.size() - mImmedSize;
        /*qDebug() << "set jump: codesize = " << mCodeSize <<
                    "code jump = " << mCurrent.size() - mCodeSize <<
                    "immed jump = " << mCurrImmed.size() - mImmedSize; */
    }
}

void Compiler::initJump() {
    mStackPos = 0;
    mCodeSize = mCurrent.size();
    mImmedSize = mCurrImmed.size();
    // qDebug() << "init jump: code addr = " << mCodeSize << "immed addr = " << mImmedSize;
}


void Compiler::pushBackImmed(int constVal) {
    mCurrImmed.append(QVariant(constVal));
    // qDebug() << "push back immed: immed size = " << mCurrImmed.size();
}

void Compiler::pushBackImmed(Math3D::Real constVal) {
    mCurrImmed.append(QVariant(constVal));
    // qDebug() << "push back immed: immed size = " << mCurrImmed.size();
}

void Compiler::pushBackImmed(const QVariant& constVal) {
    mCurrImmed.append(constVal);
    // qDebug() << "push back immed: immed size = " << mCurrImmed.size();
}
