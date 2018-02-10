#ifndef VALUE_H
#define VALUE_H

#include <QVariant>
#include <QVector>
#include "math3d.h"

using Math3D::Vector4;
using Math3D::Matrix4;

namespace Demo {

class Type;

class ValueError {
public:
    ValueError(QString msg): mDetail(std::move(msg)) {}
    const QString msg() const {return mDetail;}
private:
    QString mDetail;
};

class Value {
public:
    using Path = QVector<int>;
    using ChildList = QVector<Value*>;

    virtual void set(const QVariant& v, Path p) = 0;
    virtual QVariant get(Path p) const = 0;
    virtual Value* clone() const = 0;
    virtual Value* tmpl() const {return clone();}
    virtual ~Value() = default;

    static Value* Create(const Type* t);
};

class ListValue: public Value {
public:

    ChildList kids;
    ListValue() = default;
    ListValue(const ListValue& node) {
        for (auto v: node.kids) {
            kids << v->clone();
        }
    }

    void set(const QVariant& v, Path p) override;
    QVariant get(Path p) const override;
    ListValue* clone() const override {return new ListValue(*this);}
    ListValue* tmpl() const override;

    ~ListValue() {qDeleteAll(kids);}

};

class RecValue: public Value {
public:

    ChildList kids;

    RecValue() = default;
    RecValue(const RecValue& node) {
        for (auto v: node.kids) {
            kids << v->clone();
        }
    }

    void set(const QVariant& v, Path p) override;
    QVariant get(Path p) const override;
    RecValue* clone() const override {return new RecValue(*this);}

    ~RecValue() {qDeleteAll(kids);}

};

class LeafValue: public Value {
public:
    QVariant variant;

    LeafValue() {variant = QVariant::fromValue(0);}
    LeafValue(const LeafValue& v) {variant = v.variant;}

    void set(const QVariant& v, Path p) override {
        if (!p.isEmpty()) throw ValueError("Non-empty path when setting leaf value");
        variant = v;
    }
    QVariant get(Path p) const override {
        if (!p.isEmpty()) throw ValueError("Non-empty path when getting leaf value");
        return variant;
    }

    LeafValue* clone() const override {return new LeafValue(*this);}
};

class VectorValue: public Value {
public:
    Vector4 vec;

    VectorValue() = default;
    VectorValue(const VectorValue& v) {vec = v.vec;}

    void set(const QVariant& v, Path p) override;
    QVariant get(Path p) const override;

    VectorValue* clone() const override {return new VectorValue(*this);}
};

class MatrixValue: public Value {
public:
    Matrix4 mat;

    MatrixValue() {mat.setIdentity();}
    MatrixValue(const MatrixValue& m) {mat = m.mat;}

    void set(const QVariant& v, Path p) override;
    QVariant get(Path p) const override;

    MatrixValue* clone() const override {return new MatrixValue(*this);}
};

}
#endif // VALUE_H
