#ifndef SCOPE_H
#define SCOPE_H

#include "symbol.h"
#include "function.h"
#include "variable.h"
#include "projectfolder.h"

namespace Demo {

class GLWidget;
class CodeEditor;
class Project;

namespace GL {
class Compiler;
}

class Scope: public ProjectFolder
{
    Q_OBJECT

public:

    using EditorVector = QVector<Demo::CodeEditor*>;
    using IndexMap = QMap<QString, int>;

    // code offsets: smaller than 0xfff = 4095 (see Compiler::pushBack)
    static const int FunctionOffset = 1000;
    static const int VariableOffset = 2000;

    // Project interface
    void rename(const QString& from, const QString& to) override;
    void remove(int index) override;
    int size() const override;
    QString fileName(int) const override;
    QString itemName(int) const override;
    QStringList items() const override;
    void setItem(const QString& key, const QString& path = QString("")) override;

    // Compiler / Runner interface
    Scope(GLWidget* glContext, QObject *parent = nullptr);
    Scope* clone(QObject* parent = nullptr) const;
    const SymbolMap& symbols() const;
    const FunctionVector& functions() const;
    bool subscriptRelation(const QString& top, const QString& sub);
    void dispatch(const QString& other) const;
    GL::Compiler* compiler(const QString& name) const;
    const VariableMap& exports() const;
    void addFunction(Function* f);
    void recompileAll();

    const EditorVector& editors() const;
    CodeEditor* editor(const QString& name) const;
    CodeEditor* editor(int index) const;

    ~Scope() override;


signals:

public slots:

private:

    Scope(const Scope&);

    SymbolMap mSymbols;
    FunctionVector mFunctions;
    VariableMap mExports;
    EditorVector mEditors;
    IndexMap mEditorIndices;
};

}

#endif // SCOPE_H
