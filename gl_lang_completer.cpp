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
#include "typedef.h"
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

    gl_lang_lex_init(&mScanner);

    mReserved << "Shared" << "Execute" << "From" << "import" <<
                 "While" << "Endwhile" << "If" << "Else" << "Elsif" << "Endif" <<
                 "Array" << "of" << "Type" << "Var" << "Record";

    mCompleter->setWidget(parent);
    mCompleter->setWrapAround(false);
    connect(mCompleter, SIGNAL(activated(QString)), parent, SLOT(insertCompletion(const QString&)));
}


// public GL interface



void Completer::complete(const QString& script, int completionPos) {

    qDeleteAll(mSymbols);
    mSymbols.clear();
    mExports.clear();
    addSymbol(new LocalVar("gl_result", new Integer_T));

    mCompletions = CompleterException();

    mCompletionPos = completionPos;
    // ensure that the source ends with newlines
    QString source = "\n" + script + "\n\n";
    YY_BUFFER_STATE buf = gl_lang__scan_string(source.toUtf8().data(), mScanner);
    int err = gl_lang_parse(this, mScanner);
    if (err && mCompletions.completions().isEmpty()) {
        IdentifierType id;
        id.pos = gl_lang_get_lloc(mScanner)->pos;
        id.name = QString(gl_lang_get_text(mScanner));
        createCompletion(id, CR);
    }

    gl_lang__flush_buffer(buf, mScanner);
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
    // qDebug() << "updatepopup" << prefix;
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



static void addCompletion(QStringList& c, const QString& symbol, const QString& candidate) {
    if (symbol.startsWith(candidate)) c.append(symbol);
}

bool Completer::createCompletion(const IdentifierType &id, unsigned mask) {


    if (mCompletionPos < id.pos) return false;
    if (mCompletionPos > id.pos + id.name.length()) return false;

    // qDebug() << "Completion:" << id.name << id.pos << mCompletionPos;

    QStringList completions;

    QList<SymbolIterator> its;
    its << SymbolIterator(mGlobalScope->symbols()) << SymbolIterator(mSymbols);
    for (auto& it: its) {
        while (it.hasNext()) {
            it.next();
            if (dynamic_cast<Variable*>(it.value()) && (mask & CR)) {
                addCompletion(completions, it.value()->name(), id.name);
            } else if (dynamic_cast<Function*>(it.value()) && (mask & CF)) {
                addCompletion(completions, it.value()->name(), id.name);
            } else if (dynamic_cast<Constant*>(it.value()) && (mask & CC)) {
                addCompletion(completions, it.value()->name(), id.name);
            } else if (dynamic_cast<Typedef*>(it.value()) && (mask & CT)) {
                addCompletion(completions, it.value()->name(), id.name);
            }
        }
    }
    if (mask & CR) {
        for (const QString& word: mReserved) {
            addCompletion(completions, word, id.name);
        }
    }


    if (completions.isEmpty()) return false;

    mCompletions = CompleterException(id.name, completions);
    return true;
}

Completer::~Completer() {
    qDeleteAll(mSymbols);
    gl_lang_lex_destroy(mScanner);
}



void Completer::addSymbol(Symbol* s) {
    auto v = dynamic_cast<Variable*>(s);
    if (v && v->shared()) {
        mExports[v->name()] = v;
    }
    mSymbols[s->name()] = s;
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
    mSymbols[v->name()] = v;

}

bool Completer::isScript(const QString& name) const {
    return mGlobalScope->compiler(name) != nullptr;
}
