#ifndef MODELSTORE_H
#define MODELSTORE_H

#include <QObject>
#include <QtPlugin>
#include <QVector>
#include <QStringList>

#include "blob.h"
#include "math3d.h"
#include "patcher.h"
#include "projectfolder.h"

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


class TripletIndex {
public:
    TripletIndex(int v = 0, int t = 0, int n = 0)
        : v_index(v)
        , t_index(t)
        , n_index(n) {}

    int v_index;
    int t_index;
    int n_index;
};

using TripletIndexVector = QVector<TripletIndex>;
using IndexVector = QVector<int>;
using NumericVector = QVector<float>;

class ValueType {
public:
    int v_int;
    float v_float;
    int v_triplet[3];
    TripletIndexVector v_triplets;
    QString v_string;
    NumericVector v_floats;
    IndexVector v_ints;
};


}

class GLWidget;

namespace GL {


class ModelStore : public ProjectFolder, public Blob
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "net.kvanttiapina.OpenGLDemos.Blob/1.0")
    Q_INTERFACES(Demo::GL::Blob)

public:

    ModelStore();



    // project interface
    void rename(const QString& from, const QString& to) override;
    void remove(int index) override;
    int size() const override;
    QString fileName(int) const override;
    QString itemName(int) const override;
    QStringList items() const override;
    void setItem(const QString& key, const QString& path = QString()) override;

    void clean();

    // grammar interface
    void appendVertex(float x, float y, float z, float w = 1);
    void appendNormal(float, float, float);
    void appendTex(float, float);
    void appendFace(const WF::TripletIndexVector&);
    bool inPatchDef() const;
    void setPatchType(const QString& name, bool rat = false);
    void setPatchKnots(const QString& v, const WF::NumericVector& knots);
    void setPatchRank(int udeg, int vdeg);
    bool checkPatchState() const;
    void beginPatch(float u0, float u1, float v0, float v1, const WF::IndexVector& controlpoints);
    void endPatch();

    enum Error {InSurfDef, StateNotComplete, SurfDefRequired, Unused};
    void createError(const QString& item, Error err);

    void parseModelData(const QString& path);

    // blob interface implementation
    void draw(unsigned int mode, const QString& attr) const override;
    // drawing context
    void setContext(GLWidget* context);


private:

    class VertexData {
    public:
        VertexData(float x, float y, float z, float w = 1)
            : vertex(x, y, z, w)
            , normal(0, 0, 1, 0)
            , tex(0, 0, 0, 0) {}
        VertexData()
            : vertex(0, 0, 0, 0)
            , normal(0, 0, 1, 0)
            , tex(0, 0, 0, 0) {}
        VertexData(Vector4 v0)
            : vertex(v0.readArray(), 4)
            , normal(0, 0, 1, 0)
            , tex(0, 0, 0, 0) {}

        VertexData(const Vector4& v, const Vector4& n, const Vector4& t)
            : vertex(v.readArray(), 4)
            , normal(n.readArray(), 3)
            , tex(t.readArray(), 2) {}

        Vector4 vertex, normal, tex;
    };

    using VertexDataVector = QVector<Demo::GL::ModelStore::VertexData>;
    using Vector4Vector = QVector<Math3D::Vector4>;
    using IndexVector = QVector<GLuint>;
    using OffsetVector = QVector<uintptr_t>;
    using IndexHash = QHash<QString, GLuint>;
    using Strip = QVector<GLuint>;
    using StripVector = QVector<Strip>;

    class PatchState {
    public:
        PatchState()
            : patcher(nullptr)
            , udeg(0)
            , vdeg(0)
            , insurf(false)
            , vertices()
            , wireframe()
            , strips() {}

            ~PatchState() {delete patcher;}

        void reset() {
            delete patcher;
            patcher = nullptr;
            udeg = 0;
            vdeg = 0;
            insurf = false;
            vertices.clear();
            wireframe.clear();
            strips.clear();
        }

        void applyOffset(uint offset) {
            for (uint& idx: wireframe) {
                idx += offset;
            }
            for (IndexVector& strip: strips) {
                for (uint& idx: strip) {
                    idx += offset;
                }
            }
        }

        bool checkState() const {
            if (!patcher) return false;
            if (udeg == 0) return false;
            if (vdeg == 0) return false;
            return true;
        }

        WF::Patcher* patcher;
        uint udeg;
        uint vdeg;
        bool insurf;
        VertexDataVector vertices;
        IndexVector wireframe;
        StripVector strips;
    };


    class Model {
    public:
        Model() = default;
        QString name;
        QString fileName;
        VertexDataVector vertices;
        StripVector strips;
        OffsetVector stripOffsets;
        QVector<GLsizei> stripSizes;
        IndexVector wireFrame;
        uint wireFrameOffset;

    };

    using ModelVector = QVector<Demo::GL::ModelStore::Model>;
    using IndexMap = QMap<QString, int>;

    Vector4 makeNormal(int start) const;
    void makeModelBuffer();
    void resetParser();
    QString makeKey(const WF::TripletIndex&);
    QString makeKey(uint v1, uint v2);


private:

    GLWidget* mContext;

    ModelVector mModels;
    IndexMap mIndexMap;

    VertexDataVector mVertexData;
    IndexVector mTriangleIndices; // points to vertexdata -- 3 consecutive points form a triangle
    IndexVector mWireframeIndices;

    // temporary data storages
    Vector4Vector mVertices;
    Vector4Vector mNormals;
    Vector4Vector mTexCoords;
    IndexHash mVertexCache;
    IndexHash mEdgeCache;


    IndexVector mGenNormals; // indices of missing normals in vertexdata
    IndexVector mGenTexes; // indices of missing tex coords in vertexdata

    WF::ModelError mError;
    yyscan_t mScanner;

    PatchState mPatchState;
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
