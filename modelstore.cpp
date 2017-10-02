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


using Math3D::Vector4;

using namespace Demo::GL;

ModelStore::ModelStore():
    QObject(),
    Blob(),
    mError(),
    mScanner(nullptr) {
    setObjectName("modelstore");
    mData[GL_ARRAY_BUFFER] = Data();
    mData[GL_ELEMENT_ARRAY_BUFFER] = Data();
}


#define ALT(item) case item: throw RunError(#item, 0); break

void ModelStore::draw(unsigned int mode, const QString& name) const {
    int id;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &id);
    if (id == 0) {
        throw RunError("Missing array buffer binding", 0);
    }
    if (!mIndexMap.contains(name)) {
        throw RunError(QString("%1: no such model").arg(name), 0);
    }
    // qDebug() << "Drawing" << name;
    int index = mIndexMap[name];
    GLsizei count = mModels[index].indices.size();
    size_t offset = mModels[index].elemOffset;
    // qDebug()  << offset << count;
    glDrawElements((GLenum) mode, count, GL_UNSIGNED_INT, (void*) offset);

    switch (glGetError()) {
    ALT(GL_INVALID_ENUM);
    ALT(GL_INVALID_VALUE);
    ALT(GL_INVALID_OPERATION);
    default: ;// nothing
    }

}

#undef ALT

void ModelStore::appendVertex(float x, float y, float z) {
    mVertices.append(VertexData(x, y, z));
}

void ModelStore::appendNormal(float x, float y, float z) {
    mNormals.append(Vector4(x, y, z));
}

void ModelStore::appendTex(float u, float v) {
    mTexCoords.append(Vector4(u, v));
}

static unsigned int my_index(int x, unsigned int l) {
    if (x < 1) return (x + l) % l;
    return (x - 1) % l;
}

void ModelStore::appendFace(const WF::TripletList& ts) {
    if (ts.size() < 3) return;
    unsigned int lv = mVertices.size();
    if (!lv) return;
    unsigned int ln = mNormals.size();
    unsigned int lt = mTexCoords.size();

    IndexList face;
    // copy normals & texcoords
    for (const WF::Triplet& t: ts) {
        unsigned int v_idx = my_index(t.v_index, lv);
        if (copyNeeded(v_idx, t)) {
            // qDebug() << "copying" << v_idx;
            VertexData c(mVertices[v_idx].v);
            v_idx = mVertices.size();
            mVertices.append(c);
        }
        face.append(v_idx);

        if (t.n_index) {
            mVertices[v_idx].n = mNormals[my_index(t.n_index, ln)];
        }

        if (t.t_index) {
            mVertices[v_idx].t = mTexCoords[my_index(t.t_index, lt)];
        }
    }

    // append triangle indices
    for (int i = 2; i < face.size(); ++i) {
        mIndices.append(face[0]);
        mIndices.append(face[i-1]);
        mIndices.append(face[i]);
    }
}

bool ModelStore::copyNeeded(unsigned int orig, const WF::Triplet& t) const {

    const VertexData& v = mVertices[orig];

    if (t.n_index && v.n[3] != 1) {
        unsigned int n_idx = my_index(t.n_index, mNormals.size());
        if (v.n != mNormals[n_idx]) return true;
    }

    if (t.t_index && v.t[3] != 1) {
        unsigned int t_idx = my_index(t.t_index, mTexCoords.size());
        if (v.t != mTexCoords[t_idx]) return true;
    }

    return false;
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
    for (auto& model: mModels) {
        uint32_t modelSize = model.vertices.size() * sizeof(GLfloat) * 8;
        // blob specs: num components, type, is normalized, packing offset, data offset
        mSpecs[model.name + ":vertex"] = BlobSpec(3, GL_FLOAT, false, 0, vertexSize);
        mSpecs[model.name + ":normal"] = BlobSpec(3, GL_FLOAT, true, 0, vertexSize + modelSize / 8 * 3);
        mSpecs[model.name + ":tex"] = BlobSpec(2, GL_FLOAT, false, 0, vertexSize + modelSize / 8 * 6);
        model.elemOffset = elemSize;
        elemSize += model.indices.size() * sizeof(GLuint);
        vertexSize += modelSize;
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
    for (auto& m: mModels) {
        for (auto& d: m.vertices) {
            ::memcpy(p, d.v.readArray(), nsize); p += nsize;
        }
        for (auto& d: m.vertices) {
            ::memcpy(p, d.n.readArray(), nsize); p += nsize;
        }
        for (auto& d: m.vertices) {
            ::memcpy(p, d.t.readArray(), tsize); p += tsize;
        }

        for (auto idx: qAsConst(m.indices)) {
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
    QString inp(square);
    if (!path.isEmpty()) {
        QFile file(path);
        file.open(QFile::ReadOnly);
        inp = QString(file.readAll()).append('\n');
        file.close();
    }

    wavefront_lex_init(&mScanner);
    YY_BUFFER_STATE buf = wavefront__scan_string(inp.toUtf8().data(), mScanner);
    int err = wavefront_parse(this, mScanner);
    wavefront__delete_buffer(buf, mScanner);

    if (err) throw WF::ModelError(mError);

    // gen missing tex coords
    if (mTexCoords.isEmpty()) {
        // qDebug() << "gen tex" << path;
        for (auto& vd: mVertices) {
            // (x,y,z) -> (x,y)
            vd.t = vd.v;
            vd.t(2) = 0;
        }
    }

    // gen missing normals
    if (mNormals.isEmpty()) {
        // qDebug() << "gen normals" << path;
        QVector<Vector4> normalSum(mVertices.size(), Vector4());
        QVector<int> faceCount(mVertices.size(), 0);
        int numFaces = mIndices.size() / 3; // they are triangles
        for (int face = 0; face < numFaces; ++face) {
            Vector4 n = makeNormal(3*face);
            for (int p = 0; p < 3; ++p) {
                normalSum[mIndices[3*face+p]] += n;
                faceCount[mIndices[3*face+p]] += 1;
            }
        }
        for (int i = 0; i < mVertices.size(); ++i) {
            if (faceCount[i] > 0) {
                mVertices[i].n = (normalSum[i] * (1.0 / faceCount[i])).normalized3();
            }
        }
    }
}

Vector4 ModelStore::makeNormal(int start) const {
    Vector4 v0 = mVertices[mIndices[start+0]].v;
    Vector4 v1 = mVertices[mIndices[start+1]].v;
    Vector4 v2 = mVertices[mIndices[start+2]].v;
    return Math3D::cross(v1 - v0, v2 - v0).normalized3();
}


void ModelStore::setModel(const QString& key, const QString& path) {
    try {
        parseModelData(path);

        Model model;
        model.name = key;
        model.fileName = path;
        model.vertices = mVertices;
        model.indices = mIndices;

        if (mIndexMap.contains(key)) {
            mModels[mIndexMap[key]] = model;
        } else {
            mIndexMap[key] = mModels.size();
            mModels.append(model);
        }

        makeModelBuffer();

    } catch (WF::ModelError& e) {
        qDebug() << e.msg() << e.row() << e.col();
    }
    mVertices.clear();
    mNormals.clear();
    mTexCoords.clear();
    mIndices.clear();
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

void ModelStore::createError(WF::LocationType* loc, const QString &msg) {
    mError = WF::ModelError(msg, loc->row, loc->col, loc->pos);
}

void wavefront_error(Demo::WF::LocationType* loc, Demo::GL::ModelStore* models, yyscan_t, const char* msg) {
    models->createError(loc, QString(msg));
}

