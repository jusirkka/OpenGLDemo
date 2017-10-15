#include "patcher.h"
#include "bezierpatcher.h"

using namespace Demo::WF;

Patcher* Patcher::Create(const QString &name) {
    if (name == "bezier") return new BezierPatcher();
    return nullptr;
}

void Patcher::reset() {
    mVertices.clear();
    mNormals.clear();
    mTexes.clear();
    mWireframe.clear();
    mStrips.clear();
    mUdeg = 0;
    mVdeg = 0;
    extraReset();
}

void Patcher::setControlPoints(uint udeg, uint vdeg, const Vector4Vector& controlpoints) {
    mUdeg = udeg;
    mVdeg = vdeg;
    setCP(controlpoints);
}
