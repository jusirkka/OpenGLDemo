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
using Math3D::dot3;

Camera::Camera(const Vector4& eye, const Vector4& center, const Vector4& up)
{
    reset(eye, center, up);
}


void Camera::reset(const Vector4& eye, const Vector4& center, const Vector4& up) {
    Vector4 z = (eye - center).normalized3();
    Vector4 y = up - dot3(z, up) * z;
    mRot.setBasis(y, z);
    mRot0 = mRot;

    mEye = eye;
    mEye0 = mEye;

    mD = (eye - center).length3();
    mD0 = mD;

    mTot.setTranslation(- mEye);
    mTot = mRot * mTot;

}

void Camera::reset() {
    mRot = mRot0;
    mEye = mEye0;
    mD = mD0;

    mTot.setTranslation(- mEye);
    mTot = mRot * mTot;
}


void Camera::rotate(float phi, float theta) {

    Vector4 d0 = mD * mRot.row3(Math3D::Z); // eye - center

    Matrix4 r;
    r.setRotation(theta, Vector4(sin(phi), cos(phi), 0));
    mRot = r * mRot;

    Vector4 d = mD * mRot.row3(Math3D::Z);

    mEye = mEye + d - d0;

    mTot.setTranslation(- mEye);
    mTot = mRot * mTot;
}

void Camera::pan(float phi, float theta) {
    Matrix4 r;
    r.setRotation(theta, Vector4(sin(phi), cos(phi), 0));
    mRot = r * mRot;

    mTot.setTranslation(- mEye);
    mTot = mRot * mTot;
}

void Camera::zoom(float dz) {
    if (mD + dz < 0.1) return;
    mD += dz;

    Vector4 d = dz * mRot.row3(Math3D::Z); // eye - center

    mEye += d;

    mTot.setTranslation(- mEye);
    mTot = mRot * mTot;
}


const Matrix4& Camera::trans() const {
    return mTot;
}
