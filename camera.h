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

    Camera(const Math3D::Vector4& eye);
    void rotate(float phi, float theta);
    void pan(float dx, float dy);
    void zoom(float inc);
    void reset();
    const Math3D::Matrix4& trans() const;
    ~Camera() {}

private:

    Math3D::Matrix4 mTrans;
    Math3D::Matrix4 mTransReset;
    Math3D::Matrix4 mRot;
    Math3D::Matrix4 mCenter;
    Math3D::Matrix4 mTot;
};


#endif // _CAMERA_H_
