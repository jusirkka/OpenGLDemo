#ifndef PATCHER_H
#define PATCHER_H

#include <QString>
#include <QVector>

#include "math3d.h"

using Math3D::Vector4;

namespace Demo {namespace WF {

class PatchError {

public:
    PatchError(QString msg)
        : emsg(std::move(msg)) {}

    PatchError()
        :emsg() {}

    const QString msg() const {return emsg;}

private:

    QString emsg;

};

class Patcher
{
public:

    using KnotVector = QVector<float>;
    using Vector4Vector = QVector<Vector4>;
    using IndexVector = QVector<uint>;
    using StripVector = QVector<IndexVector>;

    static Patcher* Create(const QString& name);

    void reset();
    void setControlPoints(uint udeg, uint vdeg, const Vector4Vector& controlpoints);

    virtual void setKnots(const QString& varname, const KnotVector& knots) = 0;
    virtual void setBoundary(float u0, float u1, float v0, float v1) = 0;
    virtual void gendata(uint offset) = 0;

    const Vector4Vector& vertices() const {return mVertices;}
    const Vector4Vector& normals() const {return mNormals;}
    const Vector4Vector& texes() const {return mTexes;}
    const IndexVector& wireframe() const {return mWireframe;}
    const StripVector& strips() const {return mStrips;}


    virtual ~Patcher() = default;

protected:

    virtual void extraReset() = 0;
    virtual void setCP(const Vector4Vector& controlpoints) = 0;

    Patcher()
        : mVertices()
        , mNormals()
        , mTexes()
        , mWireframe()
        , mStrips()
        , mUdeg(0)
        , mVdeg(0) {}

protected:

    Vector4Vector mVertices;
    Vector4Vector mNormals;
    Vector4Vector mTexes;
    IndexVector mWireframe;
    StripVector mStrips;
    uint mUdeg;
    uint mVdeg;

};

}}

#endif // PATCHER_H
