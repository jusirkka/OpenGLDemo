//
// C++ Interface: camera
//
// Description:
//
//
// Author: Jukka Sirkka <jukka.sirkka@elisanet.fi>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "math3d.h"


class Camera {

public:

    Camera(const Math3D::Vector4& eye, const Math3D::Vector4& center, const Math3D::Vector4& up);
    void rotate(float phi, float theta);
    void pan(float phi, float theta);
    void zoom(float inc);
    void reset();
    void reset(const Math3D::Vector4& eye, const Math3D::Vector4& center, const Math3D::Vector4& up);
    const Math3D::Matrix4& trans() const;
    ~Camera() {}

private:

    Math3D::Matrix4 mRot, mRot0, mTot;
    Math3D::Vector4 mEye, mEye0;
    Math3D::Real mD, mD0;
};


#endif // _CAMERA_H_
