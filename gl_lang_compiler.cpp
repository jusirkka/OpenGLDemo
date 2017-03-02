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
#include "constant.h"

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

#include "project.h"


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
    "%1 expects a text expression"
};

using Math3D::Real;
using Math3D::Vector4;
using Math3D::Matrix4;

using namespace Demo::GL;


Compiler::Compiler()
    : QObject(),
    mStackSize(0),
    mStackPos(0),
    mCodeSize(0),
    mImmedSize(0),
    mScanner(0),
    mError(),
    mRunner(new Runner()),
    mReady(false) {
}


// public GL interface



void Compiler::compile(const QString& script) {
    reset();
    gl_lang_lex_init(&mScanner);
    // ensure that the source ends with a newline
    QString src = script;
    YY_BUFFER_STATE buf = gl_lang__scan_string(src.append('\n').toUtf8().data(), mScanner);
    int err = gl_lang_parse(this, mScanner);
    gl_lang__delete_buffer(buf, mScanner);

    if (err) throw CompileError(mError);


    mRunner->setup(mAssignments, mVariables, mFunctions, mStackSize);

    mReady = true;

    foreach(Variable* v, mVariables) {
        if (v->shared() && v->used()) {
            if (!mSharedCounts.contains(v->name())) {
                mSharedCounts[v->name()] = 0;
            }
            mSharedCounts[v->name()] = mSharedCounts[v->name()] + 1;
            qDebug() << "created: count of" << v->name() << "is" << mSharedCounts[v->name()];
        }
    }

}

void Compiler::run() {
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

Compiler::~Compiler() {
    qDeleteAll(mSymbols);
    gl_lang_lex_destroy(mScanner);
}



void Compiler::reset() {


    mReady = false;

    mStackSize = 0;
    mStackPos = 0;
    mCodeSize = 0;
    mImmedSize = 0;

    mCurrent.clear();
    mCurrImmed.clear();

    mAssignments.clear();

    VariableList shared;

    foreach(Variable* v, mVariables) {
        mSymbols.remove(v->name());
        if (!v->shared()) {
            delete v;
        } else {
            if (!mSharedCounts.contains(v->name())) {
                // qDebug() << "check: deleting unused" << v->name();
                delete v;
            } else if (mSharedCounts[v->name()] < 1) {
                // qDebug() << "check: count of" << v->name() << "is" << mSharedCounts[v->name()] << ":deleting";
                mSharedCounts.remove(v->name());
                delete v;
            } else {
                // qDebug() << "check: count of" << v->name() << "is" << mSharedCounts[v->name()];
                shared.append(v);
            }
        }
    }

    mVariables.clear();

    foreach(Variable* v, shared) {
        addSymbol(v, false);
    }
    addSymbol(new Var::Local::Natural("gl_result"));

}


const Demo::SymbolMap& Compiler::symbols() const {
    return mSymbols;
}

void Compiler::addSymbol(Symbol* sym, bool used) {
    mSymbols[sym->name()] = sym;
    Variable* var = dynamic_cast<Variable*>(sym);
    if (var) {
        var->setUsed(used);
        var->setIndex(mVariables.size() + FirstVariable);
        mVariables.append(var);
    }
    Function* fun = dynamic_cast<Function*>(sym);
    if (fun) {
        fun->setIndex(mFunctions.size() + FirstFunction);
        mFunctions.append(fun);
    }
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

Compiler::Dispatcher::Dispatcher(Project* p):
    Function("dispatch", Symbol::Integer),
    mParent(p) {
    int argt = Symbol::Text;
    mArgTypes.append(argt);
}

const QVariant& Compiler::Dispatcher::execute(const QVector<QVariant>& vals, int start) {
    QString other = vals[start].value<QString>();
    mParent->dispatch(other);
    mValue.setValue(0);
    return mValue;
}


void gl_lang_error(LocationType*, Compiler* c, yyscan_t, const char*msg) {
    c->createError(msg, Compiler::numerrors);
}
