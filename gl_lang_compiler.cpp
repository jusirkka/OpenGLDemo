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
#include "typedef.h"



using Math3D::Real;
using Math3D::Vector4;
using Math3D::Matrix4;

using namespace Demo::GL;


Compiler::Compiler(const QString &name, Scope* globalScope, QObject *parent):
    QObject(parent),
    mStackSize(0),
    mStackPos(0),
    mCodeAddr(0),
    mImmedAddr(0),
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

    // ensure that the source starts and ends with newlines
    mSource = "\n" + script + "\n";
    gl_lang__scan_string(mSource.toUtf8().data(), mScanner);
    int err = gl_lang_parse(this, mScanner);

    if (!err) err = checkControls();

    gl_lang_lex_destroy(mScanner);

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
        Statement::Statement* s = mStatements[mConds.top().top().cond];
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

void Compiler::createError(const QString &item, QString detail) {
    LocationType* loc = gl_lang_get_lloc(mScanner);
    mError = CompileError(detail.arg(item), loc->pos);
}

bool Compiler::createCompletion(const IdentifierType&, unsigned) {
    return false; // noop
}

Compiler::~Compiler() {
    qDeleteAll(mSymbols);
    qDeleteAll(mStatements);
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
    mCodeAddr = 0;
    mImmedAddr = 0;

    mWhiles.clear();
    mConds.clear();

    mCurrent.clear();
    mCurrImmed.clear();


    qDeleteAll(mSymbols);
    mSymbols.clear();

    qDeleteAll(mStatements);
    mStatements.clear();

    qDeleteAll(mTmpTypes);
    mTmpTypes.clear();

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

    addSymbol(new LocalVar("gl_result", new Integer_T));
}


void Compiler::addSymbol(Symbol* s) {
    // qDebug() << "adding" << objectName() << v->name();
    auto v = dynamic_cast<Variable*>(s);
    if (v) {
        v->setIndex(mVariables.size() + Scope::VariableOffset);
        mVariables[v->name()] = v;
        if (v->shared()) {
            // qDebug() << "exporting" << objectName() << v->name();
            mExports[v->name()] = v;
        }
    }
    mSymbols[s->name()] = s;
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

void Compiler::assignment() {
    LocationType* loc = gl_lang_get_lloc(mScanner);
    mStatements.append(new Statement::Assignment(mCurrent, mCurrImmed, mStackSize, loc->pos));
    mCurrent.clear();
    mCurrImmed.clear();
    mCodeAddr = 0;
    mImmedAddr = 0;
    mStackPos = 0;
    // qDebug() << name << ": assignment: Byte code ready. Stack size =" << mStackSize;
    mStackSize = 0;
}

void Compiler::binit(const Type *t) {
    mTmpTypes << t;
}

void Compiler::beginWhile() {
    mWhiles.push(mStatements.size()); // index of the statement
    LocationType* loc = gl_lang_get_lloc(mScanner);
    mStatements.append(new Statement::CondJump(mCurrent, mCurrImmed, mStackSize, loc->pos));

    mCurrent.clear();
    mCurrImmed.clear();
    mCodeAddr = 0;
    mImmedAddr = 0;
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
    PendingJumpStack jumps;
    jumps.push(PendingJump(mStatements.size())); // index of the statement
    mConds.push(jumps);
    LocationType* loc = gl_lang_get_lloc(mScanner);
    mStatements.append(new Statement::CondJump(mCurrent, mCurrImmed, mStackSize, loc->pos));

    mCurrent.clear();
    mCurrImmed.clear();
    mCodeAddr = 0;
    mImmedAddr = 0;
    mStackPos = 0;
    mStackSize = 0;
}

bool Compiler::addElse() {
    if (mConds.isEmpty()) return false;
    if (mConds.top().top().hasElse()) return false;

    mConds.top().top().jump = mStatements.size();

    LocationType* loc = gl_lang_get_lloc(mScanner);
    // add 4 to go to the end of line
    mStatements.append(new Statement::Jump(loc->pos + 4));

    return true;
}

bool Compiler::addElsif() {
    if (mConds.isEmpty()) return false;
    if (mConds.top().top().hasElse()) return false;

    mConds.top().top().jump = mStatements.size();

    LocationType* loc = gl_lang_get_lloc(mScanner);
    mStatements.append(new Statement::Jump(loc->pos + 4));
    mConds.top().push(PendingJump(mStatements.size()));

    mStatements.append(new Statement::CondJump(mCurrent, mCurrImmed, mStackSize, loc->pos));

    mCurrent.clear();
    mCurrImmed.clear();
    mCodeAddr = 0;
    mImmedAddr = 0;
    mStackPos = 0;
    mStackSize = 0;

    return true;
}

bool Compiler::endIf() {
    if (mConds.isEmpty()) return false;
    int myIndex = mStatements.size();
    int nextCond = myIndex;
    PendingJumpStack myStack = mConds.pop();
    while (!myStack.isEmpty()) {
        PendingJump curr = myStack.pop();
        if (curr.hasElse()) {
            auto jump = dynamic_cast<Statement::Jump*>(mStatements[curr.jump]);
            jump->setJump(myIndex - curr.jump);
            nextCond = curr.jump + 1;
        }
        auto cond = dynamic_cast<Statement::CondJump*>(mStatements[curr.cond]);
        cond->setJump(nextCond - curr.cond);
    }
    return true;
}


void Compiler::pushBack(unsigned op, unsigned lrtype, int inc) {
    mCurrent.append((op & 0xfff) | ((lrtype & 0xff) << 12));
    mStackPos += inc;
    if (mStackSize < mStackPos) mStackSize = mStackPos;
}

void Compiler::setJump() {
    mStackPos = 0;
    mCurrent[mCodeAddr - 2] = mCurrent.size() - mCodeAddr;
    mCurrent[mCodeAddr - 1] = mCurrImmed.size() - mImmedAddr;
    mGuardJumps.push(GuardJump(mCurrent.size(), mCurrImmed.size()));
}

void Compiler::initJump() {
    mStackPos = 0;
    mCodeAddr = mCurrent.size();
    mImmedAddr = mCurrImmed.size();
}

void Compiler::finalizeJumps() {
    while (!mGuardJumps.isEmpty()) {
        auto jumps = mGuardJumps.pop();
        mCurrent[jumps.codeAddr - 2] = mCurrent.size() - jumps.codeAddr;
        mCurrent[jumps.codeAddr - 1] = mCurrImmed.size() - jumps.immedAddr;
    }
}

void Compiler::pushBackImmed(int constVal) {
    mCurrImmed.append(QVariant(constVal));
}

void Compiler::pushBackImmed(Math3D::Real constVal) {
    mCurrImmed.append(QVariant(constVal));
}

void Compiler::pushBackImmed(const QVariant& constVal) {
    mCurrImmed.append(constVal);
}

void Compiler::setImmed(int index, int val) {
    mCurrImmed[index] = QVariant::fromValue(val);
}

int Compiler::getImmed() const {
    return mCurrImmed.size() - 1;
}
