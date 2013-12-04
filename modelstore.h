#ifndef MODELSTORE_H
#define MODELSTORE_H

#include <QObject>
#include <QtPlugin>
#include <QList>
#include <QStringList>

#include "blob.h"
#include "math3d.h"

using Math3D::Vector4;

namespace GL {


class ModelError {

public:
    ModelError(const QString& msg, int row, int col, int pos)
        :emsg(msg),
          erow(row),
          ecol(col),
          epos(pos)
    {}

    const QString msg() const {return emsg;}
    int row() const {return erow;}
    int col() const {return ecol;}
    int pos() const {return epos;}

private:

    QString emsg;
    int erow;
    int ecol;
    int epos;

};

class Triplet {
public:
    Triplet(int v = 0, int t = 0, int n = 0):
        v_index(v),
        t_index(t),
        n_index(n) {}
     int v_index;
     int t_index;
     int n_index;
};

typedef QList<Triplet> TripletList;


class ModelStore : public QObject, public Blob
{
    Q_OBJECT
    Q_INTERFACES(GL::Blob)

public:

    // project interface
    static void Clean();
    static void SetModel(const QString& key, const QString& path = QString(""));
    static int Size();
    static const QString& ModelName(int);
    static const QString& FileName(int);
    static void Rename(const QString& from, const QString& to);
    static void Remove(int index);

    // grammar interface
    static void AppendVertex(float, float, float);
    static void AppendNormal(float, float, float);
    static void AppendTex(float, float);
    static void AppendFace(const TripletList&);


    ModelStore();
    // b[ob interface implementation
    void draw(unsigned int mode, const QString& attr) const;
    ~ModelStore();



private:

    static ModelStore* instance();


    void rename(const QString& from, const QString& to);
    void remove(int index);
    void setModel(const QString& key, const QString& path);
    void clean();
    int size();
    const QString& fileName(int);
    const QString& modelName(int);

    void appendVertex(float, float, float);
    void appendNormal(float, float, float);
    void appendTex(float, float);
    void appendFace(const TripletList&);

    void parseModelData(const QString& path);



private:

    class Model {
    public:
        Model() {}
        unsigned int elemOffset;
        unsigned int elemLength;
        unsigned int vertexOffset;
        unsigned int vertexLength;
    };

    typedef QMap<QString, Model> ModelMap;

    class VertexData {
    public:
        VertexData(float x, float y, float z)
            : v(x, y, z), n(0, 0, 1, 0), t(0, 0, 0, 0) {}
        VertexData()
            : v(0, 0, 0, 0), n(0, 0, 1, 0), t(0, 0, 0, 0) {}

        Vector4 v, n, t;
    };

    typedef QList<VertexData> VertexList;
    typedef QList<Vector4> Vector4List;
    typedef QList<unsigned int> IndexList;


private:

    ModelMap mModels;
    QStringList mNames;
    QStringList mFileNames;

    VertexList mVertices;
    Vector4List mNormals;
    Vector4List mTexCoords;
    IndexList mIndices;

};

} // namespace GL

#endif // MODELSTORE_H
