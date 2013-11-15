#ifndef RUNNER_H
#define RUNNER_H

#include "parser.h"

#include <QString>
#include <QStringList>
#include <QStack>
#include <QMap>
#include <QVariant>
#include <QtDebug>

namespace Demo {

class Runner: public QObject {

    Q_OBJECT

public:


    Runner(
       const Parser::AssignmentList& ass,
       const Parser::VariableList& vars,
       const Parser::FunctionList& funcs,
       int stackSize);


    ~Runner();

public slots:

    void evaluate();

private:

    typedef Parser::CodeStack CodeStack;
    typedef Parser::ValueStack ValueStack;
    typedef Parser::AssignmentList AssignmentList;
    typedef Parser::VariableList VariableList;
    typedef Parser::FunctionList FunctionList;
    typedef Parser::Assignment Assignment;

    typedef QMap<QString, int> IndexMap;

private:

    Runner(const Runner&); // Not implemented
    Runner &operator=(const Runner&); // Not implemented

    const QVariant& evalCode(const CodeStack& code, const ValueStack& immed);

    static unsigned LRType(unsigned code);
    static unsigned Code(unsigned code);

private:

    AssignmentList mAssignments;
    VariableList mVariables;
    FunctionList mFunctions;
    ValueStack mStack;
    QString mEvalError;
    QStringList mShared;
    IndexMap mIndex;

signals:

    void shared_deleted(const QStringList&);
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


}

#endif // RUNNER_H
