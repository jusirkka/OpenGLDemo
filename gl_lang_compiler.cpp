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
    "%1 is not a supported operation for Text"
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
    gl_lang__delete_buffer(buf, mScanner);

    if (err) throw mError;

    mRunner->setup(mAssignments, mVariables, mGlobalScope->functions(), mStackSize);
    mReady = true;
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
    mError = CompileError(detail, loc->row, loc->col, loc->pos);
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

    mCurrent.clear();
    mCurrImmed.clear();

    mAssignments.clear();

    qDeleteAll(mSymbols);
    mSymbols.clear();

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
    mVariables.append(v);
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
    mVariables.append(v);
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
    mAssignments.append(Assignment(name, mCurrent, mCurrImmed, loc->pos));
    mCurrent.clear();
    mCurrImmed.clear();
    mCodeSize = 0;
    mImmedSize = 0;
    // qDebug() << name << ": assignment: Byte code ready. Stack size =" << mStackSize;
}

void Compiler::pushBack(unsigned op, unsigned lrtype, int inc) {
    mCurrent.append((op & 0xfff) | ((lrtype & 0xff) << 12));
    mStackPos += inc;
    if (mStackSize < mStackPos) mStackSize = mStackPos;
}

void Compiler::setJump() {
    mStackPos = 0;
    if (mCodeSize) {
        mCurrent[mCodeSize - 2] = mCurrent.size() - mCodeSize;
        mCurrent[mCodeSize - 1] = mCurrImmed.size() - mImmedSize;
    }
}

void Compiler::initJump() {
    mStackPos = 0;
    mCodeSize = mCurrent.size();
    mImmedSize = mCurrImmed.size();
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
