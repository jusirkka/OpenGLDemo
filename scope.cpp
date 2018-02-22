#include "scope.h"
#include "constant.h"
#include "gl_widget.h"
#include "gl_lang_runner.h"
#include "codeeditor.h"
#include "statement.h"
#include "typedef.h"
#include "project.h"

using namespace Demo;

class Dispatcher: public Function {
public:
    Dispatcher(Scope* p);
    const QVariant& execute(const QVector<QVariant>& vals, int start) override;
    Dispatcher* clone() const override;
private:
    Scope* mParent;
};


Dispatcher::Dispatcher(Scope* p)
    : Function("dispatch", new Integer_T)
    , mParent(p)
{
    mArgTypes.append(new Text_T);
}

const QVariant& Dispatcher::execute(const QVector<QVariant>& vals, int start) {
    QString other = vals[start].toString();
    mParent->dispatch(other);
    mValue.setValue(0);
    return mValue;
}

Dispatcher* Dispatcher::clone() const {
    return new Dispatcher(*this);
}

Scope::Scope(GLWidget* glContext, QObject *parent)
    : ProjectFolder("scope", parent)
{
    // GL functions, constants & variables
    glContext->addGLSymbols(mSymbols, mExports);

    // constants
    Constants cons;
    for (auto con: qAsConst(cons.contents)) mSymbols[con->name()] = con;

    // functions
    Functions funcs;
    for (auto sym: qAsConst(funcs.contents)) mSymbols[sym->name()] = sym;

    // types
    Basetypes types;
    for (auto sym: qAsConst(types.contents)) mSymbols[sym->name()] = sym;

    Function* dispatcher = new Dispatcher(this);
    mSymbols[dispatcher->name()] = dispatcher;

    // process functions
    for (auto sym: qAsConst(mSymbols)) {
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
    ProjectFolder("scope")
{
    for (auto sym: s.symbols()) {
        mSymbols[sym->name()] = sym->clone();
    }

    for (auto v: s.exports()) {
        mExports[v->name()] = v->clone();
    }

    // replace dispatcher
    Function* dispatcher = new Dispatcher(this);
    if (mSymbols.contains(dispatcher->name())) {
        delete mSymbols[dispatcher->name()];
    }
    mSymbols[dispatcher->name()] = dispatcher;

    // process functions
    for (auto sym: qAsConst(mSymbols)) {
        Function* fun = dynamic_cast<Function*>(sym);
        if (fun) {
            fun->setIndex(mFunctions.size() + FunctionOffset);
            mFunctions.append(fun);
        }
    }
}

Scope* Scope::clone(QObject* parent) const {
    auto s = new Scope(*this);
    s->setParent(parent);
    return s;
}

const SymbolMap& Scope::symbols() const {
    return mSymbols;
}


const FunctionVector& Scope::functions() const {
    return mFunctions;
}

bool Scope::subscriptRelation(const QString& top, const QString& sub) {
    GL::Compiler* c = compiler(top);
    if (!c) return false;
    if (top == sub) return true;
    for (auto& name: c->subscripts()) {
        if (subscriptRelation(name, sub)) return true;
    }
    return false;
}

void Scope::dispatch(const QString& other) const {
    // qCDebug(OGL) << "dispatch" << other;
    if (mEditorIndices.contains(other)) {
        CodeEditor* other_ed = mEditors[mEditorIndices[other]];
        other_ed->run();
    } else {
        throw RunError(QString("Script %1 not found").arg(other), 0);
    }
}

const Scope::EditorVector& Scope::editors() const {
    return mEditors;
}

void Scope::setItem(const QString& key, const QString& path) {
    QString code;
    if (!path.isEmpty()) {
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            code = QString(file.readAll()).append('\n');
            file.close();
        } else {
            code = ERR_CODE;
        }
    } else if (key == INIT_NAME) {
        code = INIT_CODE;
    } else if (key == DRAW_NAME) {
        code = DRAW_CODE;
    } else {
        code = DEFAULT_CODE;
    }

    CodeEditor* ed;
    if (mEditorIndices.contains(key)) {
        ed = mEditors[mEditorIndices[key]];
        ed->setPlainText(code);
    } else {
        auto project = qobject_cast<Project*>(parent());
        ed = new CodeEditor(key, this, project, code);
        mEditorIndices[ed->objectName()] = mEditors.size();
        mEditors.append(ed);
    }

    if (code != ERR_CODE) {
        ed->setFileName(path);
    }
}

void Scope::remove(int index) {
    if (index < 0 || index >= mEditors.size()) return;
    mEditors.removeAt(index);
    mEditorIndices.clear();
    for (int idx = 0; idx < mEditors.size(); ++idx) {
        CodeEditor* ed = mEditors[idx];
        mEditorIndices[ed->objectName()] = idx;
    }
}

void Scope::rename(const QString& from, const QString& to) {
    if (!mEditorIndices.contains(from)) return;
    int index = mEditorIndices.take(from);
    mEditorIndices[to] = index;
    mEditors[index]->setObjectName(to);
}

QStringList Scope::items() const {
    QStringList r;
    for (auto ed: mEditors) {
        r.append(ed->objectName());
    }
    return r;
}


int Scope::size() const {
    return mEditors.size();
}

QString Scope::fileName(int index) const {
    return mEditors[index]->fileName();
}

QString Scope::itemName(int index) const {
    return mEditors[index]->objectName();
}

CodeEditor* Scope::editor(const QString& name) const {
    if (!mEditorIndices.contains(name)) return nullptr;
    return mEditors[mEditorIndices[name]];
}


CodeEditor* Scope::editor(int index) const {
    if (index < 0 || index >= mEditors.size()) return nullptr;
    return mEditors[index];
}

GL::Compiler* Scope::compiler(const QString& name) const {
    CodeEditor* ed = editor(name);
    if (ed) return ed->compiler();
    return nullptr;
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
    EditorVector currFailed;
    for (auto ed: qAsConst(mEditors)) {
        ed->compile();
        if (!ed->compiler()->ready()) {
            // qCDebug(OGL) << ed->objectName() << ": compile failed";
            currFailed.append(ed);
        }
    }
    EditorVector prevFailed;
    while (!currFailed.isEmpty() && prevFailed != currFailed) {
        // qCDebug(OGL) << "num failed = " << currFailed.size();
        prevFailed = currFailed;
        currFailed.clear();
        for (auto ed: qAsConst(prevFailed)) {
            ed->compile();
            if (!ed->compiler()->ready()) {
                // qCDebug(OGL) << ed->objectName() << ": compile failed";
                currFailed.append(ed);
            }
        }
    }

    // qCDebug(OGL) << "final num failed = " << currFailed.size();
}
