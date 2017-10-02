#include "patch.h"

using Math3D::Vector4;

Patch::Patch(VectorList cps)
    : mControls(std::move(cps))
{
    mDeg = true;
    for (int i = 0; i < 3; ++i)
        if (!(mControls[i] == mControls[i+1])) {
            mDeg = false;
            break;
        }
}

float Patch::B(int k, float u) {
    float v = 1 - u;
    if (k == 0) return v*v*v;
    if (k == 1) return 3*u*v*v;
    if (k == 2) return 3*u*u*v;
    if (k == 3) return u*u*u;
    return 0;
}

float Patch::DB(int k, float u) {
    float v = 1 - u;
    if (k == 0) return -3*v*v;
    if (k == 1) return -3*v*(2*u-v);
    if (k == 2) return  3*u*(2*v-u);
    if (k == 3) return  3*u*u;
    return 0;
}

Vector4 Patch::Vertex(float u, float v) const {
    Vector4 res;
    for (int m = 0; m < 4; ++m) for (int n = 0; n < 4; ++n) {
        res += B(m, u) * B(n, v) * mControls[4 * m + n];
    }
    return res;
}

Vector4 Patch::Tex(float u, float v) const {
    return Vector4(u, v, 0);
}

Vector4 Patch::Normal(float u, float v) const {
    Vector4 du;
    for (int m = 0; m < 4; ++m) for (int n = 0; n < 4; ++n) {
        du += DB(m, u) * B(n, v) * mControls[4 * m + n];
    }
    Vector4 dv;
    for (int m = 0; m < 4; ++m) for (int n = 0; n < 4; ++n) {
        dv += B(m, u) * DB(n, v) * mControls[4 * m + n];
    }
    return Math3D::cross(du, dv).normalized3();
}

void Patch::Append(DataList* list, Math3D::Vector4 vec, int num) {
    for (int k = 0; k < num; ++k)
        list->append(vec[k]);
}

void Patch::gendata(int div) {
    mVertices.clear();
    mNormals.clear();
    mTexcoords.clear();
    mStrips.clear();

    int num = div + 1;
    for (int m = 0; m < num; ++m) {
        IndexList indices;
        unsigned int mode = GL_TRIANGLE_STRIP;
        if (m == 0 && mDeg) mode = GL_TRIANGLES;
        float u = 1.0 * m / div;
        for (int n = 0; n < num; ++n) {
            float v = 1.0 * n / div;
            Append(&mVertices, Vertex(u, v), 3);
            if (mode == GL_TRIANGLES) {
                Append(&mNormals, Normal(u + 0.001, v), 3);
            } else {
                Append(&mNormals, Normal(u, v), 3);
            }
            Append(&mTexcoords, Tex(u, v), 2);
            if (m == div) continue;
            if (mode == GL_TRIANGLES) {
                if (n == div) continue;
                indices.append(m*num + n);
                indices.append((m + 1)*num + n);
                indices.append((m + 1)*num + n + 1);
            } else {
                indices.append(m*num + n);
                indices.append((m + 1)*num + n);
            }
        }
        if (m == div) continue;
        Strip strip;
        strip.mode = mode;
        strip.indices = indices;
        mStrips.append(strip);
    }
}
