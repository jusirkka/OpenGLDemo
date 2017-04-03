#ifndef MATH3D_H
#define MATH3D_H

#include <cmath>
#include <QMetaType>
#include <QDebug>
#include <GL/gl.h>

namespace Math3D {

#define OSTREAM_MATH3D

typedef GLfloat Real;
typedef int Integer;

// --------------------------------------------------------------
// Constants
// --------------------------------------------------------------

const Real DEGS_PER_RAD = 57.29577951308232087680;
const Real RADS_PER_DEG =  0.01745329251994329577;
const Real TWO_PI =        6.28318530717958647693;
const Real PI =            3.14159265358979323846;
const Real EPSILON =       1.0e-8;
const Real EPSILON2 =      1.0e-16;

enum {X = 0, Y = 1, Z = 2, W = 3};

// --------------------------------------------------------------
// Utility Functions
// --------------------------------------------------------------

inline bool eqz(Real x) {return std::fabs(x) <= EPSILON;}
inline Real rads(Real x) {return x * RADS_PER_DEG;}
inline Real degs(Real x) {return x * DEGS_PER_RAD;}


// --------------------------------------------------------------
// Classes: Vector4
// --------------------------------------------------------------


class Vector4 {

public:

    Vector4 (Real x = 0, Real y = 0, Real z = 0, Real w = 1) {e[X] = x; e[Y] = y; e[Z] = z; e[W] = w;}
    explicit Vector4(const Real* vec, int len = 3) {for (int i = 0; i < len; ++i) e[i] = vec[i]; if (len == 3) e[W] = 1;}

    const Real& operator[] (int i) const {return e[i];}
    Real& operator() (int i) {return e[i];}

    inline Vector4& operator+= (const Vector4& v); // (1)
    inline Vector4& operator-= (const Vector4& v); // (2)
    inline Vector4& operator*= (Real s); // (3)

    const Real* readArray() const {return e;}
    Real* getArray() {return e;}

    inline Real length23() const; // (4)
    inline Real length3() const; // (5)

    inline void normalize3(); // (6)
    inline Vector4 normalized3() const; // (7)

    inline bool isZero() const; // (8)

private:

    Real e[4];

};


inline Real dot3(const Vector4& v1, const Vector4& v2); // (9)
inline Vector4 cross(const Vector4& v1, const Vector4& v2); // (10)


inline Vector4 operator- (const Vector4&); // (11)
inline Vector4 operator* (const Vector4&, Real); // (12)
inline Vector4 operator* (Real, const Vector4&); // (13)

inline Vector4 operator+ (const Vector4&, const Vector4&); // (14)
inline Vector4 operator- (const Vector4&, const Vector4&); // (15)

inline bool operator== (const Vector4&, const Vector4&); // (16)
inline bool operator!= (const Vector4&, const Vector4&); // (16b)


// --------------------------------------------------------------
// Classes: Matrix4
// --------------------------------------------------------------

class Matrix4 {

public:

    Matrix4 () {}

    // Note: indices are specified column first: M(x)[y] or M[x][y] means M(y, x)
    Matrix4(Real* vec) {
        int i = 0;
        for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y) (*this)(x)[y] = vec[i++];
    }

    const Real* operator[] (int i) const {return &e[i<<2];}
    Real* operator() (int i) {return &e[i<<2];}

    inline Matrix4& operator+= (const Matrix4& m); // (1)
    inline Matrix4& operator-= (const Matrix4& m); // (2)
    inline Matrix4& operator*= (const Matrix4& m); // (3)
    inline Matrix4& operator*= (Real s); // (4)

    const Real* readArray() const {return e;}
    Real* getArray() {return e;}

    inline Matrix4 transpose3() const; // (5)
    inline Matrix4 transpose4() const; // (6)
    inline Vector4 translation() const; // (7)
    inline Matrix4 linear() const; // (8)
    inline Matrix4 comatrix() const; // (8b)

    inline Matrix4& doTranspose3(); // (22)
    inline Matrix4& doTranspose4(); // (23)

    inline Matrix4& setIdentity(); // (9)
    inline Matrix4& setRotation(Real angle, Vector4 axis); // (10)
    inline Matrix4& setRotation(Real yaw, Real pitch, Real roll); // (24)
    inline Matrix4& setTranslation(const Vector4& translation); // (11)
    inline Matrix4& setScaling(Real x, Real y, Real z); // (12)
    inline Matrix4& setBasis(const Vector4& v, const Vector4& n); // (13)
    inline Matrix4& setBasis(const Vector4& u, const Vector4& v, const Vector4& n); // (14)

    inline Vector4 row3(int i) const; // (14b)

private:

    Real e[16];


};

inline Vector4 operator* (const Matrix4&, const Vector4&); // (15)
inline Matrix4 operator* (const Matrix4&, const Matrix4&); // (16)
inline Matrix4 operator* (Real, const Matrix4&); // (17)
inline Matrix4 operator* (const Matrix4&, Real); // (18)
inline Matrix4 operator+ (const Matrix4&, const Matrix4&); // (19)
inline Matrix4 operator- (const Matrix4&, const Matrix4&); // (20)
inline Matrix4 operator- (const Matrix4&); // (21)

inline bool operator== (const Matrix4&, const Matrix4&); // (25)


// --------------------------------------------------------------
// Vector4 Implementation
// --------------------------------------------------------------


// ----------------- (1) ----------------------------------------
inline Vector4& Vector4::operator+= (const Vector4& v) {
    e[X] += v[X]; e[Y] += v[Y]; e[Z] += v[Z];
    return *this;
}

// ----------------- (2) ----------------------------------------
inline Vector4& Vector4::operator-= (const Vector4& v) {
    e[X] -= v[X]; e[Y] -= v[Y]; e[Z] -= v[Z];
    return *this;
}

// ----------------- (3) ----------------------------------------
inline Vector4& Vector4::operator*= (Real s) {
    e[X] *= s; e[Y] *= s; e[Z] *= s;
    return *this;
}

// ----------------- (4) ----------------------------------------
inline Real Vector4::length23() const {
    return dot3(*this, *this);
}

// ----------------- (5) ----------------------------------------
inline Real Vector4::length3() const {
    return std::sqrt(length23());
}

// ----------------- (6) ----------------------------------------
inline void Vector4::normalize3() {
    *this *= 1 / length3();
}


// ----------------- (7) ----------------------------------------
inline Vector4 Vector4::normalized3() const {
    return *this * (1 / length3());
}

// ----------------- (8) ----------------------------------------
inline bool Vector4::isZero() const {
    return length23() < EPSILON2;
}

// ----------------- (9) ----------------------------------------
inline Real dot3(const Vector4& v1, const Vector4& v2) {
    return v1[X] * v2[X] + v1[Y] * v2[Y] + v1[Z] * v2[Z];
}

// ---------------- (10) ----------------------------------------
inline Vector4 cross(const Vector4& v1, const Vector4& v2) {
  return Vector4(v1[Y] * v2[Z] - v1[Z] * v2[Y],
                 v1[Z] * v2[X] - v1[X] * v2[Z],
                 v1[X] * v2[Y] - v1[Y] * v2[X]);
}

// ---------------- (11) ----------------------------------------
inline Vector4 operator- (const Vector4& v) {
  return Vector4(-v[X], -v[Y], -v[Z]);
}

// ---------------- (12) ----------------------------------------
inline Vector4 operator* (const Vector4& v, Real s) {
  return Vector4(v[X] * s, v[Y] * s, v[Z] * s);
}

// ---------------- (13) ----------------------------------------
inline Vector4 operator* (Real s, const Vector4& v) {
    return v * s;
}

// ---------------- (14) ----------------------------------------
inline Vector4 operator+ (const Vector4& v1, const Vector4& v2) {
  return Vector4(v1[X] + v2[X], v1[Y] + v2[Y], v1[Z] + v2[Z]);
}

// ---------------- (15) ----------------------------------------
inline Vector4 operator- (const Vector4& v1, const Vector4& v2) {
  return Vector4(v1[X] - v2[X], v1[Y] - v2[Y], v1[Z] - v2[Z]);
}

// ---------------- (16) ----------------------------------------
inline bool operator== (const Vector4& v1, const Vector4& v2) {
    Vector4 v = v1 - v2;
    return v.isZero();
}

// ---------------- (16b) ----------------------------------------
inline bool operator!= (const Vector4& v1, const Vector4& v2) {
    return !(v1 == v2);
}

// --------------------------------------------------------------
// Matrix4 implementation
// --------------------------------------------------------------


// ----------------- (1) ----------------------------------------
inline Matrix4& Matrix4::operator+= (const Matrix4& m) {
    Real* self_v = getArray();
    const Real* v = m.readArray();
    // do not change e[15]
    for (int i = 0; i < 15; ++i) self_v[i] += v[i];
    return *this;
}

// ----------------- (2) ----------------------------------------
inline Matrix4& Matrix4::operator-= (const Matrix4& m) {
    Real* self_v = getArray();
    const Real* v = m.readArray();
    // do not change e[15]
    for (int i = 0; i < 15; ++i) self_v[i] -= v[i];
    return *this;
}

// ----------------- (3) ----------------------------------------
inline Matrix4& Matrix4::operator*= (const Matrix4& m1) {
    Matrix4 m;
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y) {
            m(x)[y] = 0;
            for (int i = 0; i < 4; ++i) m(x)[y] += m1[x][i] * (*this)[i][y];
    }
    Real* self_v = getArray();
    const Real* v = m.readArray();
    for (int i = 0; i < 16; ++i) self_v[i] = v[i];
    return *this;
}

// ----------------- (4) ----------------------------------------
inline Matrix4& Matrix4::operator*= (Real s) {
    Real* self_v = getArray();
    // do not change e[15]
    for (int i = 0; i < 15; ++i) self_v[i] *= s;
    return *this;
}

// ----------------- (5) ----------------------------------------
inline Matrix4 Matrix4::transpose3() const {
    Matrix4 m;
    m.setIdentity();
    for (int x = 0; x < 3; ++x) for (int y = 0; y < 3; ++y) m(x)[y] = (*this)[y][x];
    return m;
}

// ----------------- (6) ----------------------------------------
inline Matrix4 Matrix4::transpose4() const {
    Matrix4 m;
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y) m(x)[y] = (*this)[y][x];
    return m;
}

// ----------------- (7) ----------------------------------------
inline Vector4 Matrix4::translation() const {
    return Vector4((*this)[3]);
}

// ----------------- (8) ----------------------------------------
inline Matrix4 Matrix4::linear() const {
    Matrix4 m;
    m.setIdentity();
    for (int x = 0; x < 3; ++x) for (int y = 0; y < 3; ++y) m(x)[y] = (*this)[x][y];
    return m;
}

// ----------------- (8b) ----------------------------------------
inline Matrix4 Matrix4::comatrix() const {
    Matrix4 m;
    m.setIdentity();
    for (int x = 0; x < 3; ++x) {
        int u_p = (x + 1) % 3;
        int u_m = (x - 1 + 3) % 3;
        for (int y = 0; y < 3; ++y) {
            int v_p = (y + 1) % 3;
            int v_m = (y - 1 + 3) % 3;
            m(x)[y] = (*this)[u_p][v_p] * (*this)[u_m][v_m] - (*this)[u_p][v_m] * (*this)[u_m][v_p];
        }
    }
    return m;
}

// ----------------- (9) ----------------------------------------
inline Matrix4& Matrix4::setIdentity() {
    for(int i = 0; i < 16; ++i) e[i] = 0;
    e[0] = e[5] = e[10] = e[15] = 1;
    return *this;
}

// ---------------- (10) ----------------------------------------
inline Matrix4& Matrix4::setRotation(Real angle, Vector4 axis) {

    Real c = std::cos(angle);
    Real s = std::sin(angle);

    // One minus c
    Real omc = (1 - c);

    axis.normalize3();

    Real x = axis[X];
    Real y = axis[Y];
    Real z = axis[Z];
    Real xs = x * s;
    Real ys = y * s;
    Real zs = z * s;
    Real xyomc = x * y * omc;
    Real xzomc = x * z * omc;
    Real yzomc = y * z * omc;

    e[0] = x*x*omc + c;
    e[1] = xyomc + zs;
    e[2] = xzomc - ys;
    e[3] = 0;

    e[4] = xyomc - zs;
    e[5] = y*y*omc + c;
    e[6] = yzomc + xs;
    e[7] = 0;

    e[8] = xzomc + ys;
    e[9] = yzomc - xs;
    e[10] = z*z*omc + c;
    e[11] = 0;

    e[12] = 0;
    e[13] = 0;
    e[14] = 0;
    e[15] = 1;
    return *this;
}

// ---------------- (11) ----------------------------------------
inline Matrix4& Matrix4::setTranslation(const Vector4& translation) {
    setIdentity();
    e[12] = translation[X];
    e[13] = translation[Y];
    e[14] = translation[Z];
    return *this;
}

// ---------------- (12) ----------------------------------------
inline Matrix4& Matrix4::setScaling(Real x, Real y, Real z) {
    setIdentity();
    e[0] = x;
    e[5] = y;
    e[10] = z;
    return *this;
}

// ---------------- (13) ----------------------------------------
inline Matrix4& Matrix4::setBasis(const Vector4& v, const Vector4& n) {
    Vector4 u = cross(v,n);
    setBasis(u, v, n);
    return *this;
}

// ---------------- (14) ----------------------------------------
inline Matrix4& Matrix4::setBasis(const Vector4& u, const Vector4& v, const Vector4& n) {
    e[0] = u[X];
    e[1] = v[X];
    e[2] = n[X];
    e[3] = 0;

    e[4] = u[Y];
    e[5] = v[Y];
    e[6] = n[Y];
    e[7] = 0;

    e[8] = u[Z];
    e[9] = v[Z];
    e[10] = n[Z];
    e[11] = 0;

    e[12] = 0;
    e[13] = 0;
    e[14] = 0;
    e[15] = 1;
    return *this;
}

inline Vector4 Matrix4::row3(int i) const {
    return Vector4(e[i], e[4 + i], e[8 + i]);
}


// ---------------- (15) ----------------------------------------
inline Vector4 operator* (const Matrix4& m, const Vector4& v) {
    return Vector4(v[X]*m[X][X] + v[Y]*m[Y][X] + v[Z]*m[Z][X] + v[W]*m[W][X],
                   v[X]*m[X][Y] + v[Y]*m[Y][Y] + v[Z]*m[Z][Y] + v[W]*m[W][Y],
                   v[X]*m[X][Z] + v[Y]*m[Y][Z] + v[Z]*m[Z][Z] + v[W]*m[W][Z]);

}

// ---------------- (16) ----------------------------------------
inline Matrix4 operator* (const Matrix4& m1, const Matrix4& m2) {
    Matrix4 m;
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y) {
            m(x)[y] = 0;
            for (int i = 0; i < 4; ++i) m(x)[y] += m2[x][i] * m1[i][y];
     }
     return m;
}

// ---------------- (17) ----------------------------------------
inline Matrix4 operator*(Real s, const Matrix4& m) {
    return m * s;
}

// ---------------- (18) ----------------------------------------
inline Matrix4 operator*(const Matrix4& m1, Real s) {
    const Real* v1 = m1.readArray();
    Matrix4 m;
    Real* v = m.getArray();
    // do not change e[15]
    for (int i = 0; i < 15; ++i) v[i] = v1[i] * s;
    v[15] = v1[15];
    return m;
}

// ---------------- (19) ----------------------------------------
inline Matrix4 operator+(const Matrix4& m1, const Matrix4& m2) {
    const Real* v1 = m1.readArray();
    const Real* v2 = m2.readArray();
    Matrix4 m;
    Real* v = m.getArray();
    // keep e[15] = 1
    for (int i = 0; i < 15; ++i) v[i] = v1[i] + v2[i];
    v[15] = 1;
    return m;
}

// ---------------- (20) ----------------------------------------
inline Matrix4 operator-(const Matrix4& m1, const Matrix4& m2) {
    const Real* v1 = m1.readArray();
    const Real* v2 = m2.readArray();
    Matrix4 m;
    Real* v = m.getArray();
    // keep e[15] = 1
    for (int i = 0; i < 15; ++i) v[i] = v1[i] - v2[i];
    v[15] = 1;
    return m;
}

// ---------------- (21) ----------------------------------------
inline Matrix4 operator-(const Matrix4& m1) {
    const Real* v1 = m1.readArray();
    Matrix4 m;
    Real* v = m.getArray();
    // keep e[15] = 1
    for (int i = 0; i < 15; ++i) v[i] = - v1[i];
    v[15] = 1;
    return m;
}

// ---------------- (22) ----------------------------------------
inline Matrix4& Matrix4::doTranspose3() {
    Matrix4 m;
    m.setIdentity();
    for (int x = 0; x < 3; ++x) for (int y = 0; y < 3; ++y) m(x)[y] = (*this)[y][x];
    const Real* v = m.readArray();
    for (int i = 0; i < 16; ++i) e[i] = v[i];
    return *this;
}

// ---------------- (23) ----------------------------------------
inline Matrix4& Matrix4::doTranspose4() {
    Matrix4 m;
    for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y) m(x)[y] = (*this)[y][x];
    const Real* v = m.readArray();
    for (int i = 0; i < 16; ++i) e[i] = v[i];
    return *this;
}

// ---------------- (24) ----------------------------------------
inline Matrix4& Matrix4::setRotation(Real yaw, Real pitch, Real roll) {

    Real cy = std::cos(yaw);
    Real sy = std::sin(yaw);
    Real cp = std::cos(pitch);
    Real sp = std::sin(pitch);
    Real cr = std::cos(roll);
    Real sr = std::sin(roll);


    e[0] = cy * cr - sy * cp * sr;
    e[1] = - cy * sr - sy * cp * cr;
    e[2] = sy * sp;
    e[3] = 0;

    e[4] = sy * cr + cy * cp * sr;
    e[5] = - sy * sr + cy * cp * cr;
    e[6] = - cy * sp;
    e[7] = 0;

    e[8] = sp * sr;
    e[9] = sp * cr;
    e[10] = cp;
    e[11] = 0;

    e[12] = 0;
    e[13] = 0;
    e[14] = 0;
    e[15] = 1;

    return *this;
}

// ---------------- (25) ----------------------------------------
inline bool operator== (const Matrix4& m1, const Matrix4& m2) {
    const Real* v1 = m1.readArray();
    const Real* v2 = m2.readArray();

    for (int i = 0; i < 16; ++i) {
        if (!eqz(v1[i] - v2[i])) return false;
    }

    return true;
}


} // namespace Math3D

#ifdef OSTREAM_MATH3D
#include <QDataStream>

inline QDataStream& operator<< (QDataStream& os, const Math3D::Vector4& v) {
    for (int i=0; i<4; ++i)
        os << v[i];
    return os;
}

inline QDataStream& operator>> (QDataStream& os, Math3D::Vector4& v) {
    for (int i=0; i<4; ++i)
        os >> v(i);
    return os;
}

inline QDataStream& operator<< (QDataStream& os, const Math3D::Matrix4& m) {
    for (int y=0; y<4; ++y) {
        for (int x=0;x<4;++x)
            os << m[x][y];
    }
    return os;
}

inline QDebug operator<< (QDebug dbg, const Math3D::Matrix4& m) {
    for (int y=0; y<4; ++y) {
        for (int x=0;x<4;++x)
            dbg << m[x][y];
    }
    return dbg;
}

#endif  // OSTREAM_MATH3D

Q_DECLARE_METATYPE(Math3D::Vector4)
Q_DECLARE_METATYPE(Math3D::Matrix4)


#endif // MATH3D_H
