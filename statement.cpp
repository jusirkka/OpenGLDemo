#include "statement.h"
#include "math3d.h"
#include "gl_lang_compiler.h"
#include "scope.h"

using Math3D::Matrix4;
using Math3D::Vector4;
using Math3D::Real;
using Demo::GL::Compiler;

using namespace Demo::Statement;

Statement::Statement(CodeStack code, ValueStack immed, unsigned stackSize, int pos)
    : mCode(std::move(code))
    , mImmed(std::move(immed))
    , mStack(stackSize)
    , mPos(pos)
{}

Statement::Statement(int pos)
    : mCode()
    , mImmed()
    , mStack()
    , mPos(pos)
{}

static void neg_f(QVariant& right, int lrtype) {

    static QFunc funcs[] = {
        Neg<int>, Neg<Real>, Neg<Vector4>, Neg<Matrix4>, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr
    };

    funcs[lrtype](right);
}


static void take_f(QVariant& left, int index, int lrtype) {

    static QIFunc funcs[] = {
        nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr,
        Take<Vector4>, nullptr, nullptr, nullptr, nullptr,
        Vec<Matrix4>, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr
    };

    funcs[lrtype](left, index);
}

static void add_f(QVariant& left, const QVariant& right, int lrtype) {

    static QQFunc funcs[] = {
        Add<int, int>, Add<int, Real>, nullptr, nullptr, nullptr,
        Add<Real, int>, Add<Real, Real>, nullptr, nullptr, nullptr,
        nullptr, nullptr, Add<Vector4, Vector4>, nullptr, nullptr,
        nullptr, nullptr, nullptr, Add<Matrix4, Matrix4>, nullptr,
        nullptr, nullptr, nullptr, nullptr, Add<QString, QString>
    };

    funcs[lrtype](left, right);
}

static void sub_f(QVariant& left, const QVariant& right, int lrtype) {

    static QQFunc funcs[] = {
        Sub<int, int>, Sub<int, Real>, nullptr, nullptr, nullptr,
        Sub<Real, int>, Sub<Real, Real>, nullptr, nullptr, nullptr,
        nullptr, nullptr, Sub<Vector4, Vector4>, nullptr, nullptr,
        nullptr, nullptr, nullptr, Sub<Matrix4, Matrix4>, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr
    };

    funcs[lrtype](left, right);
}

static void mul_f(QVariant& left, const QVariant& right, int lrtype) {

    static QQFunc funcs[] = {
        Mul<int, int>, Mul<int, Real>, Mul<int, Vector4>, Mul<int, Matrix4>, nullptr,
        Mul<Real, int>, Mul<Real, Real>, Mul<Real, Vector4>, Mul<Real, Matrix4>, nullptr,
        Mul<Vector4, int>, Mul<Vector4, Real>, nullptr, nullptr, nullptr,
        Mul<Matrix4, int>, Mul<Matrix4, Real>, Mul<Matrix4, Vector4>, Mul<Matrix4, Matrix4>, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr
    };

    funcs[lrtype](left, right);
}


static bool div_f(QVariant& left, const QVariant& right, int lrtype) {

    static BQQFunc funcs[] = {
        Div<int, int>, Div<int, Real>, nullptr, nullptr, nullptr,
        Div<Real, int>, Div<Real, Real>, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr
    };

    if (!funcs[lrtype](left, right)) return false;
    return true;
}

static bool eq_f(QVariant& left, const QVariant& right, int lrtype) {

    static BQQFunc funcs[] = {
        Eq<int, int>, Eq<int, Real>, nullptr, nullptr, nullptr,
        Eq<Real, int>, Eq<Real, Real>, nullptr, nullptr, nullptr,
        nullptr, nullptr, Eq<Vector4, Vector4>, nullptr, nullptr,
        nullptr, nullptr, nullptr, Eq<Matrix4, Matrix4>, nullptr,
        nullptr, nullptr, nullptr, nullptr, Eq<QString, QString>
    };

    return funcs[lrtype](left, right);
}

static bool gt_f(QVariant& left, const QVariant& right, int lrtype) {

    static BQQFunc funcs[] = {
        Gt<int, int>, Gt<int, Real>, nullptr, nullptr, nullptr,
        Gt<Real, int>, Gt<Real, Real>, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr
    };

    return funcs[lrtype](left, right);
}

static bool lt_f(QVariant& left, const QVariant& right, int lrtype) {

    static BQQFunc funcs[] = {
        Lt<int, int>, Lt<int, Real>, nullptr, nullptr, nullptr,
        Lt<Real, int>, Lt<Real, Real>, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr
    };

    return funcs[lrtype](left, right);
}


static unsigned LRType(unsigned code) {
    return (code >> 12) & 0xff;
}

static unsigned Code(unsigned code) {
    return code & 0xfff;
}


const QVariant& Statement::evalCode(const VariableIndexMap& vars, const FunctionVector& funcs) {

    const unsigned int* codes = mCode.data();

    int sPos = -1;
    int dPos = 0;
    bool jumpFlag = false;
    bool stopFlag = false;

    for (int ic = 0; ic < mCode.size(); ++ic) {

        int lrType = LRType(codes[ic]);

        switch (Code(codes[ic])) {

        case Compiler::cImmed:
            mStack[++sPos] = mImmed[dPos++];
            break;

        case Compiler::cNeg:
            neg_f(mStack[sPos], lrType);
            break;
        case Compiler::cAdd:
            add_f(mStack[sPos-1], mStack[sPos], lrType);
            --sPos;
            break;
        case Compiler::cSub:
            sub_f(mStack[sPos-1], mStack[sPos], lrType);
            --sPos;
            break;
        case Compiler::cMul:
            mul_f(mStack[sPos-1], mStack[sPos], lrType);
            --sPos;
            break;
        case Compiler::cDiv:
            if (!div_f(mStack[sPos-1], mStack[sPos], lrType)) {
                throw RunError("Division by zero error", mPos);
            }
            --sPos;
            break;

        case Compiler::cEqual:
            mStack[sPos-1].setValue(int(eq_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;
        case Compiler::cNEqual:
            mStack[sPos-1].setValue(int(!eq_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;
        case Compiler::cLess:
            mStack[sPos-1].setValue(int(lt_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;
        case Compiler::cLessOrEq:
            mStack[sPos-1].setValue(int(!gt_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;
        case Compiler::cGreater:
            mStack[sPos-1].setValue(int(gt_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;
        case Compiler::cGreaterOrEq:
            mStack[sPos-1].setValue(int(!lt_f(mStack[sPos-1], mStack[sPos], lrType)));
            --sPos;
            break;

        case Compiler::cBAnd:
            mStack[sPos-1].setValue(mStack[sPos-1].value<int>() & mStack[sPos].value<int>());
            --sPos;
            break;
        case Compiler::cBOr:
            mStack[sPos-1].setValue(mStack[sPos-1].value<int>() | mStack[sPos].value<int>());
            --sPos;
            break;

        case Compiler::cAnd:
            mStack[sPos-1].setValue(mStack[sPos-1].value<int>() && mStack[sPos].value<int>());
            --sPos;
            break;
        case Compiler::cOr:
            mStack[sPos-1].setValue(mStack[sPos-1].value<int>() || mStack[sPos].value<int>());
            --sPos;
            break;
        case Compiler::cNot:
            mStack[sPos].setValue(int(!mStack[sPos].value<int>()));
            break;

        case Compiler::cFun:
        {
            Function* fun = funcs[codes[++ic] - Scope::FunctionOffset];
            sPos -= fun->argTypes().size() - 1;
            mStack[sPos] = fun->execute(mStack, sPos);
        }
            break;

        case Compiler::cVar:
            mStack[++sPos] = vars[codes[++ic]]->value();
            break;

        case Compiler::cTake:
        {
            int index = mStack[sPos].value<int>();
            if (index < 0 || index > 3) {
                throw RunError("Out of range error", mPos);
            }
            take_f(mStack[sPos-1], index, lrType);
            --sPos;
        }
            break;

        case Compiler::cGuard:
        {
            if (stopFlag) return mStack[sPos - 1];

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
            Q_ASSERT(false);
        }
    }

    // we have jumped but not stopped
    if (jumpFlag && !stopFlag) {
        throw RunError("No Value error", mPos);
    }

    return mStack[sPos];
}


int Assignment::exec_and_jump(VariableIndexMap& vars, const FunctionVector& funcs) {
    Variable* v = vars[mVarIndex];
    v->setValue(evalCode(vars, funcs));
    return 1; // jump to the next statement
    // qDebug() << v->name() << "=" << v->value();
}


int CondJump::exec_and_jump(VariableIndexMap& vars, const FunctionVector& funcs) {
    QVariant r = evalCode(vars, funcs);
    if (r.toBool()) return 1;
    return mJump;
}

