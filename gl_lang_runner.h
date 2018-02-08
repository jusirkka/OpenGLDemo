#ifndef RUNNER_H
#define RUNNER_H

#include "gl_lang_compiler.h"

#include <QString>
#include <QStringList>
#include <QStack>
#include <QMap>
#include <QVariant>
#include <QtDebug>

namespace Demo {
namespace GL {


class Runner: public QObject {

    Q_OBJECT

public:

    Runner(QObject* parent = nullptr);

    using StatementVector = Compiler::StatementVector;
    using FunctionVector = Compiler::FunctionVector;

    void setup(const StatementVector& sts, const VariableMap& vars, const FunctionVector& funcs);

    ~Runner() override;

public slots:

    void run();


private:

    Runner(const Runner&); // Not implemented
    Runner &operator=(const Runner&); // Not implemented

    using VariableIndexMap = Demo::Statement::Statement::VariableIndexMap;


private:

    StatementVector mStatements;
    VariableIndexMap mVariables;
    FunctionVector mFunctions;
};




}}

#endif // RUNNER_H
