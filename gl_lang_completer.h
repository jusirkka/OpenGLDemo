#ifndef GL_LANG_COMPLETER_H
#define GL_LANG_COMPLETER_H

#include "gl_lang_parser_interface.h"

#include <QVector>
#include <QMap>
#include <QtDebug>
#include <QCompleter>


namespace Demo {

class Scope;
class CodeEditor;

namespace GL {


class CompleterException {

public:

    CompleterException(QString prefix, QStringList completions)
        : mPrefix(std::move(prefix))
        , mCompletions(std::move(completions))
    {}

    CompleterException(): mPrefix(), mCompletions() {}
    const QStringList& completions() const {return mCompletions;}
    const QString& prefix() const {return mPrefix;}

private:

    QString mPrefix;
    QStringList mCompletions;
};



class Completer: public QObject, public Parser {

    Q_OBJECT

public:

    using VariableList = QList<Demo::Variable *>;

public:

    Completer(Scope* globalScope, CodeEditor* parent);

    // GL interface
    void complete(const QString& script, int completionPos);
    bool popupVisible() const;
    void popupCompletions(const QStringList& completions);
    void hidePopup();
    void updatePopup(const QString& prefix);

    // grammar interface
    void setCode(const QString& name) override;
    void pushBack(unsigned op, unsigned lrtype, int inc) override;
    void setJump() override;
    void initJump() override;
    void pushBackImmed(int constVal) override;
    void pushBackImmed(Math3D::Real constVal) override;
    void pushBackImmed(const QVariant& constVal) override;
    void createError(const QString& item, Error err) override;
    bool createCompletion(const IdentifierType& id, unsigned completionMask) override;
    void addVariable(Variable* v) override;
    bool hasSymbol(const QString& sym) const override;
    Symbol* symbol(const QString& sym) const override;
    bool isImported(const Variable* var) const override;
    bool isExported(const QString& v, const QString& script) const override;
    void addImported(const QString& v, const QString& script) override;
    bool isScript(const QString& name) const override;
    void addSubscript(const QString& name) override;

    ~Completer() override;


private:

    Completer(const Completer&); // Not implemented
    Completer &operator=(const Completer&); // Not implemented

private:

    VariableList mVariables;
    VariableMap mExports;
    SymbolMap mSymbols;
    yyscan_t mScanner;
    CompleterException mCompletions;
    Scope* mGlobalScope;
    QCompleter* mCompleter;
    int mCompletionPos;
};


}} // namespace Demo::GL


#endif
