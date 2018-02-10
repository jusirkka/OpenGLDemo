#include "modelstore.h"
#include <QDebug>
#include <QPluginLoader>
#include <QFile>

#include "wavefront_parser.h"
#ifndef YYSTYPE
#define YYSTYPE WAVEFRONT_STYPE
#endif
#ifndef YYLTYPE
#define YYLTYPE WAVEFRONT_LTYPE
#endif
#include "wavefront_scanner.h"
#include "gl_lang_runner.h"
#include "triangleoptimizer.h"
#include "gl_widget.h"
#include "patcher.h"

using Math3D::Vector4;

using namespace Demo::GL;

ModelStore::ModelStore()
    : QObject()
    , Blob()
    , mContext(nullptr)
    , mError()
    , mScanner(nullptr)
    , mPatchState()
{
    setObjectName("modelstore");
    wavefront_lex_init(&mScanner);
    mData[GL_ARRAY_BUFFER] = Data();
    mData[GL_ELEMENT_ARRAY_BUFFER] = Data();
}

void ModelStore::setContext(GLWidget *context) {
    mContext = context;
}

#define ALT(item) case item: throw RunError(#item, 0); break

void ModelStore::draw(unsigned int mode, const QString& name) const {
    if (!mContext) {
        throw RunError("Context not initialized", 0);
    }
    int id;
    mContext->glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &id);
    if (id == 0) {
        throw RunError("Missing array buffer binding", 0);
    }
    if (!mIndexMap.contains(name)) {
        throw RunError(QString("%1: no such model").arg(name), 0);
    }
    int index = mIndexMap[name];
    if (mode == GL_TRIANGLES || mode == GL_POINTS) {
        if (mode == GL_TRIANGLES) mode = GL_TRIANGLE_STRIP;
        const GLsizei* count = mModels[index].stripSizes.constData();
        const uintptr_t* offsets = mModels[index].stripOffsets.constData();
        GLsizei numStrips = mModels[index].stripOffsets.size();
        mContext->glMultiDrawElements(mode, count, GL_UNSIGNED_INT, (const GLvoid*const*) offsets, numStrips);
        // for (int i = 0; i < numStrips; i++) {
        //     mContext->glDrawElements(mode, count[i], GL_UNSIGNED_INT, (const GLvoid*) offsets[i]);
        // }
    } else if (mode == GL_LINES) {
        GLsizei count = mModels[index].wireFrame.size();
        size_t offset = mModels[index].wireFrameOffset;
        mContext->glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, (const GLvoid*) offset);
    } else {
        throw RunError("Unsupported drawing mode", 0);
    }

    switch (glGetError()) {
    ALT(GL_INVALID_ENUM);
    ALT(GL_INVALID_VALUE);
    ALT(GL_INVALID_OPERATION);
    default: ;// nothing
    }
}

#undef ALT

void ModelStore::appendVertex(float x, float y, float z, float w) {
    mVertices.append(Vector4(x, y, z, w));
}

void ModelStore::appendNormal(float x, float y, float z) {
    mNormals.append(Vector4(x, y, z, 0));
}

void ModelStore::appendTex(float u, float v) {
    mTexCoords.append(Vector4(u, v, 0, 0));
}

static unsigned int makeIndex(int x, unsigned int l) {
    if (l == 0) return 0;
    if (x < 1) return (x + l) % l;
    return (x - 1) % l;
}

QString ModelStore::makeKey(const WF::TripletIndex& t) {
    unsigned int v_index = makeIndex(t.v_index, mVertices.size());
    unsigned int n_index = makeIndex(t.n_index, mNormals.size());
    unsigned int t_index = makeIndex(t.t_index, mTexCoords.size());

    if (t.n_index && t.t_index) {
        return QString("%1:%2:%3").arg(v_index).arg(n_index).arg(t_index);
    }

    if (t.n_index) {
        return QString("%1:%2:").arg(v_index).arg(n_index);
    }

    if (t.t_index) {
        return QString("%1::%2").arg(v_index).arg(t_index);
    }

    return QString("%1::").arg(v_index);
}

QString ModelStore::makeKey(uint v1, uint v2) {
    if (v1 < v2) return QString("%1:%2").arg(v1).arg(v2);
    return QString("%1:%2").arg(v2).arg(v1);
}

void ModelStore::appendFace(const WF::TripletIndexVector& ts) {
    if (ts.size() < 3) return;

    unsigned int Lv = mVertices.size();
    if (!Lv) return;

    IndexVector face;
    for (const WF::TripletIndex& t: ts) {
        QString key = makeKey(t);
        if (!mVertexCache.contains(key)) {
            unsigned int v_index = makeIndex(t.v_index, Lv);
            VertexData data(mVertices[v_index]);

            if (!t.n_index) {
                mGenNormals.append(v_index);
            } else {
                unsigned int n_index = makeIndex(t.n_index, mNormals.size());
                data.normal = mNormals[n_index];
            }

            if (!t.t_index) {
                mGenTexes.append(v_index);
            } else {
                unsigned int t_index = makeIndex(t.t_index, mTexCoords.size());
                data.tex = mTexCoords[t_index];
            }
            mVertexData.append(data);
            mVertexCache[key] = mVertexData.size() - 1;
        }
        face.append(mVertexCache[key]);
    }

    // append wireframe indices
    for (int i = 1; i < face.size(); i++) {
        QString key = makeKey(face[i-1], face[i]);
        if (mEdgeCache.contains(key)) continue;
        mEdgeCache[key] = 0; // value is unimportant
        mWireframeIndices.append(face[i-1]);
        mWireframeIndices.append(face[i]);
    }

    // append triangle indices
    for (int i = 2; i < face.size(); ++i) {
        mTriangleIndices.append(face[0]);
        mTriangleIndices.append(face[i-1]);
        mTriangleIndices.append(face[i]);
    }
}


bool ModelStore::inPatchDef() const {
    return mPatchState.insurf;
}

void ModelStore::setPatchType(const QString& name, bool) {
    delete mPatchState.patcher;
    mPatchState.patcher = WF::Patcher::Create(name);
}

void ModelStore::setPatchKnots(const QString& v, const WF::NumericVector& knots) {
    mPatchState.patcher->setKnots(v, knots);
}

void ModelStore::setPatchRank(int udeg, int vdeg) {
    mPatchState.udeg = udeg;
    mPatchState.vdeg = vdeg;
}

bool ModelStore::checkPatchState() const {
    return mPatchState.checkState();
}

void ModelStore::beginPatch(float u0, float u1, float v0, float v1, const WF::IndexVector& controlpoints) {
    mPatchState.insurf = true;
    mPatchState.patcher->setBoundary(u0, u1, v0, v1);
    Vector4Vector cv;
    uint Lv = mVertices.size();
    for (const uint& i: controlpoints) {
        cv.append(mVertices[makeIndex(i, Lv)]);
    }
    mPatchState.patcher->setControlPoints(mPatchState.udeg, mPatchState.vdeg, cv);
}

void ModelStore::endPatch() {
    mPatchState.insurf = false;
    mPatchState.patcher->gendata(mPatchState.vertices.size());
    mPatchState.wireframe.append(mPatchState.patcher->wireframe());
    mPatchState.strips.append(mPatchState.patcher->strips());
    uint len = mPatchState.patcher->vertices().size();
    for (uint i = 0; i < len; i++) {
        const Vector4& v = mPatchState.patcher->vertices()[i];
        const Vector4& n = mPatchState.patcher->normals()[i];
        const Vector4& t = mPatchState.patcher->texes()[i];
        mPatchState.vertices.append(VertexData(v, n, t));
    }
    mPatchState.patcher->reset();
}


void ModelStore::rename(const QString& from, const QString& to) {
    if (mIndexMap.contains(from)) {
        int index = mIndexMap.take(from);
        mIndexMap[to] = index;
        mModels[index].name = to;
        mSpecs[to + ":vertex"] = mSpecs.take(from + ":vertex");
        mSpecs[to + ":normal"] = mSpecs.take(from + ":normal");
        mSpecs[to + ":tex"] = mSpecs.take(from + ":tex");
    }
}

void ModelStore::makeModelBuffer() {

    mSpecs.clear();
    delete mData[GL_ARRAY_BUFFER].data;
    mData[GL_ARRAY_BUFFER] = Data();
    delete mData[GL_ELEMENT_ARRAY_BUFFER].data;
    mData[GL_ELEMENT_ARRAY_BUFFER] = Data();

    // pass 1: compute specs & buffer length
    uint32_t vertexSize = 0;
    uint32_t elemSize = 0;
    for (Model& model: mModels) {
        // vertex data sizes & offsets
        uint32_t modelSize = model.vertices.size() * sizeof(GLfloat) * 8;
        // blob specs: num components, type, is normalized, packing offset, data offset
        mSpecs[model.name + ":vertex"] = BlobSpec(3, GL_FLOAT, false, 0, vertexSize);
        mSpecs[model.name + ":normal"] = BlobSpec(3, GL_FLOAT, true, 0, vertexSize + modelSize / 8 * 3);
        mSpecs[model.name + ":tex"] = BlobSpec(2, GL_FLOAT, false, 0, vertexSize + modelSize / 8 * 6);
        vertexSize += modelSize;
        // element index sizes & offsets
        model.stripOffsets.clear();
        model.stripSizes.clear();
        int stripSize = 0;
        for (const Strip& strip: qAsConst(model.strips)) {
            model.stripOffsets.append(elemSize + stripSize);
            model.stripSizes.append(strip.size());
            stripSize += strip.size() * sizeof(GLuint);
        }
        elemSize += stripSize;
        model.wireFrameOffset = elemSize;
        elemSize += model.wireFrame.size() * sizeof(GLuint);
    }

    mData[GL_ARRAY_BUFFER].data = new char[vertexSize];
    mData[GL_ARRAY_BUFFER].length = vertexSize;
    mData[GL_ELEMENT_ARRAY_BUFFER].data = new char[elemSize];
    mData[GL_ELEMENT_ARRAY_BUFFER].length = elemSize;

    unsigned int nsize = 3 * sizeof(GLfloat);
    unsigned int tsize = 2 * sizeof(GLfloat);
    unsigned int isize = 1 * sizeof(GLuint);
    char* p = mData[GL_ARRAY_BUFFER].data;
    char* q = mData[GL_ELEMENT_ARRAY_BUFFER].data;
    for (const Model& m: qAsConst(mModels)) {
        for (auto& d: m.vertices) {
            ::memcpy(p, d.vertex.readArray(), nsize); p += nsize;
        }
        for (auto& d: m.vertices) {
            ::memcpy(p, d.normal.readArray(), nsize); p += nsize;
        }
        for (auto& d: m.vertices) {
            ::memcpy(p, d.tex.readArray(), tsize); p += tsize;
        }

        for (const Strip& strip: m.strips) {
            for (auto idx: strip) {
                ::memcpy(q, &idx, isize); q += isize;
            }
        }

        for (auto idx: m.wireFrame) {
            ::memcpy(q, &idx, isize); q += isize;
        }
    }
}


void ModelStore::remove(int index) {
    if (index < 0 || index >= mModels.size()) return;
    mModels.removeAt(index);
    mIndexMap.clear();
    for (int k = 0; k < mModels.size(); ++k) {
        mIndexMap[mModels[k].name] = k;
    }
    makeModelBuffer();
}

static const char square [] =
    "v -1 1 1\n"
    "v -1 -1 1\n"
    "v 1 -1 1\n"
    "v 1 1 1\n"
    "vt 0 1 \n"
    "vt 0 0 \n"
    "vt 1 0 \n"
    "vt 1 1 \n"
    "vn 0 0 1\n"
    "f 1/1/1 2/2/1 3/3/1 4/4/1\n";


void ModelStore::parseModelData(const QString& path) {

    resetParser();

    QString inp(square);
    if (!path.isEmpty()) {
        QFile file(path);
        file.open(QFile::ReadOnly);
        inp = QString(file.readAll()).append('\n');
        file.close();
    }

    YY_BUFFER_STATE buf = wavefront__scan_string(inp.toUtf8().data(), mScanner);
    int err = wavefront_parse(this, mScanner);
    wavefront__delete_buffer(buf, mScanner);

    if (err) throw WF::ModelError(mError);

    // gen missing tex coords
    if (!mGenTexes.isEmpty()) {
        // qDebug() << "gen tex" << path;
        for (auto v_index: qAsConst(mGenTexes)) {
            // (x,y,z) -> (x,y)
            mVertexData[v_index].tex = mVertexData[v_index].vertex;
            mVertexData[v_index].tex(2) = 0;
        }
    }

    // gen missing normals
    if (!mGenNormals.isEmpty()) {
        // qDebug() << "gen normals" << path;
        Vector4Vector normalSum(mVertexData.size(), Vector4());
        IndexVector faceCount(mVertexData.size(), 0);
        int numFaces = mTriangleIndices.size() / 3; // they are triangles
        for (int face = 0; face < numFaces; ++face) {
            Vector4 n = makeNormal(3*face);
            for (int p = 0; p < 3; ++p) {
                normalSum[mTriangleIndices[3*face+p]] += n;
                faceCount[mTriangleIndices[3*face+p]] += 1;
            }
        }
        for (auto v_index: qAsConst(mGenNormals)) {
            if (faceCount[v_index] > 0) {
                mVertexData[v_index].normal = (normalSum[v_index] * (1.0 / faceCount[v_index])).normalized3();
            }
        }
    }
}

Vector4 ModelStore::makeNormal(int start) const {
    Vector4 v0 = mVertexData[mTriangleIndices[start+0]].vertex;
    Vector4 v1 = mVertexData[mTriangleIndices[start+1]].vertex;
    Vector4 v2 = mVertexData[mTriangleIndices[start+2]].vertex;
    return Math3D::cross(v1 - v0, v2 - v0).normalized3();
}


void ModelStore::setModel(const QString& key, const QString& path) {
    try {
        parseModelData(path);
    } catch (WF::ModelError& e) {
        qWarning() << e.msg() << e.row() << e.col();
        // parse error - add default object
        parseModelData("");
    }

    AC::TriangleOptimizer stripper(mTriangleIndices);
    // qDebug() << stripper.strips().size();

    Model model;
    model.name = key;
    model.fileName = path;
    model.vertices = mVertexData;
    model.strips = stripper.strips();
    model.wireFrame = mWireframeIndices;

    if (!mPatchState.vertices.isEmpty()) {
        mPatchState.applyOffset(model.vertices.size());
        model.vertices.append(mPatchState.vertices);
        model.strips.append(mPatchState.strips);
        model.wireFrame.append(mPatchState.wireframe);
    }

    resetParser();

    if (mIndexMap.contains(key)) {
        mModels[mIndexMap[key]] = model;
    } else {
        mIndexMap[key] = mModels.size();
        mModels.append(model);
    }

    makeModelBuffer();

}

void ModelStore::resetParser() {
    mVertexData.clear();
    mTriangleIndices.clear();
    mWireframeIndices.clear();

    mVertices.clear();
    mNormals.clear();
    mTexCoords.clear();
    mVertexCache.clear();
    mEdgeCache.clear();

    mGenNormals.clear();
    mGenTexes.clear();

    mPatchState.reset();
}

void ModelStore::clean() {
    mModels.clear();
    mIndexMap.clear();
    mSpecs.clear();

    delete mData[GL_ARRAY_BUFFER].data;
    mData[GL_ARRAY_BUFFER] = Data();

    delete mData[GL_ELEMENT_ARRAY_BUFFER].data;
    mData[GL_ELEMENT_ARRAY_BUFFER] = Data();

}

int ModelStore::size() {
    return mModels.size();
}

const QString& ModelStore::fileName(int index) {
    return mModels[index].fileName;
}

const QString& ModelStore::modelName(int index) {
    return mModels[index].name;
}

QStringList ModelStore::itemSample(const QString& except) const {
    QStringList r;
    const auto keys = mIndexMap.keys();
    for (auto& k: keys) {
        if (!except.isEmpty() && k == except) continue;
        r.append(k);
    }
    return r;
}

void ModelStore::createError(const QString &msg, Error err) {
    QString detail;
    switch (err) {
    case InSurfDef:
        detail = "%1 cannot be used inside surface definition";
        break;
    case StateNotComplete:
        detail = "%1 cannot be used when surface state is not complete";
        break;
    case SurfDefRequired:
        detail = "%1 can only be used inside surface definition";
        break;
    default:
        detail = "%1";
    }
    WF::LocationType* loc = wavefront_get_lloc(mScanner);
    mError = WF::ModelError(detail.arg(msg), loc->row, loc->col, loc->pos);
}

void wavefront_error(Demo::WF::LocationType*, Demo::GL::ModelStore* models, yyscan_t, const char* msg) {
    models->createError(msg, Demo::GL::ModelStore::Error::Unused);
}

