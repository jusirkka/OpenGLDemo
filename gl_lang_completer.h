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

    CompleterException(const QString& prefix, const QStringList& completions):
        mPrefix(prefix),
        mCompletions(completions)
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

    typedef QList<Variable*> VariableList;

public:

    Completer(Scope* globalScope, CodeEditor* parent);

    // GL interface
    void complete(const QString& script, int completionPos);
    bool popupVisible() const;
    void popupCompletions(const QStringList& completions);
    void hidePopup();
    void updatePopup(const QString& prefix);

    // grammar interface
    void setCode(const QString& name);
    void pushBack(unsigned op, unsigned lrtype, int inc);
    void setJump();
    void initJump();
    void pushBackImmed(int constVal);
    void pushBackImmed(Math3D::Real constVal);
    void pushBackImmed(const QVariant& constVal);
    void createError(const QString& item, Error err);
    bool createCompletion(const IdentifierType& id, unsigned completionMask);
    void addVariable(Variable* v);
    bool hasSymbol(const QString& sym) const;
    Symbol* symbol(const QString& sym) const;
    bool isImported(const Variable* var) const;
    bool isExported(const QString& v, const QString& script) const;
    void addImported(const QString& v, const QString& script);
    bool isScript(const QString& name) const;
    void addSubscript(const QString& name);

    ~Completer();


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
