#ifndef PATCH_H
#define PATCH_H

#include <QList>
#include "math3d.h"

class Patch
{
public:

    typedef QList<unsigned int> IndexList;

    class Strip {
    public:
       unsigned int mode;
       IndexList indices;
    };
    typedef QList<Strip> StripList;
    typedef QList<float> DataList;
    typedef QList<Math3D::Vector4> VectorList;

    Patch(const VectorList& controlpoints);

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
