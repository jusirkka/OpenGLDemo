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
    mTrans.setTranslation(- eye);
    mRot.setIdentity();
    mTot = mRot * mTrans;
}


void Camera::rotate(float phi, float theta) {
    Matrix4 r;
//    float cp = cos(phi);
//    float sp = sin(phi);
//    float ct = cos(theta);
//    float st = sin(theta);
//    Vector4 x(1 - cp*cp * (1-ct), -sp*cp*(1-ct), -cp*st);
//    Vector4 y(sp*cp*(1-ct), 1 - sp*sp * (1-ct), -sp*st);
//    Vector4 z(cp*st, sp*st, ct);
    // r.setBasis(x, y, z);
    r.setRotation(theta, Vector4(sin(phi), cos(phi), 0));
    mRot = r * mRot;
    mTot = mTrans * mRot;
}

const Matrix4& Camera::trans() const {
    return mTot;
}
