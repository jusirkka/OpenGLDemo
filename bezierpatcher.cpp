#include "bezierpatcher.h"

using namespace Demo::WF;

BezierPatcher::BezierPatcher()
    : mUI(0)
    , mUF(1)
    , mVI(0)
    , mVF(1)
    , mRotations()
    , mCps()
    , mDiv(10)
    , mUPoly()
    , mVPoly()
    , mDUPoly()
    , mDVPoly()
{}

void BezierPatcher::setKnots(const QString& varname, const KnotVector& knots) {
    // bezier surfaces have static knots at v, u = (0, 1), (0, 1)
    if (varname != "u" && varname != "v") {
        throw PatchError(R"(Variable name should be "u" or "v")");
    }
    if (knots.size() != 2 || !Math3D::eqz(knots[0]) || !Math3D::eqz(knots[1] - 1)) {
        throw PatchError(QString("Bezier surfaces have static knots at %1 = (0, 1)").arg(varname));
    }
}

void BezierPatcher::setBoundary(float u0, float u1, float v0, float v1) {

    if (u0 < 0 || u0 >= u1 || u1 > 1 || v0 < 0 || v0 >= v1 || v1 > 1) {
        throw PatchError("Bad boundary");
    }

    mUI = u0;
    mUF = u1;
    mVI = v0;
    mVF = v1;
}

static bool equalVector(const Demo::WF::BezierPatcher::Vector4Vector& v) {
    for (int i = 1; i < v.size(); ++i) {
        if (v[i] != v[0]) return false;
    }
    return true;
}

void BezierPatcher::setCP(const Vector4Vector& cps) {
    int targetSize = (mUdeg + 1) * (mVdeg + 1);
    if (cps.size() != targetSize) {
        throw PatchError(QString("Number of control points should be %1").arg((mUdeg + 1) * (mVdeg + 1)));
    }
    // test degeneracy
    Vector4Vector v;
    for (uint i = 0; i < mUdeg + 1; i++) {
        v << cps[i];
    }
    if (equalVector(v)) mRotations.append(0);
    v.clear();
    for (uint i = 0; i < mVdeg + 1; i++) {
        v << cps[i * (mUdeg + 1)];
    }
    if (equalVector(v)) mRotations.append(1);
    v.clear();
    for (uint i = 0; i < mUdeg + 1; i++) {
        v << cps[i + mVdeg * (mUdeg + 1)];
    }
    if (equalVector(v)) mRotations.append(2);
    v.clear();
    for (uint i = 0; i < mVdeg + 1; i++) {
        v << cps[mUdeg + i * (mUdeg + 1)];
    }
    if (equalVector(v)) mRotations.append(3);

    if (mRotations.size() > 1) {
        throw PatchError("At most singly degenerate bezier patches supported");
    }

    mCps = cps;

    mUPoly = genPoly(mUdeg);
    mVPoly = genPoly(mVdeg);
    mDUPoly = genDPoly(mUPoly);
    mDVPoly = genDPoly(mVPoly);
}

float BezierPatcher::pol(float x, const CoeffVector& a) const {
    float sum = 0;
    float prod = 1;
    for (int k = 0; k < a.size(); k++) {
        sum += a[k] * prod;
        prod *= x;
    }
    return sum;
}


float BezierPatcher::polu(uint k, float u) const {
    return pol(u, mUPoly[k]);
}

float BezierPatcher::polv(uint k, float v) const {
    return pol(v, mVPoly[k]);
}

float BezierPatcher::dpolu(uint k, float u) const {
    return pol(u, mDUPoly[k]);
}

float BezierPatcher::dpolv(uint k, float v) const {
    return pol(v, mDVPoly[k]);
}


Vector4 BezierPatcher::point(float u, float v) const {
    Vector4 res;
    for (uint n = 0; n <= mVdeg; ++n) for (uint m = 0; m <= mUdeg; ++m) {
        res += polu(m, u) * polv(n, v) * mCps[(mUdeg + 1) * n + m];
    }
    return res;
}

Vector4 BezierPatcher::texcoord(float u, float v) const {
    return Vector4(u, v, 0);
}

Vector4 BezierPatcher::normal(float u, float v) const {
    Vector4 du;
    for (uint n = 0; n <= mVdeg; ++n) for (uint m = 0; m <= mUdeg; ++m) {
        du += dpolu(m, u) * polv(n, v) * mCps[(mUdeg + 1) * n + m];
    }
    Vector4 dv;
    for (uint n = 0; n <= mVdeg; ++n) for (uint m = 0; m <= mUdeg; ++m) {
        dv += polu(m, u) * dpolv(n, v) * mCps[(mUdeg + 1) * n + m];
    }
    return Math3D::cross(du, dv).normalized3();
}



BezierPatcher::PolyVector BezierPatcher::genPoly(uint deg) {
    CoeffVector b10;
    b10 << 1 << -1;
    CoeffVector b11;
    b11 << 0 << 1;
    PolyVector b1;
    b1 << b10 << b11;
    PolyVector b2;
    uint n = 2;
    while (n <= deg) {
        b2.clear();
        // (n, 0)
        CoeffVector b = b1[0];
        b.append(0);
        for (int k = 1; k < b.size(); k++) {
            b[k] -= b1[0][k - 1];
        }
        b2.append(b);
        // (n, 1) .. (n, n-1)
        for (int k = 1; k < b1.size(); k++) {
            b = b1[k];
            b.append(0);
            for (int l = 1; l < b.size(); l++) {
                b[l] += b1[k-1][l-1] - b1[k][l-1];
            }
            b2.append(b);
        }
        // (n, n)
        b = b1.last();
        b.last() = 0;
        b.append(1);
        b2.append(b);
        b1 = b2;
        n++;
    }
    // qDebug() << b2;
    return b2;
}

BezierPatcher::PolyVector BezierPatcher::genDPoly(const PolyVector& p) {
    PolyVector dp;
    for (const CoeffVector& v: p) {
        CoeffVector dv;
        for (int k = 1; k < v.size(); k++) {
            dv.append(k * v[k]);
        }
        dp.append(dv);
    }
    return dp;
}

void BezierPatcher::gendata(uint offset) {

    if (!mRotations.isEmpty()) {
        genDegenerateData(offset);
        return;
    }

    uint s = mDiv + 1;
    for (uint n = 0; n < s; ++n) {
        IndexVector indices;
        float v = 1.0 * n / mDiv;
        for (uint m = 0; m < s; ++m) {
            float u = 1.0 * m / mDiv;
            mVertices.append(point(u, v));
            mNormals.append(normal(u, v));
            mTexes.append(texcoord(u, v));
            if (n == mDiv) continue;
            indices.append(offset + s*n + m);
            indices.append(offset + s*(n+1) + m);
            mWireframe.append(offset + s*n + m);
            mWireframe.append(offset + s*(n+1) + m);
            if (m == mDiv) continue;
            mWireframe.append(offset + s*n + m);
            mWireframe.append(offset + s*n + m+1);
        }
        mStrips.append(indices);
    }
    for (uint n = 0; n < mDiv; ++n) {
        mWireframe.append(offset + s*n + mDiv);
        mWireframe.append(offset + (s+1)*n + mDiv);
    }
    for (uint m = 0; m < mDiv; ++m) {
        mWireframe.append(offset + s*mDiv + m);
        mWireframe.append(offset + s*mDiv + m+1);
    }

}


void BezierPatcher::genDegenerateData(uint offset) {

    if (mRotations[0] != 0) {
        throw PatchError("Rotated singular points not yet supported");
    }

    // point 0
    float u = 0;
    float v = 0.0001;
    mVertices.append(point(u, v));
    mNormals.append(normal(u, v));
    mTexes.append(texcoord(u, v));

    uint m0 = 1;
    uint icount = 0;
    for (uint n = 0; n < 2 * mDiv + 1; ++n) {
        icount += m0;
        m0++;
        IndexVector indices;
        v = 1.0 * n / (2 * mDiv);
        for (uint m = 0; m < m0; ++m) {
            u = 1.0 * m / (m0 - 1);
            mVertices.append(point(u, v));
            mNormals.append(normal(u, v));
            mTexes.append(texcoord(u, v));
            if (m != m0 - 1) {
                indices.append(offset + icount - m0 + 1 + m);
            }
            indices.append(offset + icount + m);
            if (m == m0 - 1) continue;
            mWireframe.append(offset + icount + m);
            mWireframe.append(offset + icount - m0 + 1 + m);
            mWireframe.append(offset + icount + m);
            mWireframe.append(offset + icount + m + 1);
            mWireframe.append(offset + icount + m + 1);
            mWireframe.append(offset + icount - m0 + 1 + m);
        }
        mStrips.append(indices);
    }

}


void BezierPatcher::extraReset() {
    mUI = 0;
    mUF = 1;
    mVI = 0;
    mVF = 1;
    mRotations.clear();
    mCps.clear();
}


