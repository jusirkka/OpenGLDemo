#ifndef BEZIERPATCHER_H
#define BEZIERPATCHER_H

#include "patcher.h"

namespace Demo {
namespace WF {


class BezierPatcher: public Patcher
{
public:

    BezierPatcher();

    void setKnots(const QString& varname, const KnotVector& knots) override;
    void setBoundary(float u0, float u1, float v0, float v1) override;
    void gendata(uint offset) override;

protected:

    void extraReset() override;
    void setCP(const Vector4Vector& controlpoints) override;

private:

    using CoeffVector = QVector<int>;
    using PolyVector = QVector<CoeffVector>;

    PolyVector genPoly(uint deg);
    PolyVector genDPoly(const PolyVector& p);
    float pol(float x, const CoeffVector& a) const;
    float polu(uint k, float u) const;
    float polv(uint k, float v) const;
    float dpolu(uint k, float u) const;
    float dpolv(uint k, float v) const;
    Vector4 point(float u, float v) const;
    Vector4 normal(float u, float v) const;
    Vector4 texcoord(float u, float v) const;

    void genDegenerateData(uint offset);


private:

    float mUI, mUF, mVI, mVF;
    IndexVector mRotations;
    Vector4Vector mCps;
    uint mDiv;
    PolyVector mUPoly;
    PolyVector mVPoly;
    PolyVector mDUPoly;
    PolyVector mDVPoly;

};

}}



#endif // BEZIERPATCHER_H

