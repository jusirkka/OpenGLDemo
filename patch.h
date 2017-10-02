#ifndef PATCH_H
#define PATCH_H

#include <QList>
#include "math3d.h"

class Patch
{
public:

    using IndexList = QList<unsigned int>;

    class Strip {
    public:
       unsigned int mode;
       IndexList indices;
    };
    using StripList = QList<Patch::Strip>;
    using DataList = QList<float>;
    using VectorList = QList<Math3D::Vector4>;

    Patch(VectorList controlpoints);

    void gendata(int division);

    const DataList& vertices() const {return mVertices;}
    const DataList& normals() const {return mNormals;}
    const DataList& texcoords() const {return mTexcoords;}
    const StripList& strips() const {return mStrips;}

private:

    static float B(int, float);
    static float DB(int, float);

    static void Append(DataList*, Math3D::Vector4, int);

    Math3D::Vector4 Vertex(float u, float v) const;
    Math3D::Vector4 Normal(float u, float v) const;
    Math3D::Vector4 Tex(float u, float v) const;

private:

    const VectorList mControls;

    DataList mVertices;
    DataList mNormals;
    DataList mTexcoords;
    StripList mStrips;
    bool mDeg;

};

#endif // PATCH_H
