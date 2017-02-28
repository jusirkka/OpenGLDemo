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

#include "parser.h"
#include "constant.h"
#include "runner.h"

extern "C"
{

#include "gl_lang_scanner.h"
#include "gl_lang_types.h"

int gl_lang_parse(void);
extern int gl_lang_debug;

extern char parser_error_buffer[256];

}

#include <QtGlobal>


using Math3D::Real;
using Math3D::Vector4;
using Math3D::Matrix4;





// public GL interface

void Demo::Parser::ParseIt(const QString& name, const QString& inp) {
    gl_lang_lex_destroy();
    instance().init(name);
    gl_lang_debug = 0;
    // ensure that the source ends with a newline
    QString src(inp);
    src += "\n";
    gl_lang__scan_string(src.toUtf8().data());
    int err = gl_lang_parse();
    if (err) throw ParseError(QString(parser_error_buffer), gl_lang_lloc.row, gl_lang_lloc.col, gl_lang_lloc.pos);
}

Demo::Runner* Demo::Parser::CreateRunner() {
    return instance().createRunner();
}

// public grammar interface

QString Demo::Parser::Name() {
    return instance().objectName();
}



void Demo::Parser::AddSymbol(Symbol* s) {
    instance().addSymbol(s);
}


const Demo::Parser::SymbolMap& Demo::Parser::Symbols() {
    return instance().symbols();
}

void Demo::Parser::SetCode(const QString& s) {
    instance().setCode(s);
}

void Demo::Parser::PushBack(unsigned op, unsigned lrtype, int inc) {
    instance().pushBack(op, lrtype, inc);
}

void Demo::Parser::SetJump() {
    instance().setJump();
}

void Demo::Parser::InitJump() {
    instance().initJump();
}

void Demo::Parser::PushBackImmed(int con) {
    instance().pushBackImmed(con);
}

void Demo::Parser::PushBackImmed(Math3D::Real con) {
    instance().pushBackImmed(con);
}

void Demo::Parser::PushBackImmed(const QVariant& con) {
    instance().pushBackImmed(con);
}

// Dtor

Demo::Parser::~Parser() {
    qDeleteAll(mSymbols);
}


// private

Demo::Parser::Parser()
    : QObject(),
    mStackSize(0),
    mStackPos(0),
    mCodeSize(0),
    mImmedSize(0) {

    // shared matrices
    addSymbol(new Var::Shared::Matrix("camera"));
    addSymbol(new Var::Shared::Matrix("projection"));
    // shared time variable
    addSymbol(new Var::Shared::Natural("time"));

    // in glwidget
    mSharedCounts["camera"] = 1;
    mSharedCounts["projection"] = 1;
    mSharedCounts["time"] = 1;

    // constants
    Constants cons;
    foreach(Symbol* con, cons.contents) addSymbol(con);

    // functions
    Functions funcs;
    foreach(Symbol* func, funcs.contents) addSymbol(func);


}

Demo::Parser& Demo::Parser::instance() {
    static Parser* instance = new Parser();
    return *instance;
}

// private GL interface

void Demo::Parser::init(const QString& name) {

    setObjectName(name);

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

Demo::Runner* Demo::Parser::createRunner() {
    Runner* runner = new Runner(mAssignments, mVariables, mFunctions, mStackSize);

    connect(runner, SIGNAL(shared_deleted(QStringList)), this, SLOT(shared_deleted(QStringList)));

    foreach(Variable* v, mVariables) {
        if (v->shared() && v->used()) {
            if (!mSharedCounts.contains(v->name())) {
                mSharedCounts[v->name()] = 0;
            }
            mSharedCounts[v->name()] = mSharedCounts[v->name()] + 1;
            qDebug() << "created: count of" << v->name() << "is" << mSharedCounts[v->name()];
        }
    }

    return runner;
}



void Demo::Parser::shared_deleted(const QStringList& shared) {
    foreach(const QString& name, shared) {
        mSharedCounts[name] = mSharedCounts[name] - 1;
        qDebug() << "deleted: count of" << name << "is" << mSharedCounts[name];
    }
}

// private grammar interface
const Demo::Parser::SymbolMap& Demo::Parser::symbols() const {
    return mSymbols;
}

void Demo::Parser::addSymbol(Symbol* sym, bool used) {
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


void Demo::Parser::setCode(const QString& name) {
    mAssignments.append(Assignment(name, mCurrent, mCurrImmed, gl_lang_lloc.pos));
    mCurrent.clear();
    mCurrImmed.clear();
    mCodeSize = 0;
    mImmedSize = 0;
    // qDebug() << name << ": assignment: Byte code ready. Stack size =" << mStackSize;
}

void Demo::Parser::pushBack(unsigned op, unsigned lrtype, int inc) {
    mCurrent.append((op & 0xfff) | ((lrtype & 0xff) << 12));
    mStackPos += inc;
    if (mStackSize < mStackPos) mStackSize = mStackPos;
}

void Demo::Parser::setJump() {
    mStackPos = 0;
    if (mCodeSize) {
        mCurrent[mCodeSize - 2] = mCurrent.size() - mCodeSize;
        mCurrent[mCodeSize - 1] = mCurrImmed.size() - mImmedSize;
    }
}

void Demo::Parser::initJump() {
    mStackPos = 0;
    mCodeSize = mCurrent.size();
    mImmedSize = mCurrImmed.size();
}


void Demo::Parser::pushBackImmed(int constVal) {
    mCurrImmed.append(QVariant(constVal));
}

void Demo::Parser::pushBackImmed(Math3D::Real constVal) {
    mCurrImmed.append(QVariant(constVal));
}

void Demo::Parser::pushBackImmed(const QVariant& constVal) {
    mCurrImmed.append(constVal);
}




