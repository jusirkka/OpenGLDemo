#ifndef OPERATION_H
#define OPERATION_H

#include "type.h"
#include "math3d.h"

#define NULLP_MSG QStringLiteral("%1: Null pointer(s)")
#define NUM_TYPE_MSG QStringLiteral("%1: Expected numeric type")
#define INCOMPATIBLE_MSG QStringLiteral("%1: Incompatible type(s)")
#define INTEGER_MSG QStringLiteral("%1: Expected integer type")
#define BASE_MSG QStringLiteral("%1: Expected base type")
#define NO_MEMBER_MSG QStringLiteral("%1: no such member")


namespace Demo {

using Real_T = BaseType<Math3D::Real>;
using Integer_T = BaseType<Math3D::Integer>;
using Vector_T = BaseType<Math3D::Vector4>;
using Matrix_T = BaseType<Math3D::Matrix4>;
using Text_T = BaseType<QString>;


class OpError {
public:
    OpError(QString msg)
        : mDetail(std::move(msg))
    {}

    const QString msg() const {return mDetail;}

private:
    QString mDetail;
};

class Operation {
public:
    Operation(QString name, int code)
        : mName(std::move(name))
        , mCode(code)
    {}

    virtual void check(const Type* left, const Type* right = nullptr) const = 0;
    virtual const Type* type(const Type* left, const Type* right = nullptr) const = 0;

    const QString name() const {return mName;}
    int code() const {return mCode;}

    void checkBase(const Type* left, const Type* right, bool unary = false) const {
        if (unary) {
            if (left == nullptr || right != nullptr) throw OpError(NULLP_MSG);
        } else {
            if (left == nullptr || right == nullptr) throw OpError(NULLP_MSG);
        }
        if (dynamic_cast<const ComboundType*>(left)) throw OpError(BASE_MSG);
        if (dynamic_cast<const ComboundType*>(right)) throw OpError(BASE_MSG);
    }

    virtual ~Operation() = default;

protected:

    QString mName;
    int mCode;

};

class BoolOp: public Operation {
public:

    BoolOp(QString name, int code)
        : Operation(name, code)
        , mType(new Integer_T)
    {}

    const Type* type(const Type*, const Type*) const override {
        return mType;
    }

    ~BoolOp() {delete mType;}


protected:

    Type* mType;

};

class RelOp: public BoolOp {
public:

    RelOp(QString name, int code)
        : BoolOp(name, code)
    {}

    void check(const Type* left, const Type* right = nullptr) const override {
        checkBase(left, right);
        int lid = left->id();
        if (lid != Type::Integer && lid != Type::Real) throw OpError(NUM_TYPE_MSG);
        int rid = right->id();
        if (rid != Type::Integer && rid != Type::Real) throw OpError(NUM_TYPE_MSG);
    }
};


class EqOp: public BoolOp {
public:

    EqOp(QString name, int code)
      : BoolOp(name, code)
    {}

    void check(const Type* left, const Type* right) const override {
        checkBase(left, right);
        int lid = left->id();
        int rid = right->id();
        if (lid == rid) return;
        if (lid == Type::Integer && rid == Type::Real) return;
        if (lid == Type::Real && rid == Type::Integer) return;
        throw OpError(INCOMPATIBLE_MSG);
    }
};


class AndOrOp: public BoolOp {
public:
    AndOrOp(QString name, int code)
      : BoolOp(name, code)
    {}

    void check(const Type* left, const Type* right = nullptr) const override {
        checkBase(left, right);
        if (left->id() != Type::Integer) throw OpError(INTEGER_MSG);
        if (right->id() != Type::Integer) throw OpError(INTEGER_MSG);
    }
};


class AddOp: public Operation {
public:
    AddOp(QString name, int code)
      : Operation(name, code)
    {}

    void check(const Type* left, const Type* right = nullptr) const override {
        checkBase(left, right);
        int lid = left->id();
        int rid = right->id();
        if (lid == Type::Text && rid == Type::Text && name() != "+") throw OpError(INCOMPATIBLE_MSG);
        if (lid == rid) return;
        if (lid == Type::Integer && rid == Type::Real) return;
        if (lid == Type::Real && rid == Type::Integer) return;
        throw OpError(INCOMPATIBLE_MSG);
    }

    const Type* type(const Type* left, const Type* right) const override {
        if (right->id() == Type::Real) return right;
        return left;
    }

};


class MulOp: public Operation {
public:
    MulOp(QString name, int code)
      : Operation(name, code)
    {}

    void check(const Type* left, const Type* right = nullptr) const override {
        checkBase(left, right);
        int lid = left->id();
        int rid = right->id();
        if (lid == Type::Text || rid == Type::Text) throw OpError(INCOMPATIBLE_MSG);
        if (lid == Type::Vector && rid == Type::Vector) throw OpError(INCOMPATIBLE_MSG);
        if (lid == Type::Vector && rid == Type::Matrix) throw OpError(INCOMPATIBLE_MSG);
    }

    const Type* type(const Type* left, const Type* right) const override {
        int lid = left->id();
        int rid = right->id();
        if (rid == Type::Vector) return right;
        if (rid == Type::Matrix) return right;
        if (lid == Type::Integer && rid == Type::Real) return right;
        return left;
    }

};

class DivOp: public Operation {
public:
    DivOp(QString name, int code)
      : Operation(name, code)
    {}

    void check(const Type* left, const Type* right = nullptr) const override {
        checkBase(left, right);
        int lid = left->id();
        int rid = right->id();
        if (lid == Type::Text || rid == Type::Text) throw OpError(INCOMPATIBLE_MSG);
        if (lid == Type::Vector || rid == Type::Vector) throw OpError(INCOMPATIBLE_MSG);
        if (lid == Type::Matrix || rid == Type::Matrix) throw OpError(INCOMPATIBLE_MSG);
    }

    const Type* type(const Type* left, const Type* right) const override {
        if (right->id() == Type::Real) return right;
        return left;
    }

};

class SignOp: public Operation {
public:
    SignOp(QString name, int code)
      : Operation(name, code)
    {}

    void check(const Type* left, const Type* right = nullptr) const override {
        checkBase(left, right, true);
        if (left->id() == Type::Text) throw OpError(INCOMPATIBLE_MSG);
    }

    const Type* type(const Type* left, const Type*) const override {
        return left;
    }

};

class NegOp: public Operation {
public:
    NegOp(QString name, int code)
      : Operation(name, code)
    {}

    void check(const Type* left, const Type* right) const override {
        checkBase(left, right, true);
        if (left->id() != Type::Integer) throw OpError(INTEGER_MSG);
    }

    const Type* type(const Type* left, const Type*) const override {
        return left;
    }

};


class TakeOp: public Operation {
public:
    TakeOp(QString name, int code)
      : Operation(name, code)
      , mVectorT(new Vector_T)
      , mRealT(new Real_T)
    {}

    void check(const Type* left, const Type* right) const override {
        if (left == nullptr || right == nullptr) throw OpError(NULLP_MSG);
        if (right->id() != Type::Integer) throw OpError(INTEGER_MSG);
        if (dynamic_cast<const ArrayType*>(left)) return;
        int lid = left->id();
        if (lid == Type::Matrix) return;
        if (lid == Type::Vector) return;
        throw OpError(INCOMPATIBLE_MSG);
    }

    const Type* type(const Type* left, const Type*) const override {
        auto arr = dynamic_cast<const ArrayType*>(left);
        if (arr) return arr->subtypes().first();
        if (left->id() == Type::Matrix) return mVectorT;
        return mRealT;
    }

    ~TakeOp() {delete mVectorT; delete mRealT;}

private:

    Type* mVectorT;
    Type* mRealT;
};

class MemberOp: public Operation {
public:
    MemberOp(QString name, int code)
      : Operation(name, code)
    {}

    void check(const Type* left, const Type* right) const override {
        if (left == nullptr || right == nullptr) throw OpError(NULLP_MSG);
        auto rec = dynamic_cast<const RecordType*>(left);
        auto sel = dynamic_cast<const Selector*>(right);
        if (!rec || !sel) throw OpError(INCOMPATIBLE_MSG);
        if (!rec->type(sel->name())) throw OpError(NO_MEMBER_MSG);
    }

    const Type* type(const Type* left, const Type* right) const override {
        auto rec = dynamic_cast<const RecordType*>(left);
        auto sel = dynamic_cast<const Selector*>(right);
        return rec->type(sel->name());
    }

};

}

#endif // OPERATION_H
