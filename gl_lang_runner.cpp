#include "gl_lang_runner.h"
#include "gl_functions.h"
#include "scope.h"
#include "value.h"

using Math3D::Real;
using Math3D::Vector4;
using Math3D::Matrix4;

using namespace Demo::GL;

Runner::Runner(QObject* parent):
    QObject(parent),
    mStatements(),
    mVariables(),
    mFunctions() {}

void Runner::setup(const StatementVector& sts,
                   const VariableMap& vars,
                   const FunctionVector& funcs) {

    qDeleteAll(mStatements);
    mStatements.clear();
    qDeleteAll(mVariables);
    mVariables.clear();


    mFunctions = funcs;

    for (const Statement::Statement* s: sts) {
        mStatements.append(s->clone());
    }

    for (const Variable* v: vars.values()) {
        mVariables[v->index()] = v->clone();
    }
}


void Runner::run() {
    int index = 0;
    while (index < mStatements.size()) {
        Statement::Statement* s = mStatements[index];
        try {
            index += s->exec_and_jump(mVariables, mFunctions);
        } catch (RunError& e) {
            throw RunError(e.msg(), s->pos());
        } catch (GL::GLError& e) {
            throw RunError(e.msg(), s->pos());
        } catch (ValueError& e) {
            throw RunError(e.msg(), s->pos());
        }
    }
}


Runner::~Runner() {
    qDeleteAll(mStatements);
    qDeleteAll(mVariables);
}




