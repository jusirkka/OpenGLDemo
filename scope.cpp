#include "scope.h"
#include "constant.h"
#include "gl_widget.h"
#include "gl_lang_runner.h"
#include "codeeditor.h"

using namespace Demo;

class Dispatcher: public Function {
public:
    Dispatcher(Scope* p);
    const QVariant& execute(const QVector<QVariant>& vals, int start);
    Dispatcher* clone() const;
    ~Dispatcher();
private:
    Scope* mParent;
};


Dispatcher::Dispatcher(Scope* p):
    Function("dispatch", Symbol::Integer),
    mParent(p) {
    int argt = Symbol::Text;
    mArgTypes.append(argt);
}

const QVariant& Dispatcher::execute(const QVector<QVariant>& vals, int start) {
    QString other = vals[start].value<QString>();
    mParent->dispatch(other);
    mValue.setValue(0);
    return mValue;
}

Dispatcher* Dispatcher::clone() const {
    return new Dispatcher(*this);
}

Dispatcher::~Dispatcher() {}

Scope::Scope(GLWidget* glContext, QObject *parent):
    QObject(parent)
{
    // GL functions, constants & variables
    glContext->addGLSymbols(mSymbols, mExports);

    // constants
    Constants cons;
    foreach(Symbol* con, cons.contents) mSymbols[con->name()] = con;

    // functions
    Functions funcs;
    foreach(Symbol* sym, funcs.contents) mSymbols[sym->name()] = sym;

    Function* dispatcher = new Dispatcher(this);
    mSymbols[dispatcher->name()] = dispatcher;

    // process functions
    foreach(Symbol* sym, mSymbols) {
        Function* fun = dynamic_cast<Function*>(sym);
        if (fun) {
            fun->setIndex(mFunctions.size() + FunctionOffset);
            mFunctions.append(fun);
        }
    }
}

Scope::~Scope() {
    qDeleteAll(mEditors);
}

Scope::Scope(const Scope& s):
    QObject()
{
    foreach (Symbol* sym, s.symbols()) {
        mSymbols[sym->name()] = sym->clone();
    }

    foreach (Variable* v, s.exports()) {
        mExports[v->name()] = v->clone();
    }

    // replace dispatcher
    Function* dispatcher = new Dispatcher(this);
    if (mSymbols.contains(dispatcher->name())) {
        delete mSymbols[dispatcher->name()];
    }
    mSymbols[dispatcher->name()] = dispatcher;

    // process functions
    foreach(Symbol* sym, mSymbols) {
        Function* fun = dynamic_cast<Function*>(sym);
        if (fun) {
            fun->setIndex(mFunctions.size() + FunctionOffset);
            mFunctions.append(fun);
        }
    }
}

Scope* Scope::clone(QObject* parent) const {
    Scope* s = new Scope(*this);
    s->setParent(parent);
    return s;
}

const SymbolMap& Scope::symbols() const {
    return mSymbols;
}


const FunctionList& Scope::functions() const {
    return mFunctions;
}

bool Scope::subscriptRelation(const QString& top, const QString& sub) {
    GL::Compiler* c = compiler(top);
    if (!c) return false;
    if (top == sub) return true;
    foreach (QString name, c->subscripts()) {
        if (subscriptRelation(name, sub)) return true;
    }
    return false;
}

void Scope::dispatch(const QString& other) const {
    // qDebug() << "dispatch" << other;
    if (mEditorIndices.contains(other)) {
        CodeEditor* other_ed = mEditors[mEditorIndices[other]];
        other_ed->run();
    } else {
        throw GL::RunError(QString("Script %1 not found").arg(other), 0);
    }
}

const Scope::EditorList& Scope::editors() const {
    return mEditors;
}

void Scope::appendEditor(CodeEditor* editor, const QString& script, const QString& file) {
    editor->setPlainText(script);
    editor->setFileName(file);
    mEditorIndices[editor->objectName()] = mEditors.size();
    mEditors.append(editor);
}

void Scope::removeEditor(int index) {
    if (index < 0 || index >= mEditors.size()) return;
    mEditors.removeAt(index);
    mEditorIndices.clear();
    for (int idx = 0; idx < mEditors.size(); ++idx) {
        CodeEditor* ed = mEditors[idx];
        mEditorIndices[ed->objectName()] = idx;
    }
}

CodeEditor* Scope::editor(const QString& name) const {
    if (!mEditorIndices.contains(name)) return 0;
    return mEditors[mEditorIndices[name]];
}


CodeEditor* Scope::editor(int index) const {
    if (index < 0 || index >= mEditors.size()) return 0;
    return mEditors[index];
}

GL::Compiler* Scope::compiler(const QString& name) const {
    CodeEditor* ed = editor(name);
    if (ed) return ed->compiler();
    return 0;
}

void Scope::rename(CodeEditor* ed, const QString& name) {
    if (!mEditorIndices.contains(ed->objectName())) return;
    if (mEditors[mEditorIndices[ed->objectName()]] != ed) return;

    int index = mEditorIndices.take(ed->objectName());
    mEditorIndices[name] = index;
    ed->setObjectName(name);
}

QStringList Scope::itemSample(const QString& except) const {
    QStringList r;
    foreach (CodeEditor* ed, mEditors) {
        if (!except.isEmpty() && ed->objectName() == except) continue;
        r.append(ed->objectName());
    }
    return r;
}

const VariableMap& Scope::exports() const {
    return mExports;
}

void Scope::addFunction(Function* f) {
    if (mSymbols.contains(f->name())) {
        Function* old = dynamic_cast<Function*>(mSymbols[f->name()]);
        int idx = mFunctions.indexOf(old);
        if (idx > 0) {
            mFunctions[idx] = f;
        }
        f->setIndex(idx + FunctionOffset);
        delete mSymbols[f->name()];
    } else {
        f->setIndex(mFunctions.size() + FunctionOffset);
        mFunctions.append(f);
    }
    mSymbols[f->name()] = f;
}

void Scope::recompileAll() {
    EditorList currFailed;
    foreach (CodeEditor* ed, mEditors) {
        ed->compile();
        if (!ed->compiler()->ready()) {
            // qDebug() << ed->objectName() << ": compile failed";
            currFailed.append(ed);
        }
    }
    EditorList prevFailed;
    while (!currFailed.isEmpty() && prevFailed != currFailed) {
        // qDebug() << "num failed = " << currFailed.size();
        prevFailed = currFailed;
        currFailed.clear();
        foreach (CodeEditor* ed, prevFailed) {
            ed->compile();
            if (!ed->compiler()->ready()) {
                qDebug() << ed->objectName() << ": compile failed";
                currFailed.append(ed);
            }
        }
    }

    // qDebug() << "final num failed = " << currFailed.size();
}
