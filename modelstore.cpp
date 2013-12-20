#include "modelstore.h"
#include <QDebug>
#include <QPluginLoader>
#include <QFile>

extern "C"
{

#include "s_o.h"
#include "s_o_types.h"

int g_oparse(void);
extern int g_odebug;

extern char model_error_buffer[256];

}


using Math3D::Vector4;

static GL::ModelStore* staticInstance() {
    foreach (QObject *plugin, QPluginLoader::staticInstances()) {
        GL::ModelStore* store = qobject_cast<GL::ModelStore*>(plugin);
        if (store) return store;
    }
    return 0;
}

GL::ModelStore* GL::ModelStore::instance() {
    static GL::ModelStore* store = staticInstance();
    return store;
}


void GL::ModelStore::Clean() {
    instance()->clean();
}

void GL::ModelStore::SetModel(const QString& key, const QString& path) {
    instance()->setModel(key, path);
}

int GL::ModelStore::Size() {
    return instance()->size();
}

const QString& GL::ModelStore::ModelName(int idx) {
    return instance()->modelName(idx);
}

const QString& GL::ModelStore::FileName(int idx) {
    return instance()->fileName(idx);
}

void GL::ModelStore::Rename(const QString& from, const QString& to) {
    instance()->rename(from, to);
}

void GL::ModelStore::Remove(int index) {
    instance()->remove(index);
}

void GL::ModelStore::AppendVertex(float x, float y, float z) {
    instance()->appendVertex(x, y, z);
}

void GL::ModelStore::AppendNormal(float x, float y, float z) {
    instance()->appendNormal(x, y, z);
}

void GL::ModelStore::AppendTex(float u, float v) {
    instance()->appendTex(u, v);
}

void GL::ModelStore::AppendFace(const TripletList& triplets) {
    instance()->appendFace(triplets);
}

GL::ModelStore::ModelStore()
    : QObject(),
      Blob()
{
    setObjectName("modelstore");
    mData[GL_ARRAY_BUFFER] = Data();
    mData[GL_ELEMENT_ARRAY_BUFFER] = Data();
}


void GL::ModelStore::draw(unsigned int mode, const QString& attr) const {
    int name;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &name);
    if (name == 0) return;
    if (mModels.contains(attr)) {
        Model model = mModels[attr];
        glDrawElements((GLenum) mode,
                       (GLsizei) model.elemLength / sizeof(GLuint),
                       GL_UNSIGNED_INT,
                       (void*) model.elemOffset);
    }
}


void GL::ModelStore::appendVertex(float x, float y, float z) {
    mVertices.append(VertexData(x, y, z));
}

void GL::ModelStore::appendNormal(float x, float y, float z) {
    mNormals.append(Vector4(x, y, z));
}

void GL::ModelStore::appendTex(float u, float v) {
    mTexCoords.append(Vector4(u, v));
}

static unsigned int my_index(int x, unsigned int l) {
    if (x < 1) return (x + l) % l;
    return (x - 1) % l;

}

void GL::ModelStore::appendFace(const TripletList& ts) {
    if (ts.size() < 3) return;
    unsigned int lv = mVertices.size();
    if (!lv) return;
    unsigned int ln = mNormals.size();
    unsigned int lt = mTexCoords.size();

    // copy normals & texcoords
    for (int i = 0; i < 2; ++i) {
        unsigned int v_idx = my_index(ts[i].v_index, lv);
        if (mVertices[v_idx].n[3] == 0 && ln && ts[i].n_index) {
            mVertices[v_idx].n = mNormals[my_index(ts[i].n_index, ln)];
        }
        if (mVertices[v_idx].t[3] == 0 && lt && ts[i].t_index) {
            mVertices[v_idx].t = mTexCoords[my_index(ts[i].t_index, lt)];
        }
    }
    // append triangle indices, copy normals & texcoords
    for (int i = 2; i < ts.size(); ++i) {
        mIndices.append(my_index(ts[0].v_index, lv));
        mIndices.append(my_index(ts[i-1].v_index, lv));
        unsigned int v_idx = my_index(ts[i].v_index, lv);
        mIndices.append(v_idx);
        if (mVertices[v_idx].n[3] == 0 && ln && ts[i].n_index) {
            mVertices[v_idx].n = mNormals[my_index(ts[i].n_index, ln)];
        }
        if (mVertices[v_idx].t[3] == 0 && lt && ts[i].t_index) {
            mVertices[v_idx].t = mTexCoords[my_index(ts[i].t_index, lt)];
        }
    }
}


GL::ModelStore::~ModelStore() {
}

void GL::ModelStore::rename(const QString& from, const QString& to) {
    if (mModels.contains(from)) {
        Model model = mModels[from];
        mModels.remove(from);
        mModels[to] = model;
        mNames[mNames.indexOf(from)] = to;
    }
}

void GL::ModelStore::remove(int index, bool keepNames) {
    QString name = mNames[index];

    mSpecs.remove(name + ":vertex");
    mSpecs.remove(name + ":normal");
    mSpecs.remove(name + ":tex");

    Model model = mModels[name];

    unsigned int len = model.vertexLength;
    unsigned int off = model.vertexOffset;

    char* data = new char[mData[GL_ARRAY_BUFFER].length - len];
    ::memcpy(data, mData[GL_ARRAY_BUFFER].data, off);
    ::memcpy(data + off, mData[GL_ARRAY_BUFFER].data + off + len, mData[GL_ARRAY_BUFFER].length - off - len);

    delete mData[GL_ARRAY_BUFFER].data;
    mData[GL_ARRAY_BUFFER] = Data(data, mData[GL_ARRAY_BUFFER].length - len);


    len = model.elemLength;
    off = model.elemOffset;

    data = new char[mData[GL_ELEMENT_ARRAY_BUFFER].length - len];
    ::memcpy(data, mData[GL_ELEMENT_ARRAY_BUFFER].data, off);
    QVector<GLuint> indices;
    unsigned int rlen = mData[GL_ELEMENT_ARRAY_BUFFER].length - off - len;
    for (unsigned int b = 0; b < rlen; b += sizeof(GLuint)) {
        GLuint idx;
        ::memcpy(&idx, mData[GL_ELEMENT_ARRAY_BUFFER].data + off + len + b, sizeof(GLuint));
        indices.append(idx - model.vertexLength);
    }
    ::memcpy(data + off, indices.constData(), rlen);

    delete mData[GL_ELEMENT_ARRAY_BUFFER].data;
    mData[GL_ELEMENT_ARRAY_BUFFER] = Data(data, mData[GL_ELEMENT_ARRAY_BUFFER].length - len);

    mModels.remove(name);

    if (!keepNames) {
        mNames.removeAt(index);
        mFileNames.removeAt(index);
    }

    // fix offsets + specs of other models
    foreach (QString other, mNames) {
        if (mModels[other].vertexOffset > model.vertexOffset) {
            mModels[other].vertexOffset -= model.vertexLength;
            mModels[other].elemOffset -= model.elemLength;
            mSpecs[other + ":vertex"].offset -= model.vertexLength;
            mSpecs[other + ":normal"].offset -= model.vertexLength;
            mSpecs[other + ":tex"].offset -= model.vertexLength;
        }
    }
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


void GL::ModelStore::parseModelData(const QString& path) {
    QString inp(square);
    if (!path.isEmpty()) {
        QFile file(path);
        file.open(QFile::ReadOnly);
        inp = QString(file.readAll());
        file.close();
    }

    g_odebug = 0;
    s_o_scan_string(inp.toAscii().data());
    int err = g_oparse();
    s_olex_destroy();

    if (err) throw ModelError(QString(model_error_buffer), g_olloc.row, g_olloc.col, g_olloc.pos);
}


void GL::ModelStore::setModel(const QString& key, const QString& path) {
    try {
        parseModelData(path);
        if (mModels.contains(key)) {
            remove(mNames.indexOf(key), true);
        } else {
            mNames.append(key);
            mFileNames.append(path);
        }
        // model
        Model model;
        model.vertexOffset = mData[GL_ARRAY_BUFFER].length;
        model.vertexLength = mVertices.size() * sizeof(GLfloat) * 8;
        model.elemOffset = mData[GL_ELEMENT_ARRAY_BUFFER].length;
        model.elemLength = mIndices.size() * sizeof(GLuint);

        mModels[key] = model;

        // blob specs: num components, type, is normalized, packing offset, data offset
        unsigned int len = model.vertexLength;
        unsigned int off1 = model.vertexOffset;
        unsigned int off2 = model.vertexOffset + len / 8 * 3;
        unsigned int off3 = model.vertexOffset + len / 8 * 6;
        mSpecs[key + ":vertex"] = BlobSpec(3, GL_FLOAT, false, 0, off1);
        mSpecs[key + ":normal"] = BlobSpec(3, GL_FLOAT, true, 0, off2);
        mSpecs[key + ":tex"] = BlobSpec(2, GL_FLOAT, false, 0, off3);

        // data
        char* data = new char[off1 + len];
        ::memcpy(data, mData[GL_ARRAY_BUFFER].data, off1);

        unsigned int nsize = 3 * sizeof(GLfloat);
        unsigned int tsize = 2 * sizeof(GLfloat);
        for (int i = 0; i < mVertices.size(); ++i) {
            ::memcpy(data + off1 + i * nsize, mVertices[i].v.readArray(), nsize);
            ::memcpy(data + off2 + i * nsize, mVertices[i].n.readArray(), nsize);
            ::memcpy(data + off3 + i * tsize, mVertices[i].t.readArray(), tsize);
        }

        delete mData[GL_ARRAY_BUFFER].data;
        mData[GL_ARRAY_BUFFER] = Data(data, off1 + len);


        len = model.elemLength;
        off1 = model.elemOffset;

        data = new char[off1 + len];
        ::memcpy(data, mData[GL_ELEMENT_ARRAY_BUFFER].data, off1);
        QVector<GLuint> indices;
        foreach(GLuint index, mIndices) {
            indices.append(model.vertexOffset + index);
        }
        // qDebug() << indices;
        ::memcpy(data + off1, indices.constData(), len);

        delete mData[GL_ELEMENT_ARRAY_BUFFER].data;
        mData[GL_ELEMENT_ARRAY_BUFFER] = Data(data, off1 + len);

    } catch (ModelError& e) {
        qDebug() << e.msg() << e.row() << e.col();
    }
    mVertices.clear();
    mNormals.clear();
    mTexCoords.clear();
    mIndices.clear();
}



void GL::ModelStore::clean() {
    mNames.clear();
    mFileNames.clear();
    mModels.clear();

    delete mData[GL_ARRAY_BUFFER].data;
    mData[GL_ARRAY_BUFFER] = Data();

    delete mData[GL_ELEMENT_ARRAY_BUFFER].data;
    mData[GL_ELEMENT_ARRAY_BUFFER] = Data();

}

int GL::ModelStore::size() {
    return mNames.size();
}

const QString& GL::ModelStore::fileName(int index) {
    return mFileNames[index];
}

const QString& GL::ModelStore::modelName(int index) {
    return mNames[index];
}


Q_EXPORT_PLUGIN2(pnp_gl_modelstore, GL::ModelStore)
