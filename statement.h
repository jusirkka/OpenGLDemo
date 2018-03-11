#ifndef STATEMENT_H
#define STATEMENT_H

#include "variable.h"
#include "function.h"


namespace Demo {


namespace Statement {

class Statement {
public:

    using VariableIndexMap = QMap<unsigned, Variable*>;
    using FunctionVector = QVector<Function*>;
    using CodeStack = QVector<unsigned int>;
    using ValueStack = QVector<QVariant>;

    Statement(CodeStack code, ValueStack immed, unsigned stackSize, int pos);
    Statement(int pos);

    virtual int exec_and_jump(VariableIndexMap& vars, const FunctionVector& funcs) = 0;
    virtual Statement* clone() const = 0;


    int pos() {return mPos;}

protected:

    const QVariant& evalCode(const VariableIndexMap& vars, const FunctionVector& funcs);

protected:

    CodeStack mCode;
    ValueStack mImmed;
    ValueStack mStack;
    int mPos;

};

class Assignment: public Statement {

public:

    Assignment(CodeStack c, ValueStack i, unsigned stackSize, int p)
        : Statement(c, i, stackSize, p) {}

    int exec_and_jump(VariableIndexMap& vars, const FunctionVector& funcs) override;
    Assignment* clone() const override {return new Assignment(*this);}

};

class BaseJump: public Statement {

public:

    BaseJump(CodeStack code, ValueStack immed, unsigned stackSize, int pos)
        : Statement(code, immed, stackSize, pos)
        , mJump(0) {}
    BaseJump(int pos, int jump)
        : Statement(pos)
        , mJump(jump) {}

    void setJump(int jump) {mJump = jump;}

protected:

    int mJump;
};

class Jump: public BaseJump {

public:

    Jump(int pos, int jump = 0) : BaseJump(pos, jump) {}
    int exec_and_jump(VariableIndexMap&, const FunctionVector&) override {return mJump;}
    Jump* clone() const override {return new Jump(*this);}

};

class CondJump: public BaseJump {

public:

    CondJump(CodeStack c, ValueStack i, unsigned stackSize, int p)
        : BaseJump(c, i, stackSize, p) {}

    int exec_and_jump(VariableIndexMap& vars, const FunctionVector& funcs) override;
    CondJump* clone() const override {return new CondJump(*this);}

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

using QFunc = void (*)(QVariant &);
using QIFunc = void (*)(QVariant &, int);
using QQFunc = void (*)(QVariant &, const QVariant &);
using BQQFunc = bool (*)(QVariant &, const QVariant &);

}}

#endif // STATEMENT_H
