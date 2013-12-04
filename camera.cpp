//
// C++ Implementation: camera
//
// Description:
//
//
// Author: Jukka Sirkka <jukka.sirkka@iki.fi>, (C) 2013
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "camera.h"

using Math3D::Matrix4;
using Math3D::Vector4;
using Math3D::Real;

Camera::Camera(const Vector4& eye)
{
    mTransReset.setTranslation(- eye);
    mTrans = mTransReset;
    mCenter.setTranslation(Vector4(0, 0, 0));
    mRot.setIdentity();
    mTot = mTrans * mRot * mCenter;
}


void Camera::rotate(float phi, float theta) {
    Matrix4 r;
    r.setRotation(theta, Vector4(sin(phi), cos(phi), 0));
    mRot = r * mRot;
    mTot = mTrans * mRot * mCenter;
}

void Camera::pan(float dx, float dy) {
    Vector4 t = mTrans.translation();
    float len = t.length3();
    Matrix4 c;
    t -= len * mRot.transpose3() * Vector4(dx, dy, 0);
    c.setTranslation(len * t.normalized3());
    mCenter *= c;
    mTot = mTrans * mRot * mCenter;
}

void Camera::zoom(float dz) {
    Vector4 t = mTrans.translation();
    float len = t.length3();
    if (len + dz < 0.1) return;
    mTrans.setTranslation( (len + dz) / len * t);
    mTot = mTrans * mRot * mCenter;
}

void Camera::reset() {
    mTrans = mTransReset;
    mCenter.setTranslation(Vector4(0, 0, 0));
    mRot.setIdentity();
    mTot = mTrans * mRot * mCenter;
}

const Matrix4& Camera::trans() const {
    return mTot;
}
