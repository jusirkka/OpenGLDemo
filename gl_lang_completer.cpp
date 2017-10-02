#include <QtGlobal>
#include <QAbstractItemView>
#include <QStringListModel>
#include <QPlainTextEdit>
#include <QScrollBar>

#include "gl_lang_completer.h"

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
#include "codeeditor.h"
#include "gl_lang_compiler.h"

using Math3D::Real;
using Math3D::Vector4;
using Math3D::Matrix4;

using namespace Demo::GL;


Completer::Completer(Scope* globalScope, CodeEditor *parent):
    QObject(parent),
    mScanner(nullptr),
    mCompletions(),
    mGlobalScope(globalScope),
    mCompleter(new QCompleter(parent)),
    mCompletionPos(-1) {

    mCompleter->setWidget(parent);
    mCompleter->setWrapAround(false);
    connect(mCompleter, SIGNAL(activated(QString)), parent, SLOT(insertCompletion(const QString&)));
}


// public GL interface



void Completer::complete(const QString& script, int completionPos) {

    qDeleteAll(mSymbols);
    mSymbols.clear();
    mVariables.clear();
    addVariable(new Var::Local::Natural("gl_result"));

    mCompletions = CompleterException();

    gl_lang_lex_init(&mScanner);
    mCompletionPos = completionPos;
    QString source = script;
    // ensure that the source ends with newlines
    YY_BUFFER_STATE buf = gl_lang__scan_string(source.append("\n\n").toUtf8().data(), mScanner);
    int err = gl_lang_parse(this, mScanner);
    gl_lang__delete_buffer(buf, mScanner);
    if (err && !mCompletions.completions().isEmpty()) throw mCompletions;
}

bool Completer::popupVisible() const {
    return mCompleter->popup()->isVisible();
}

void Completer::popupCompletions(const QStringList& completions) {
    mCompleter->setModel(new QStringListModel(completions, mCompleter));
    updatePopup(mCompletions.prefix());
}

void Completer::hidePopup() {
    mCompleter->popup()->hide();
}

void Completer::updatePopup(const QString& prefix) {
    qDebug() << "updatepopup" << prefix;
    if (!prefix.startsWith(mCompletions.prefix())) {
        hidePopup();
        return;
    }
    mCompleter->setCompletionPrefix(prefix);
    mCompleter->popup()->setCurrentIndex(mCompleter->completionModel()->index(0, 0));
    QRect pos = (qobject_cast<QPlainTextEdit*>(mCompleter->parent()))->cursorRect();
    pos.setWidth(mCompleter->popup()->sizeHintForColumn(0) + mCompleter->popup()->verticalScrollBar()->sizeHint().width());
    mCompleter->complete(pos);
}



void Completer::createError(const QString&, Error) {
    // noop
}

static void addCompletion(QStringList& c, const QString& symbol, const QString& candidate) {
    if (symbol.startsWith(candidate)) c.append(symbol);
}

bool Completer::createCompletion(const IdentifierType &id, unsigned mask) {


    if (mCompletionPos < id.pos) return false;
    if (mCompletionPos > id.pos + id.name.length()) return false;

    qDebug() << "Completion:" << id.name << id.pos << mCompletionPos;

    QStringList completions;

    QList<SymbolIterator> its;
    its << SymbolIterator(mGlobalScope->symbols()) << SymbolIterator(mSymbols);
    for (auto& it: its) {
        while (it.hasNext()) {
            it.next();
            if (dynamic_cast<Variable*>(it.value()) && (mask & CompleteVariables)) {
                addCompletion(completions, it.value()->name(), id.name);
            } else if (dynamic_cast<Function*>(it.value()) && (mask & CompleteFunctions)) {
                addCompletion(completions, it.value()->name(), id.name);
            } else if (dynamic_cast<Constant*>(it.value()) && (mask & CompleteConstants)) {
                addCompletion(completions, it.value()->name(), id.name);
            }
        }
    }

    if (completions.isEmpty()) return false;

    mCompletions = CompleterException(id.name, completions);
    return true;
}

Completer::~Completer() {
    qDeleteAll(mSymbols);
}



void Completer::addVariable(Variable* v) {
    // qDebug() << "adding" << objectName() << v->name();
    v->setIndex(mVariables.size() + Scope::VariableOffset);
    mVariables.append(v);
    mSymbols[v->name()] = v;
    if (v->shared()) {
        // qDebug() << "exporting" << objectName() << v->name();
        mExports[v->name()] = v;
    }
}

bool Completer::hasSymbol(const QString& sym) const {
    if (mGlobalScope->symbols().contains(sym)) return true;
    return mSymbols.contains(sym);
}

Demo::Symbol* Completer::symbol(const QString& sym) const {
    if (mGlobalScope->symbols().contains(sym)) return mGlobalScope->symbols()[sym];
    if (mSymbols.contains(sym)) return mSymbols[sym];
    return nullptr;
}

bool Completer::isImported(const Variable* var) const {
    if (!mSymbols.contains(var->name())) return false;
    // local, imported or exported variable
    if (mExports.contains(var->name())) return false;
    // local (not shared) or imported (shared)
    return var->shared();
}

bool Completer::isExported(const QString& name, const QString& script) const {
    if (script == "") { // global scope
        return mGlobalScope->exports().contains(name);
    }
    Compiler* c = mGlobalScope->compiler(script);
    if (!c) return false;
    return c->exports().contains(name);
}

void Completer::addImported(const QString& name, const QString& script) {
    Variable* v;
    if (script == "") { // global scope
        v = mGlobalScope->exports().value(name)->clone();
    } else {
        v = mGlobalScope->compiler(script)->exports().value(name)->clone();
    }

    v->setIndex(mVariables.size() + Scope::VariableOffset);
    mVariables.append(v);
    mSymbols[v->name()] = v;

}

bool Completer::isScript(const QString& name) const {
    return mGlobalScope->compiler(name) != nullptr;
}

void Completer::addSubscript(const QString&) {
    // noop
}

void Completer::setCode(const QString&) {
    // noop
}

void Completer::pushBack(unsigned, unsigned, int) {
    // noop
}

void Completer::setJump() {
    // noop
}

void Completer::initJump() {
    // noop
}


void Completer::pushBackImmed(int) {
    // noop
}

void Completer::pushBackImmed(Math3D::Real) {
    // noop
}

void Completer::pushBackImmed(const QVariant&) {
    // noop
}
