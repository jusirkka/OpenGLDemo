#include "runner.h"

using Math3D::Real;
using Math3D::Vector4;
using Math3D::Matrix4;


Demo::Runner::Runner(
    const AssignmentList& ass,
    const VariableList& vars,
    const FunctionList& funcs,
    int stackSize)
    : QObject(),
        mAssignments(ass),
        mVariables(vars),
        mFunctions(funcs),
        mEvalError("NoError")
{
    mStack.resize(stackSize);
    for (int i = 0; i < mVariables.length(); i++) {
        const Variable* v = mVariables.at(i);
        mIndex[v->name()] = i;
        mVariables[i] = v->clone();
        if (v->shared() && v->used()) {
            mShared << v->name();
        }
    }
}


void Demo::Runner::evaluate() {
    foreach (Assignment ass, mAssignments) {
        Variable* v = mVariables[mIndex[ass.var]];
        v->setValue(evalCode(ass.code, ass.immed));
        if (mEvalError != "NoError") {
            qDebug() << mEvalError << "while evaluating" << v->name();
            mEvalError = "NoError";
            return;
        }
        qDebug() << v->name() << "=" << v->value();
    }
}


Demo::Runner::~Runner() {
    // variables are owned by runner, other symbols are shared
    foreach(Variable* v, mVariables) {
        delete v;
    }
    emit shared_deleted(mShared);
}

static void neg_f(QVariant& right, int lrtype) {

    static Demo::QFunc funcs[] = {
        Demo::Neg<int>, Demo::Neg<Real>, Demo::Neg<Vector4>, Demo::Neg<Matrix4>, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0
    };

    funcs[lrtype](right);
}


static void take_f(QVariant& left, int index, int lrtype) {

    static Demo::QIFunc funcs[] = {
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        Demo::Take<Vector4>, 0, 0, 0, 0,
        Demo::Vec<Matrix4>, 0, 0, 0, 0,
        0, 0, 0, 0, 0
    };

    funcs[lrtype](left, index);
}

static void add_f(QVariant& left, const QVariant& right, int lrtype) {

    static Demo::QQFunc funcs[] = {
        Demo::Add<int, int>, Demo::Add<int, Real>, 0, 0, 0,
        Demo::Add<Real, int>, Demo::Add<Real, Real>, 0, 0, 0,
        0, 0, Demo::Add<Vector4, Vector4>, 0, 0,
        0, 0, 0, Demo::Add<Matrix4, Matrix4>, 0,
        0, 0, 0, 0, 0
    };

    funcs[lrtype](left, right);
}

static void sub_f(QVariant& left, const QVariant& right, int lrtype) {

    static Demo::QQFunc funcs[] = {
        Demo::Sub<int, int>, Demo::Sub<int, Real>, 0, 0, 0,
        Demo::Sub<Real, int>, Demo::Sub<Real, Real>, 0, 0, 0,
        0, 0, Demo::Sub<Vector4, Vector4>, 0, 0,
        0, 0, 0, Demo::Sub<Matrix4, Matrix4>, 0,
        0, 0, 0, 0, 0
    };

    funcs[lrtype](left, right);
}

static void mul_f(QVariant& left, const QVariant& right, int lrtype) {

    static Demo::QQFunc funcs[] = {
        Demo::Mul<int, int>, Demo::Mul<int, Real>, Demo::Mul<int, Vector4>, Demo::Mul<int, Matrix4>, 0,
        Demo::Mul<Real, int>, Demo::Mul<Real, Real>, Demo::Mul<Real, Vector4>, Demo::Mul<Real, Matrix4>, 0,
        Demo::Mul<Vector4, int>, Demo::Mul<Vector4, Real>, 0, 0, 0,
        Demo::Mul<Matrix4, int>, Demo::Mul<Matrix4, Real>, Demo::Mul<Matrix4, Vector4>, Demo::Mul<Matrix4, Matrix4>, 0,
        0, 0, 0, 0, 0
    };

    funcs[lrtype](left, right);
}


static bool div_f(QVariant& left, const QVariant& right, int lrtype) {

    static Demo::BQQFunc funcs[] = {
        Demo::Div<int, int>, Demo::Div<int, Real>, 0, 0, 0,
        Demo::Div<Real, int>, Demo::Div<Real, Real>, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0
    };

    if (!funcs[lrtype](left, right)) return false;
    return true;
}

static bool eq_f(QVariant& left, const QVariant& right, int lrtype) {

    static Demo::BQQFunc funcs[] = {
        Demo::Eq<int, int>, Demo::Eq<int, Real>, 0, 0, 0,
        Demo::Eq<Real, int>, Demo::Eq<Real, Real>, 0, 0, 0,
        0, 0, Demo::Eq<Vector4, Vector4>, 0, 0,
        0, 0, 0, Demo::Eq<Matrix4, Matrix4>, 0,
        0, 0, 0, 0, Demo::Eq<QString, QString>
    };

    return funcs[lrtype](left, right);
}

static bool gt_f(QVariant& left, const QVariant& right, int lrtype) {

    static Demo::BQQFunc funcs[] = {
        Demo::Gt<int, int>, Demo::Gt<int, Real>, 0, 0, 0,
        Demo::Gt<Real, int>, Demo::Gt<Real, Real>, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0
    };

    return funcs[lrtype](left, right);
}

static bool lt_f(QVariant& left, const QVariant& right, int lrtype) {

    static Demo::BQQFunc funcs[] = {
        Demo::Lt<int, int>, Demo::Lt<int, Real>, 0, 0, 0,
        Demo::Lt<Real, int>, Demo::Lt<Real, Real>, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0
    };

    return funcs[lrtype](left, right);
}



const QVariant& Demo::Runner::evalCode(const CodeStack& code,  const ValueStack& immed) {

    const unsigned int* codes = code.data();

    int sPos = -1;
    int dPos = 0;
    bool jumpFlag = false;
    bool stopFlag = false;

    for (int ic = 0; ic < code.size(); ++ic) {

        int lrType = LRType(codes[ic]);

        switch (Code(codes[ic])) {

        case Parser::cImmed:
            mStack[++sPos] = immed[dPos++];
            break;

        case Parser::cNeg:
            neg_f(mStack[sPos], lrType);
            break;
        case Parser::cAdd:
            add_f(mStack[sPos-1], mStack[sPos], lrType);
            --sPos;
            break;
        case Parser::cSub:
            sub_f(mStack[sPos-1], mStack[sPos], lrType);
            --sPos;
            break;
        case Parser::cMul:
            mul_f(mStack[sPos-1], mStack[sPos], lrType);
            --sPos;
            break;
        case Parser::cDiv:
            if (!div_f(mStack[sPos-1], mStack[sPos], lrType)) {
                mEvalError = "Division by zero error";
                return mStack[sPos]; // just some value
            }
            --sPos;
            break;

        case Parser::cEqual:
            mStack[sPos-1].setValue(int(eq_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;
        case Parser::cNEqual:
            mStack[sPos-1].setValue(int(!eq_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;
        case Parser::cLess:
            mStack[sPos-1].setValue(int(lt_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;
        case Parser::cLessOrEq:
            mStack[sPos-1].setValue(int(!gt_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;
        case Parser::cGreater:
            mStack[sPos-1].setValue(int(gt_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;
        case Parser::cGreaterOrEq:
            mStack[sPos-1].setValue(int(!lt_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;

        case Parser::cBAnd:
            mStack[sPos-1].setValue(mStack[sPos-1].value<int>() & mStack[sPos].value<int>());
            --sPos;
            break;
        case Parser::cBOr:
            mStack[sPos-1].setValue(mStack[sPos-1].value<int>() | mStack[sPos].value<int>());
            --sPos;
            break;

        case Parser::cAnd:
            mStack[sPos-1].setValue(mStack[sPos-1].value<int>() && mStack[sPos].value<int>());
            --sPos;
            break;
        case Parser::cOr:
            mStack[sPos-1].setValue(mStack[sPos-1].value<int>() || mStack[sPos].value<int>());
            --sPos;
            break;
        case Parser::cNot:
            mStack[sPos].setValue(int(!mStack[sPos].value<int>()));
            break;

        case Parser::cFun:
            {
                Function* fun = mFunctions[(codes[++ic]) - Parser::FirstFunction];
                sPos -= fun->argTypes().size() - 1;
                mStack[sPos] = fun->execute(mStack, sPos);
            }
            break;

        case Parser::cVar:
            mStack[++sPos] = mVariables[(codes[++ic]) - Parser::FirstVariable]->value();
            break;

        case Parser::cTake:
            {
                int index = mStack[sPos].value<int>();
                if (index < 0 || index > 3) {
                    mEvalError = "Out of range error";
                    return mStack[sPos]; // just some value
                }
                take_f(mStack[sPos-1], index, lrType);
                --sPos;
            }
            break;

        case Parser::cGuard:
            {
                if (stopFlag) return mStack[sPos];

                jumpFlag = !mStack[sPos].value<int>();
                if (jumpFlag) {
                    dPos += codes[ic + 2];
                    ic += codes[ic + 1] + 2;
                } else {
                    stopFlag = true;
                    ic += 2;
                }
                --sPos;
            }
            break;



        default:
            Q_ASSERT(0);
        }
    }

    // we have jumped but not stopped
    if (jumpFlag && !stopFlag) {
        mEvalError = "No Value error";
        return mStack[0]; // sPos might be = -1
    }

    return mStack[sPos];
}

unsigned Demo::Runner::LRType(unsigned code) {
    return (code >> 12) & 0xff;
}

unsigned Demo::Runner::Code(unsigned code) {
    return code & 0xfff;
}



