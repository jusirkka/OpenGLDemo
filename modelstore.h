#ifndef MODELSTORE_H
#define MODELSTORE_H

#include <QObject>
#include <QtPlugin>
#include <QList>
#include <QStringList>

#include "blob.h"
#include "math3d.h"

#define WAVEFRONT_LTYPE Demo::WF::LocationType
#define WAVEFRONT_STYPE Demo::WF::ValueType

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
using yyscan_t = void *;
#endif

using Math3D::Vector4;

namespace Demo {

namespace WF {

class LocationType {
public:
    int row;
    int col;
    int pos;
    int prev_col;
    int prev_pos;
};


class ModelError {

public:
    ModelError(QString msg, int row, int col, int pos)
        : emsg(std::move(msg))
        , erow(row)
        , ecol(col)
        , epos(pos)
    {}

    ModelError()
        :emsg(),
          erow(0),
          ecol(0),
          epos(0)
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

using TripletList = QList<Demo::WF::Triplet>;

class ValueType {
public:
    int v_int;
    float v_float;
    int v_triplet[3];
    TripletList v_triplet_list;
};

}

namespace GL {

class ModelStore : public QObject, public Blob
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "net.kvanttiapina.OpenGLDemos.Blob/1.0")
    Q_INTERFACES(Demo::GL::Blob)

public:

    ModelStore();

    // project interface
    void rename(const QString& from, const QString& to);
    void remove(int index);
    void setModel(const QString& key, const QString& path = QString());
    void clean();
    int size();
    const QString& fileName(int);
    const QString& modelName(int);
    QStringList itemSample(const QString& except = QString()) const;

    // grammar interface
    void appendVertex(float, float, float);
    void appendNormal(float, float, float);
    void appendTex(float, float);
    void appendFace(const WF::TripletList&);

    void parseModelData(const QString& path);
    void createError(WF::LocationType* loc, const QString& msg);

    // blob interface implementation
    void draw(unsigned int mode, const QString& attr) const override;


private:

    class VertexData {
    public:
        VertexData(float x, float y, float z): v(x, y, z), n(0, 0, 1, 0), t(0, 0, 0, 0) {}
        VertexData(): v(0, 0, 0, 0), n(0, 0, 1, 0), t(0, 0, 0, 0) {}
        VertexData(Vector4 v0): v(v0.readArray()), n(0, 0, 1, 0), t(0, 0, 0, 0) {}

        Vector4 v, n, t;
    };

    using VertexList = QList<Demo::GL::ModelStore::VertexData>;
    using Vector4List = QList<Math3D::Vector4>;
    using IndexList = QList<GLuint>;


    class Model {
    public:
        Model() = default;
        QString name;
        QString fileName;
        VertexList vertices;
        IndexList indices;
        unsigned int elemOffset;
    };

    using ModelList = QList<Demo::GL::ModelStore::Model>;
    typedef QMap<QString, int> IndexMap;

    Vector4 makeNormal(int start) const;
    void makeModelBuffer();

    bool copyNeeded(unsigned int orig, const WF::Triplet& t) const;


private:

    ModelList mModels;
    IndexMap mIndexMap;

    VertexList mVertices;
    Vector4List mNormals;
    Vector4List mTexCoords;
    IndexList mIndices;

    WF::ModelError mError;
    yyscan_t mScanner;
};

}} // namespace Demo::GL

#define YYLLOC_DEFAULT(Current, Rhs, N) do if (N) {\
    (Current).row = YYRHSLOC (Rhs, 1).row;\
    (Current).col = YYRHSLOC (Rhs, 1).col;\
    (Current).prev_col = YYRHSLOC (Rhs, 1).prev_col;\
    (Current).pos = YYRHSLOC (Rhs, 1).pos;\
    (Current).prev_pos = YYRHSLOC (Rhs, 1).prev_pos;\
    } else {                                                               \
    (Current).row = YYRHSLOC (Rhs, 0).row;\
    (Current).col = YYRHSLOC (Rhs, 0).col;\
    (Current).prev_col = YYRHSLOC (Rhs, 0).prev_col;\
    (Current).pos = YYRHSLOC (Rhs, 0).pos;\
    (Current).prev_pos = YYRHSLOC (Rhs, 0).prev_pos;\
    } while (0)

void wavefront_error(Demo::WF::LocationType*, Demo::GL::ModelStore*, yyscan_t, const char*);



#endif // MODELSTORE_H
