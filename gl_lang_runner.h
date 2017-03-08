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

class RunError {

public:
    RunError(const QString& msg, int pos)
        :emsg(msg),
          epos(pos)
    {}

    const QString msg() const {return emsg;}
    int pos() const {return epos;}

private:

    QString emsg;
    int epos;

};



class Runner: public QObject {

    Q_OBJECT

public:

    Runner(QObject* parent = 0);


    void setup(
            const Compiler::AssignmentList& ass,
            const Compiler::VariableList& vars,
            const Compiler::FunctionList& funcs,
            int stackSize);


    ~Runner();

public slots:

    void run();

private:

    typedef Compiler::CodeStack CodeStack;
    typedef Compiler::ValueStack ValueStack;
    typedef Compiler::AssignmentList AssignmentList;
    typedef Compiler::VariableList VariableList;
    typedef Compiler::FunctionList FunctionList;
    typedef Compiler::Assignment Assignment;

    typedef QMap<QString, int> IndexMap;

private:

    Runner(const Runner&); // Not implemented
    Runner &operator=(const Runner&); // Not implemented

    const QVariant& evalCode(const CodeStack& code, const ValueStack& immed, int pos);

    static unsigned LRType(unsigned code);
    static unsigned Code(unsigned code);

private:

    AssignmentList mAssignments;
    VariableList mVariables;
    FunctionList mFunctions;
    ValueStack mStack;
    QStringList mShared;
    IndexMap mIndex;
};


template<typename R> void Neg(QVariant& right) {
    right.setValue(- right.value<R>());
}

template<typename L> void Take(QVariant& left, int index) {
    left.setValue(left.value<L>()[index]);
}

template<typename L> void Vec(QVariant& left, int index) {
    left.setValue(Math3D::Vector4(left.value<L>()[index]));
}

template<typename L, typename R> void Add(QVariant& left, const QVariant& right) {
    left.setValue(left.value<L>() + right.value<R>());
}

template<typename L, typename R> void Sub(QVariant& left, const QVariant& right) {
    left.setValue(left.value<L>() - right.value<R>());
}

template<typename L, typename R> void Mul(QVariant& left, const QVariant& right) {
    left.setValue(left.value<L>() * right.value<R>());
}

template<typename L, typename R> bool Div(QVariant& left, const QVariant& right) {
    if (right.value<R>() == 0) return false;
    left.setValue(left.value<L>() / right.value<R>());
    return true;
}

template<typename L, typename R> bool Eq(QVariant& left, const QVariant& right) {
    return left.value<L>() == right.value<R>();
}

template<typename L, typename R> bool Lt(QVariant& left, const QVariant& right) {
    return left.value<L>() < right.value<R>();
}

template<typename L, typename R> bool Gt(QVariant& left, const QVariant& right) {
    return left.value<L>() > right.value<R>();
}

typedef void (*QFunc)(QVariant&);
typedef void (*QIFunc)(QVariant&, int);
typedef void (*QQFunc)(QVariant&, const QVariant&);
typedef bool (*BQQFunc)(QVariant&, const QVariant&);


}}

#endif // RUNNER_H
