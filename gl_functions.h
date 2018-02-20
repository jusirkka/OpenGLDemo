#ifndef GL_FUNCTIONS_H
#define GL_FUNCTIONS_H

#include "gl_widget.h"
#include "function.h"
#include "constant.h"
#include "blob.h"
#include "texblob.h"

#include <QVector>

using Math3D::X;
using Math3D::Y;
using Math3D::Z;
using Math3D::W;
using Math3D::Vector4;
using Math3D::Matrix4;


namespace Demo {
namespace GL {

class GLError {

public:
    GLError(QString msg)
        : emsg(std::move(msg)) {}

    const QString msg() const {return emsg;}

private:

    QString emsg;

};


class GLProc: public Demo::Function {
public:


#define ALT(item) case item: throw GLError(#item); break

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {

        const QVariant& q = gl_execute(vals, start);

        switch (glGetError()) {
        ALT(GL_INVALID_ENUM);
        ALT(GL_INVALID_VALUE);
        ALT(GL_INVALID_OPERATION);
        ALT(GL_STACK_UNDERFLOW);
        ALT(GL_STACK_OVERFLOW);
        ALT(GL_OUT_OF_MEMORY);
        ALT(GL_INVALID_FRAMEBUFFER_OPERATION);
        default: ;// nothing
        }

        return q;

    }

#undef ALT


protected:

    GLProc(const QString& name, Type* type, Demo::GLWidget* p)
        : Demo::Function(name, type),
          mParent(p)
    {}

    GLProc(const GLProc& f)
        : Function(f)
        , mParent(f.mParent) {}

    Demo::GLWidget* mParent;

private:

    virtual const QVariant& gl_execute(const QVector<QVariant>& vals, int start) = 0;

};

#define COPY_AND_CLONE(T) T(const T& f): GLProc(f) {} \
                          T* clone() const override {return new T(*this);}


class Enable: public GLProc {

public:

    Enable(Demo::GLWidget* p): GLProc("enable", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        // qDebug() << "glEnable" << vals[start].value<Math3D::Integer>();
        mParent->glEnable(vals[start].value<Math3D::Integer>());
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(Enable)
};

class Disable: public GLProc {

public:

    Disable(Demo::GLWidget* p): GLProc("disable", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        // qDebug() << "glDisable" << vals[start].value<Math3D::Integer>();
        mParent->glDisable(vals[start].value<Math3D::Integer>());
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(Disable)
};

class DepthRange: public GLProc {

public:

    DepthRange(Demo::GLWidget* p): GLProc("depthrange", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real near = vals[start].value<Math3D::Real>();
        Math3D::Real far = vals[start + 1].value<Math3D::Real>();
        // qDebug() << "glDepthRange" << near << far;
        mParent->glDepthRangef(near, far);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DepthRange)
};

class LineWidth: public GLProc {

public:

    LineWidth(Demo::GLWidget* p): GLProc("linewidth", new Integer_T, p) {
        mArgTypes.append(new Real_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real w = vals[start].value<Math3D::Real>();
        // qDebug() << "glLineWidth" << w;
        mParent->glLineWidth(w);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(LineWidth)
};

class FrontFace: public GLProc {

public:

    FrontFace(Demo::GLWidget* p): GLProc("frontface", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Integer face = vals[start].value<Math3D::Integer>();
        // qDebug() << "glFrontFace" << face;
        mParent->glFrontFace(face);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(FrontFace)
};

class CullFace: public GLProc {

public:

    CullFace(Demo::GLWidget* p): GLProc("cullface", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Integer face = vals[start].value<Math3D::Integer>();
        // qDebug() << "glCullFace" << face;
        mParent->glCullFace(face);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(CullFace)
};

class ColorMask: public GLProc {

public:

    ColorMask(Demo::GLWidget* p): GLProc("colormask", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Integer r = vals[start].value<Math3D::Integer>();
        Math3D::Integer g = vals[start+1].value<Math3D::Integer>();
        Math3D::Integer b = vals[start+2].value<Math3D::Integer>();
        Math3D::Integer a = vals[start+3].value<Math3D::Integer>();
        // qDebug() << "glColorMask" << r << g << b << a;
        mParent->glColorMask(r, g, b, a);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(ColorMask)
};

class DepthMask: public GLProc {

public:

    DepthMask(Demo::GLWidget* p): GLProc("depthmask", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Integer d = vals[start].value<Math3D::Integer>();
        // qDebug() << "glDepthMask" << d;
        mParent->glDepthMask(d);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DepthMask)
};

class Clear: public GLProc {

public:

    Clear(Demo::GLWidget* p): GLProc("clear", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Integer mask = vals[start].value<Math3D::Integer>();
        // qDebug() << "glClear" << mask;
        mParent->glClear(mask);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(Clear)
};

class ClearColor: public GLProc {

public:

    ClearColor(Demo::GLWidget* p): GLProc("clearcolor", new Integer_T, p) {
        mArgTypes.append(new Vector_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Vector4 color = vals[start].value<Vector4>();
        // qDebug() << "glClearColor" << color[X] << color[Y] << color[Z] << color[W];
        mParent->glClearColor(color[X], color[Y], color[Z], color[W]);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(ClearColor)
};

class ClearDepth: public GLProc {

public:

    ClearDepth(Demo::GLWidget* p): GLProc("cleardepth", new Integer_T, p) {
        mArgTypes.append(new Real_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real depth = vals[start].value<Math3D::Real>();
        // qDebug() << "glClearDepth" << depth;
        mParent->glClearDepthf(depth);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(ClearDepth)
};

class CreateShader: public GLProc {

public:

    CreateShader(Demo::GLWidget* p): GLProc("createshader", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int type = vals[start].value<int>();
        mValue.setValue(mParent->resource("shader", type));
        return mValue;
    }

    COPY_AND_CLONE(CreateShader)
};

class CompileShader: public GLProc {

public:

    CompileShader(Demo::GLWidget* p): GLProc("compileshader", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Text_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        QByteArray bytes = vals[start+1].toString().toLatin1();
        const char *data = bytes.constData();
        // qDebug() << "glShaderSource" << name;
        mParent->glShaderSource(name, 1, &data, nullptr);
        // qDebug() << "glCompileShader" << name;
        mParent->glCompileShader(name);
        int status;
        mParent->glGetShaderiv(name, GL_COMPILE_STATUS, &status);
        if (!status) {
            int len;
            mParent->glGetShaderiv(name, GL_INFO_LOG_LENGTH, &len);
            char info[len];
            mParent->glGetShaderInfoLog(name, len, &len, info);
            throw GLError(info);
        }
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(CompileShader)
};

class DeleteShader: public GLProc {

public:

    DeleteShader(Demo::GLWidget* p): GLProc("deleteshader", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int name = vals[start].value<int>();
        // qDebug() << "glDeleteShader" << name;
        if (!mParent->glIsShader(name)) {
            throw GLError(QString(R"("%1" is not a shader)").arg(name));
        }
        mParent->deresource("shader", name);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DeleteShader)
};


class CreateProgram: public GLProc {

public:

    CreateProgram(Demo::GLWidget* p): GLProc("createprogram", new Integer_T, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) override {
        // qDebug() << "glCreateProgram";
        mValue.setValue(mParent->resource("program"));
        return mValue;
    }

    COPY_AND_CLONE(CreateProgram)
};


class AttachShader: public GLProc {

public:

    AttachShader(Demo::GLWidget* p): GLProc("attachshader", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int prog = vals[start].value<int>();
        int shader = vals[start+1].value<int>();
        // qDebug() << "glAttachShader" << prog << shader;
        mParent->glAttachShader(prog, shader);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(AttachShader)
};


class DetachShader: public GLProc {

public:

    DetachShader(Demo::GLWidget* p): GLProc("detachshader", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int prog = vals[start].value<int>();
        int shader = vals[start+1].value<int>();
        // qDebug() << "glDetachShader" << prog << shader;
        mParent->glDetachShader(prog, shader);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DetachShader)
};


class LinkProgram: public GLProc {

public:

    LinkProgram(Demo::GLWidget* p): GLProc("linkprogram", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int name = vals[start].value<int>();
        // qDebug() << "glLinkProgram" << name;
        mParent->glLinkProgram(name);
        int status;
        mParent->glGetProgramiv(name, GL_LINK_STATUS, &status);
        if (!status) {
            int len;
            mParent->glGetProgramiv(name, GL_INFO_LOG_LENGTH, &len);
            char info[len];
            mParent->glGetProgramInfoLog(name, len, &len, info);
            throw GLError(info);
        }
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(LinkProgram)
};


class UseProgram: public GLProc {

public:

    UseProgram(Demo::GLWidget* p): GLProc("useprogram", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int name = vals[start].value<int>();
        // qDebug() << "glUseProgram" << name;
        mParent->glUseProgram(name);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(UseProgram)
};


class DeleteProgram: public GLProc {

public:

    DeleteProgram(Demo::GLWidget* p): GLProc("deleteprogram", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int name = vals[start].value<int>();
        // qDebug() << "glDeleteProgram" << name;
        if (!mParent->glIsProgram(name)) {
            throw GLError(QString(R"("%1" is not a program)").arg(name));
        }
        mParent->deresource("program", name);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DeleteProgram)
};

class GetAttribLocation: public GLProc {

public:

    GetAttribLocation(Demo::GLWidget* p): GLProc("getattriblocation", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Text_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int prog = vals[start].value<int>();
        QString name = vals[start+1].toString();
        // qDebug() << "glGetAttribLocation" << prog << name;
        QByteArray bytes = name.toLatin1();
        const char* data = bytes.constData();
        int loc = mParent->glGetAttribLocation(prog, data);
        mValue.setValue(loc);
        return mValue;
    }

    COPY_AND_CLONE(GetAttribLocation)
};


class GetUniformLocation: public GLProc {

public:

    GetUniformLocation(Demo::GLWidget* p): GLProc("getuniformlocation", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Text_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int prog = vals[start].value<int>();
        QString name = vals[start+1].toString();
        // qDebug() << "glGetUniformLocation" << prog << name;
        QByteArray bytes = name.toLatin1();
        const char* data = bytes.constData();
        int loc = mParent->glGetUniformLocation(prog, data);
        mValue.setValue(loc);
        return mValue;
    }

    COPY_AND_CLONE(GetUniformLocation)
};


class Uniform1F: public GLProc {

public:

    Uniform1F(Demo::GLWidget* p): GLProc("uniform1f", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Real_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int loc = vals[start].value<int>();
        Math3D::Real uniform = vals[start+1].value<Math3D::Real>();
        // qDebug() << "glUniform1f" << loc << uniform;
        mParent->glUniform1f(loc, uniform);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(Uniform1F)
};

class Uniform1I: public GLProc {

public:

    Uniform1I(Demo::GLWidget* p): GLProc("uniform1i", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int loc = vals[start].value<int>();
        int uniform = vals[start+1].value<int>();
        // qDebug() << "glUniform1i" << loc << uniform;
        mParent->glUniform1i(loc, uniform);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(Uniform1I)
};

class Uniform4F: public GLProc {

public:

    Uniform4F(Demo::GLWidget* p): GLProc("uniform4f", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Vector_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int loc = vals[start].value<int>();
        Vector4 uni = vals[start+1].value<Vector4>();
        // qDebug() << "glUniform4f" << loc << uni[X] << uni[Y] << uni[Z] << uni[W];
        mParent->glUniform4f(loc, uni[X], uni[Y], uni[Z], uni[W]);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(Uniform4F)
};


class UniformMatrix4F: public GLProc {

public:

    UniformMatrix4F(Demo::GLWidget* p): GLProc("uniformmatrix4f", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Matrix_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int loc = vals[start].value<int>();
        Matrix4 uni = vals[start+1].value<Matrix4>();
        // qDebug() << "glUniformMatrix4F" << loc;
        mParent->glUniformMatrix4fv(loc, 1, GL_FALSE, uni.readArray());
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(UniformMatrix4F)
};

class GenBuffer: public GLProc {

public:

    GenBuffer(Demo::GLWidget* p): GLProc("genbuffer", new Integer_T, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) override {
        mValue.setValue(mParent->resource("buffer"));
        return mValue;
    }

    COPY_AND_CLONE(GenBuffer)
};

class DeleteBuffer: public GLProc {

public:

    DeleteBuffer(Demo::GLWidget* p): GLProc("deletebuffer", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "glDeleteBuffers" << name;
        if (!mParent->glIsBuffer(name)) {
            throw GLError(QString(R"("%1" is not a buffer)").arg(name));
        }
        mParent->deresource("buffer", name);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DeleteBuffer)
};


class BindBuffer: public GLProc {

public:

    BindBuffer(Demo::GLWidget* p): GLProc("bindbuffer", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        GLuint buffer = vals[start+1].value<int>();
        // qDebug() << "glBindBuffer" << target << buffer;
        mParent->glBindBuffer(target, buffer);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(BindBuffer)
};

class BindBufferBase: public GLProc {

public:

    BindBufferBase(Demo::GLWidget* p): GLProc("bindbufferbase", new Integer_T, p) {
        mArgTypes.append(new Integer_T); // target
        mArgTypes.append(new Integer_T); // index
        mArgTypes.append(new Integer_T); // buffer
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLenum target = vals[start].value<int>();
        GLuint index = vals[start + 1].value<int>();
        GLuint buffer = vals[start + 2].value<int>();
        mParent->glBindBufferBase(target, index, buffer);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(BindBufferBase)
};

class Traversable {
public:

    Traversable() = default;

protected:
    void traverse(const QVariant& data) {
        int id = data.userType();
        if (id == Type::Real) {
            mData << data.value<Math3D::Real>();
        } else if (id == Type::Integer) {
            mData << static_cast<GLfloat>(data.value<Math3D::Integer>());
        } else if (id == Type::Vector) {
            auto v = data.value<Vector4>();
            for (int i = 0; i < 4; i++) mData << static_cast<GLfloat>(v[i]);
        } else if (id == Type::Matrix) {
            auto m = data.value<Matrix4>();
            const Math3D::Real* arr = m.readArray();
            for (int i = 0; i < 16; i++) mData << static_cast<GLfloat>(arr[i]);
        } else if (id == QMetaType::QVariantList) {
            for (auto v: data.toList()) traverse(v);
        } else {
            mData.clear();
            throw GLError("Unsupported data type");
        }
    }

protected:

    QVector<GLfloat> mData;
};

class BufferData: public GLProc, public Traversable {

public:

    BufferData(Demo::GLWidget* p): GLProc("bufferdata", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new NullType);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        traverse(vals[start+1]);
        GLuint usage = vals[start+2].value<int>();
        GLsizeiptr size = mData.size() * sizeof(GLfloat);
        mParent->glBufferData(target, size, mData.constData(), usage);
        mData.clear();
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(BufferData)

};

class BufferSubData: public GLProc, public Traversable {

public:

    BufferSubData(Demo::GLWidget* p): GLProc("buffersubdata", new Integer_T, p) {
        mArgTypes.append(new Integer_T); // target
        mArgTypes.append(new Integer_T); // offset
        mArgTypes.append(new NullType); // data
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        GLintptr offset = vals[start+1].value<int>() * sizeof(GLfloat);
        traverse(vals[start+2]);
        GLsizeiptr size = mData.size() * sizeof(GLfloat);
        mParent->glBufferSubData(target, offset, size, mData.constData());
        mData.clear();
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(BufferSubData)

private:

    void traverse(const QVariant& data) {
        int id = data.userType();
        if (id == Type::Real) {
            mData << data.value<Math3D::Real>();
        } else if (id == Type::Integer) {
            mData << static_cast<GLfloat>(data.value<Math3D::Integer>());
        } else if (id == Type::Vector) {
            auto v = data.value<Vector4>();
            for (int i = 0; i < 4; i++) mData << static_cast<GLfloat>(v[i]);
        } else if (id == Type::Matrix) {
            auto m = data.value<Matrix4>();
            const Math3D::Real* arr = m.readArray();
            for (int i = 0; i < 16; i++) mData << static_cast<GLfloat>(arr[i]);
        } else if (id == QMetaType::QVariantList) {
            for (auto v: data.toList()) traverse(v);
        } else {
            mData.clear();
            throw GLError("Unsupported data type");
        }
    }

private:

    QVector<GLfloat> mData;

};

class BufferExtData: public GLProc {

public:

    BufferExtData(Demo::GLWidget* p): GLProc("bufferextdata", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        const Blob& blob = mParent->blob(vals[start+1].value<int>());
        GLuint usage = vals[start+2].value<int>();
        // qDebug() << "glBufferData" << target << blob.name() << usage;
        mParent->glBufferData(target, blob.bytelen(target), blob.bytes(target), usage);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(BufferExtData)
};

class VertexAttribPointer: public GLProc {

public:

    VertexAttribPointer(Demo::GLWidget* p): GLProc("vertexattribpointer", new Integer_T, p) {
        mArgTypes.append(new Integer_T); // index
        mArgTypes.append(new Integer_T); // size
        mArgTypes.append(new Integer_T); // type
        mArgTypes.append(new Integer_T); // normalized
        mArgTypes.append(new Integer_T); // stride
        mArgTypes.append(new Integer_T); // offset
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint index = vals[start].value<int>();
        GLint size = vals[start + 1].value<int>();
        GLenum type = vals[start + 2].value<int>();
        GLboolean normalized = vals[start + 3].value<int>();
        GLsizei stride = vals[start + 4].value<int>() * sizeof(GLfloat);
        GLuint64 offset = vals[start + 5].value<int>() * sizeof(GLfloat);
        mParent->glVertexAttribPointer(index,
                                       size,
                                       type,
                                       normalized,
                                       stride,
                                       (const void*) offset);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(VertexAttribPointer)
};

class VertexAttribExtPointer: public GLProc {

public:

    VertexAttribExtPointer(Demo::GLWidget* p): GLProc("vertexattribextpointer", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Text_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint index = vals[start].value<int>();
        const Blob& blob = mParent->blob(vals[start+1].value<int>());
        QString attr = vals[start+2].toString();
        // qDebug() << "VertexAttribPointer" << index << blob.name() << attr;
        const BlobSpec& spec = blob.spec(attr);
        // qDebug() << "VertexAttribPointer" << spec.size << spec.offset;
        mParent->glVertexAttribPointer(index,
                                       spec.size,
                                       spec.type,
                                       spec.normalized,
                                       spec.stride,
                                       (const void*) spec.offset);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(VertexAttribExtPointer)
};

class VertexAttrib1i: public GLProc {

public:

    VertexAttrib1i(Demo::GLWidget* p): GLProc("vertexattrib1i", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint index = vals[start].toInt();
        GLint v0 = vals[start+1].toInt();
        mParent->glVertexAttribI1i(index, v0);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(VertexAttrib1i)
};

class VertexAttrib1f: public GLProc {

public:

    VertexAttrib1f(Demo::GLWidget* p): GLProc("vertexattrib1f", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Real_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint index = vals[start].toInt();
        GLfloat v0 = vals[start+1].toFloat();
        mParent->glVertexAttrib1f(index, v0);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(VertexAttrib1f)
};

class Draw: public GLProc {

public:

    Draw(Demo::GLWidget* p): GLProc("draw", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Text_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        const Blob& blob = mParent->blob(vals[start].value<int>());
        QString attr = vals[start + 1].toString();
        GLuint mode = vals[start + 2].value<int>();
        // qDebug() << "Draw" << blob.name() << attr << mode;
        blob.draw(mode, attr);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(Draw)
};

class DrawArrays: public GLProc {

public:

    DrawArrays(Demo::GLWidget* p): GLProc("drawarrays", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLenum mode = vals[start].value<int>();
        GLint first = vals[start + 1].value<int>();
        GLsizei count = vals[start + 2].value<int>();
        mParent->glDrawArrays(mode, first, count);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DrawArrays)
};

class EnableVertexAttribArray: public GLProc {

public:

    EnableVertexAttribArray(Demo::GLWidget* p): GLProc("enablevertexattribarray", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "glEnableVertexAttribArray" << name;
        mParent->glEnableVertexAttribArray(name);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(EnableVertexAttribArray)
};

class DisableVertexAttribArray: public GLProc {

public:

    DisableVertexAttribArray(Demo::GLWidget* p): GLProc("disablevertexattribarray", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "glDisableVertexAttribArray" << name;
        mParent->glDisableVertexAttribArray(name);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DisableVertexAttribArray)
};


class ActiveTexture: public GLProc {

public:

    ActiveTexture(Demo::GLWidget* p): GLProc("activetexture", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "ActiveTexture" << name;
        mParent->glActiveTexture(name);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(ActiveTexture)
};


class GenerateMipMap: public GLProc {

public:

    GenerateMipMap(Demo::GLWidget* p): GLProc("generatemipmap", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        // qDebug() << "GenerateMipMap" << target;
        mParent->glGenerateMipmap(target);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(GenerateMipMap)
};

class BindTexture: public GLProc {

public:

    BindTexture(Demo::GLWidget* p): GLProc("bindtexture", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        GLuint texture = vals[start + 1].value<int>();
        // qDebug() << "BindTexture" << target << texture;
        mParent->glBindTexture(target, texture);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(BindTexture)
};

class GenTexture: public GLProc {

public:

    GenTexture(Demo::GLWidget* p): GLProc("gentexture", new Integer_T, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) override {
        mValue.setValue(mParent->resource("texture"));
        return mValue;
    }

    COPY_AND_CLONE(GenTexture)
};

class DeleteTexture: public GLProc {

public:

    DeleteTexture(Demo::GLWidget* p): GLProc("deletetexture", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "DeleteTexture" << name;
        if (!mParent->glIsTexture(name)) {
            throw GLError(QString(R"("%1" is not a texture)").arg(name));
        }
        mParent->deresource("texture", name);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DeleteTexture)
};


class TexParameter: public GLProc {

public:

    TexParameter(Demo::GLWidget* p): GLProc("texparameter", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        GLuint name = vals[start + 1].value<int>();
        GLuint param = vals[start + 2].value<int>();
        // qDebug() << "TexParameter" << target << name << param;
        mParent->glTexParameteri(target, name, param);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(TexParameter)
};


class TexExtImage2D: public GLProc {

public:

    TexExtImage2D(Demo::GLWidget* p): GLProc("texextimage2d", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Text_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLenum target = vals[start].value<int>();
        GLint level = vals[start + 1].value<int>();
        GLint iformat = vals[start + 2].value<int>();
        const TexBlob& blob = mParent->texBlob(vals[start + 3].value<int>());
        QString attr = vals[start + 4].toString();
        // qDebug() << "TexImage2D" << target << level << iformat << blob.name() << attr;
        const TexBlobSpec spec = blob.spec(attr);
        // qDebug() << "TexImage2D" << spec.width << spec.height << spec.type;
        mParent->glTexImage2D(target, level, iformat,
                              spec.width, spec.height, 0, spec.format, spec.type, blob.readData(attr));
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(TexExtImage2D)
};

class TexEmptyImage2D: public GLProc {

public:

    TexEmptyImage2D(Demo::GLWidget* p): GLProc("texemptyimage2d", new Integer_T, p) {
        mArgTypes.append(new Integer_T); // target
        mArgTypes.append(new Integer_T); // level
        mArgTypes.append(new Integer_T); // internalFormat
        mArgTypes.append(new Integer_T); // width
        mArgTypes.append(new Integer_T); // height
        // skip border: always = 0
        mArgTypes.append(new Integer_T); // format
        mArgTypes.append(new Integer_T); // type
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLenum target = vals[start].value<int>();
        GLint level = vals[start + 1].value<int>();
        GLint iformat = vals[start + 2].value<int>();
        GLsizei w = vals[start + 3].value<int>();
        GLsizei h = vals[start + 4].value<int>();
        GLenum format = vals[start + 5].value<int>();
        GLenum type = vals[start + 6].value<int>();
        mParent->glTexImage2D(target, level, iformat, w, h, 0, format, type, (const GLvoid *) nullptr);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(TexEmptyImage2D)
};

class TexImage2D: public GLProc {

public:

    TexImage2D(Demo::GLWidget* p): GLProc("teximage2d", new Integer_T, p) {
        mArgTypes.append(new Integer_T); // target
        mArgTypes.append(new Integer_T); // level
        mArgTypes.append(new Integer_T); // internalFormat
        mArgTypes.append(new Integer_T); // width
        mArgTypes.append(new Integer_T); // height
        // skip border: always = 0
        mArgTypes.append(new Integer_T); // format
        mArgTypes.append(new Integer_T); // type
        mArgTypes.append(new ArrayType(new Real_T)); // data
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLenum target = vals[start].toInt();
        GLint level = vals[start + 1].toInt();
        GLint iformat = vals[start + 2].toInt();
        GLsizei w = vals[start + 3].toInt();
        GLsizei h = vals[start + 4].toInt();
        GLenum format = vals[start + 5].toInt();
        GLenum type = vals[start + 6].toInt();
        QVariantList list = vals[start + 7].toList();
        QVector<Math3D::Real> data;
        for (auto v: list) {
            data << v.value<Math3D::Real>();
        }
        switch (type) {
        case GL_UNSIGNED_BYTE:
        {
            GLubyte bytes[data.size()];
            for (int i = 0; i < data.size(); i++) bytes[i] = static_cast<GLubyte>(data[i]);
            mParent->glTexImage2D(target, level, iformat, w, h, 0, format, type, (const GLvoid*) bytes);
            break;
        }
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT:
        {
            GLushort shorts[data.size()];
            for (int i = 0; i < data.size(); i++) shorts[i] = static_cast<GLushort>(data[i]);
            mParent->glTexImage2D(target, level, iformat, w, h, 0, format, type, (const GLvoid*) shorts);
            break;
        }
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_24_8:
        {
            GLuint ints[data.size()];
            for (int i = 0; i < data.size(); i++) ints[i] = static_cast<GLuint>(data[i]);
            mParent->glTexImage2D(target, level, iformat, w, h, 0, format, type, (const GLvoid*) ints);
            break;
        }
        case GL_FLOAT:
        {
            GLfloat floats[data.size()];
            for (int i = 0; i < data.size(); i++) floats[i] = static_cast<GLfloat>(data[i]);
            mParent->glTexImage2D(target, level, iformat, w, h, 0, format, type, (const GLvoid*) floats);
            break;
        }
        default:
            throw GLError("Unsupported image data type");
        }
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(TexImage2D)
};


class TexSubImage1D: public GLProc {

public:

    TexSubImage1D(Demo::GLWidget* p): GLProc("texsubimage1d", new Integer_T, p) {
        mArgTypes.append(new Integer_T); // target
        mArgTypes.append(new Integer_T); // level
        mArgTypes.append(new Integer_T); // xoffset
        mArgTypes.append(new Integer_T); // width
        mArgTypes.append(new Integer_T); // format
        mArgTypes.append(new Integer_T); // type
        mArgTypes.append(new ArrayType(new Real_T)); // pixels
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLenum target = vals[start].toInt();
        GLint level = vals[start + 1].toInt();
        GLint xoffset = vals[start + 2].toInt();
        GLsizei w = vals[start + 3].toInt();
        GLenum format = vals[start + 4].toInt();
        GLenum type = vals[start + 5].toInt();
        QVariantList list = vals[start + 6].toList();
        QVector<Math3D::Real> data;
        for (auto v: list) {
            data << v.value<Math3D::Real>();
        }
        switch (type) {
        case GL_UNSIGNED_BYTE:
        {
            GLubyte bytes[data.size()];
            for (int i = 0; i < data.size(); i++) bytes[i] = static_cast<GLubyte>(data[i]);
            mParent->glTexSubImage1D(target, level, xoffset, w, format, type, (const GLvoid*) bytes);
            break;
        }
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT:
        {
            GLushort shorts[data.size()];
            for (int i = 0; i < data.size(); i++) shorts[i] = static_cast<GLushort>(data[i]);
            mParent->glTexSubImage1D(target, level, xoffset, w, format, type, (const GLvoid*) shorts);
            break;
        }
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_24_8:
        {
            GLuint ints[data.size()];
            for (int i = 0; i < data.size(); i++) ints[i] = static_cast<GLuint>(data[i]);
            mParent->glTexSubImage1D(target, level, xoffset, w, format, type, (const GLvoid*) ints);
            break;
        }
        case GL_FLOAT:
        {
            GLfloat floats[data.size()];
            for (int i = 0; i < data.size(); i++) floats[i] = static_cast<GLfloat>(data[i]);
            mParent->glTexSubImage1D(target, level, xoffset, w, format, type, (const GLvoid*) floats);
            break;
        }
        default:
            throw GLError("Unsupported image data type");
        }
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(TexSubImage1D)
};

class TexStorage1D: public GLProc {

public:

    TexStorage1D(Demo::GLWidget* p): GLProc("texstorage1d", new Integer_T, p) {
        mArgTypes.append(new Integer_T); // target
        mArgTypes.append(new Integer_T); // levels
        mArgTypes.append(new Integer_T); // internal format
        mArgTypes.append(new Integer_T); // width
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLenum target = vals[start].toInt();
        GLsizei levels = vals[start + 1].toInt();
        GLenum iformat = vals[start + 2].toInt();
        GLsizei w = vals[start + 3].toInt();
        mParent->glTexStorage1D(target, levels, iformat, w);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(TexStorage1D)
};

class Viewport: public GLProc {

public:

    Viewport(Demo::GLWidget* p): GLProc("viewport", new Integer_T, p) {
        mArgTypes.append(new Integer_T); // x
        mArgTypes.append(new Integer_T); // y
        mArgTypes.append(new Integer_T); // width
        mArgTypes.append(new Integer_T); // height
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLint x = vals[start].value<int>();
        GLint y = vals[start + 1].value<int>();
        GLsizei w = vals[start + 2].value<int>();
        GLsizei h = vals[start + 3].value<int>();
        mParent->glViewport(x, y, w, h);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(Viewport)
};

class BlendFunc: public GLProc {

public:

    BlendFunc(Demo::GLWidget* p): GLProc("blendfunc", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint src = vals[start].value<int>();
        GLuint dst = vals[start + 1].value<int>();
        mParent->glBlendFunc(src, dst);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(BlendFunc)
;
};


class BlendEquation: public GLProc {

public:

    BlendEquation(Demo::GLWidget* p): GLProc("blendequation", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint mode = vals[start].value<int>();
        mParent->glBlendEquation(mode);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(BlendEquation)
};

class BlendColor: public GLProc {

public:

    BlendColor(Demo::GLWidget* p): GLProc("blendcolor", new Integer_T, p) {
        mArgTypes.append(new Vector_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Vector4 c = vals[start].value<Vector4>();
        mParent->glBlendColor(c[X], c[Y], c[Z], c[W]);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(BlendColor)
};

class PolygonOffset: public GLProc {

public:

    PolygonOffset(Demo::GLWidget* p): GLProc("polygonoffset", new Integer_T, p) {
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Real_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real factor = vals[start].value<Math3D::Real>();
        Math3D::Real units = vals[start + 1].value<Math3D::Real>();
        // qDebug() << "glPolygonOffset" << depth;
        mParent->glPolygonOffset(factor, units);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(PolygonOffset)
};

class DepthFunc: public GLProc {

public:

    DepthFunc(Demo::GLWidget* p): GLProc("depthfunc", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint func = vals[start].value<int>();
        mParent->glDepthFunc(func);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DepthFunc)
};


class StencilFunc: public GLProc {

public:

    StencilFunc(Demo::GLWidget* p): GLProc("stencilfunc", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint func = vals[start].value<int>();
        GLint ref = vals[start+1].value<int>();
        GLuint mask = vals[start+2].value<int>();
        mParent->glStencilFunc(func, ref, mask);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(StencilFunc)
};

class StencilOp: public GLProc {

public:

    StencilOp(Demo::GLWidget* p): GLProc("stencilop", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint sfail = vals[start].value<int>();
        GLuint dpfail = vals[start+1].value<int>();
        GLuint dppass = vals[start+2].value<int>();
        mParent->glStencilOp(sfail, dpfail, dppass);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(StencilOp)
};

class GenFrameBuffer: public GLProc {

public:

    GenFrameBuffer(Demo::GLWidget* p): GLProc("genframebuffer", new Integer_T, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) override {
        GLuint res = mParent->resource("frame_buffer");
        // qDebug() << "genframebuffer" << res;
        mValue.setValue(res);
        return mValue;
    }

    COPY_AND_CLONE(GenFrameBuffer)
};

class DeleteFrameBuffer: public GLProc {

public:

    DeleteFrameBuffer(Demo::GLWidget* p): GLProc("deleteframebuffer", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "glDeleteFrameBuffers" << name;
        if (!mParent->glIsFramebuffer(name)) {
            throw GLError(QString(R"("%1" is not a frame buffer)").arg(name));
        }
        mParent->deresource("frame_buffer", name);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DeleteFrameBuffer)
};


class BindFrameBuffer: public GLProc {

public:

    BindFrameBuffer(Demo::GLWidget* p): GLProc("bindframebuffer", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        GLuint buffer = vals[start+1].value<int>();
        // qDebug() << "glBindFrameBuffer" << target << buffer;
        mParent->glBindFramebuffer(target, buffer);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(BindFrameBuffer)
};


class FrameBufferTexture2D: public GLProc {

public:

    FrameBufferTexture2D(Demo::GLWidget* p): GLProc("framebuffertexture2d", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        GLuint attachment = vals[start + 1].value<int>();
        GLuint textarget = vals[start + 2].value<int>();
        GLuint texture = vals[start + 3].value<int>();
        GLint level = vals[start + 4].value<int>();
        // qDebug() << "glFramebufferTexture2D" << target << name << param;
        mParent->glFramebufferTexture2D(target, attachment, textarget, texture, level);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(FrameBufferTexture2D)
};

class FrameBufferTextureLayer: public GLProc {

public:

    FrameBufferTextureLayer(Demo::GLWidget* p): GLProc("framebuffertexturelayer", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        GLuint attachment = vals[start + 1].value<int>();
        GLuint texture = vals[start + 2].value<int>();
        GLint level = vals[start + 3].value<int>();
        GLuint layer = vals[start + 4].value<int>();
        // qDebug() << "glFramebufferTextureLayer" << target << name << param;
        mParent->glFramebufferTextureLayer(target, attachment, texture, level, layer);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(FrameBufferTextureLayer)
};

class CheckFrameBufferStatus: public GLProc {

public:

    CheckFrameBufferStatus(Demo::GLWidget* p): GLProc("checkframebufferstatus", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        // qDebug() << "glCheckFramebufferStatus" << target;
        GLuint status = mParent->glCheckFramebufferStatus(target);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            throw GLError(QString("Framebuffer is not complete (%1)").arg(status));
        }
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(CheckFrameBufferStatus)
};

class DrawBuffer: public GLProc {

public:

    DrawBuffer(Demo::GLWidget* p): GLProc("drawbuffer", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint buf = vals[start].value<int>();
        // qDebug() << "glDrawBuffer" << target;
        mParent->glDrawBuffers(1, &buf);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DrawBuffer)
};

class BindVertexArray: public GLProc {

public:

    BindVertexArray(Demo::GLWidget* p): GLProc("bindvertexarray", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        mParent->glBindVertexArray(target);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(BindVertexArray)
};

class GenVertexArray: public GLProc {

public:

    GenVertexArray(Demo::GLWidget* p): GLProc("genvertexarray", new Integer_T, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) override {
        mValue.setValue(mParent->resource("vertex_array"));
        return mValue;
    }

    COPY_AND_CLONE(GenVertexArray)
};

class DeleteVertexArray: public GLProc {

public:

    DeleteVertexArray(Demo::GLWidget* p): GLProc("deletevertexarray", new Integer_T, p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        if (!mParent->glIsVertexArray(name)) {
            throw GLError(QString(R"("%1" is not a vertex array)").arg(name));
        }
        mParent->deresource("vertex_array", name);
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(DeleteVertexArray)
};


class Functions {

public:

    QVector<Demo::Symbol*> contents;

    Functions(Demo::GLWidget* p) {
        contents.append(new Enable(p));
        contents.append(new Disable(p));
        contents.append(new DepthRange(p));
        contents.append(new LineWidth(p));
        contents.append(new FrontFace(p));
        contents.append(new CullFace(p));
        contents.append(new ColorMask(p));
        contents.append(new DepthMask(p));
        contents.append(new Clear(p));
        contents.append(new ClearColor(p));
        contents.append(new ClearDepth(p));
        contents.append(new CreateShader(p));
        contents.append(new CompileShader(p));
        contents.append(new DeleteShader(p));
        contents.append(new CreateProgram(p));
        contents.append(new AttachShader(p));
        contents.append(new DetachShader(p));
        contents.append(new LinkProgram(p));
        contents.append(new UseProgram(p));
        contents.append(new DeleteProgram(p));
        contents.append(new GetAttribLocation(p));
        contents.append(new GetUniformLocation(p));
        contents.append(new Uniform1F(p));
        contents.append(new Uniform1I(p));
        contents.append(new Uniform4F(p));
        contents.append(new UniformMatrix4F(p));
        contents.append(new GenBuffer(p));
        contents.append(new DeleteBuffer(p));
        contents.append(new BindBuffer(p));
        contents.append(new BindBufferBase(p));
        contents.append(new BufferData(p));
        contents.append(new BufferSubData(p));
        contents.append(new BufferExtData(p));
        contents.append(new VertexAttribPointer(p));
        contents.append(new VertexAttribExtPointer(p));
        contents.append(new VertexAttrib1f(p));
        contents.append(new VertexAttrib1i(p));
        contents.append(new Draw(p));
        contents.append(new DrawArrays(p));
        contents.append(new EnableVertexAttribArray(p));
        contents.append(new DisableVertexAttribArray(p));
        contents.append(new ActiveTexture(p));
        contents.append(new GenerateMipMap(p));
        contents.append(new BindTexture(p));
        contents.append(new GenTexture(p));
        contents.append(new DeleteTexture(p));
        contents.append(new TexParameter(p));
        contents.append(new TexImage2D(p));
        contents.append(new TexEmptyImage2D(p));
        contents.append(new TexExtImage2D(p));
        contents.append(new TexSubImage1D(p));
        contents.append(new TexStorage1D(p));
        contents.append(new Viewport(p));
        contents.append(new BlendFunc(p));
        contents.append(new BlendEquation(p));
        contents.append(new BlendColor(p));
        contents.append(new PolygonOffset(p));
        contents.append(new DepthFunc(p));
        contents.append(new StencilFunc(p));
        contents.append(new StencilOp(p));
        contents.append(new GenFrameBuffer(p));
        contents.append(new BindFrameBuffer(p));
        contents.append(new DeleteFrameBuffer(p));
        contents.append(new FrameBufferTexture2D(p));
        contents.append(new FrameBufferTextureLayer(p));
        contents.append(new CheckFrameBufferStatus(p));
        contents.append(new DrawBuffer(p));
        contents.append(new GenVertexArray(p));
        contents.append(new BindVertexArray(p));
        contents.append(new DeleteVertexArray(p));
    }
};

#define CONST(value) contents.append(new Demo::Constant(QString(#value).toLower(), GL_ ## value))

class Constants {

public:

    QVector<Demo::Symbol*> contents;

    Constants() {
        // enable / disable
        CONST(BLEND);
        CONST(CULL_FACE);
        CONST(DEPTH_TEST);
        CONST(DITHER);
        CONST(POLYGON_OFFSET_FILL);
        CONST(SAMPLE_ALPHA_TO_COVERAGE);
        CONST(SAMPLE_COVERAGE);
        CONST(SCISSOR_TEST);
        CONST(STENCIL_TEST);
        CONST(PROGRAM_POINT_SIZE);
        // frontface
        CONST(CCW);
        CONST(CW);
        // cullface
        CONST(FRONT);
        CONST(BACK);
        CONST(FRONT_AND_BACK);
        // clear
        CONST(COLOR_BUFFER_BIT);
        CONST(DEPTH_BUFFER_BIT);
        CONST(STENCIL_BUFFER_BIT);
        // createshader
        CONST(VERTEX_SHADER);
        CONST(FRAGMENT_SHADER);
        CONST(GEOMETRY_SHADER);
        // bindbuffer
        CONST(ARRAY_BUFFER);
        CONST(ELEMENT_ARRAY_BUFFER);
        CONST(UNIFORM_BUFFER);
        // bufferdata
        CONST(STATIC_DRAW);
        CONST(STREAM_DRAW);
        CONST(DYNAMIC_DRAW);
        // draw & draw arrays
        CONST(POINTS);
        CONST(LINES);
        CONST(TRIANGLES);
        CONST(LINE_STRIP);
        CONST(LINE_LOOP);
        CONST(TRIANGLE_STRIP);
        CONST(TRIANGLE_FAN);
        // active texture
        CONST(TEXTURE0);
        // texture targets
        CONST(TEXTURE_1D);
        CONST(TEXTURE_2D);
        CONST(TEXTURE_2D_MULTISAMPLE);
        CONST(TEXTURE_2D_ARRAY);
        CONST(TEXTURE_CUBE_MAP);
        CONST(TEXTURE_CUBE_MAP_POSITIVE_X);
        CONST(TEXTURE_CUBE_MAP_POSITIVE_Y);
        CONST(TEXTURE_CUBE_MAP_POSITIVE_Z);
        CONST(TEXTURE_CUBE_MAP_NEGATIVE_X);
        CONST(TEXTURE_CUBE_MAP_NEGATIVE_Y);
        CONST(TEXTURE_CUBE_MAP_NEGATIVE_Z);
        // texture params & values
        CONST(TEXTURE_WRAP_S);
        CONST(TEXTURE_WRAP_T);
        CONST(TEXTURE_MIN_FILTER);
        CONST(TEXTURE_MAG_FILTER);
        CONST(NEAREST);
        CONST(LINEAR);
        CONST(NEAREST_MIPMAP_LINEAR);
        CONST(NEAREST_MIPMAP_NEAREST);
        CONST(LINEAR_MIPMAP_LINEAR);
        CONST(LINEAR_MIPMAP_NEAREST);
        CONST(NEAREST_MIPMAP_LINEAR);
        CONST(CLAMP_TO_EDGE);
        CONST(MIRRORED_REPEAT);
        CONST(REPEAT);
        // texture formats & types
        CONST(ALPHA);
        CONST(LUMINANCE);
        CONST(LUMINANCE_ALPHA);
        CONST(RGB);
        CONST(RGBA);
        CONST(UNSIGNED_BYTE);
        CONST(UNSIGNED_SHORT_5_6_5);
        CONST(UNSIGNED_SHORT_4_4_4_4);
        CONST(UNSIGNED_SHORT_5_5_5_1);
        CONST(UNSIGNED_SHORT);
        CONST(UNSIGNED_INT);
        CONST(UNSIGNED_INT_24_8);
        CONST(FLOAT);
        CONST(FLOAT_32_UNSIGNED_INT_24_8_REV);
        CONST(DEPTH_COMPONENT);
        CONST(DEPTH_COMPONENT16);
        CONST(DEPTH_COMPONENT24);
        CONST(DEPTH_COMPONENT32F);
        CONST(DEPTH24_STENCIL8);
        CONST(DEPTH32F_STENCIL8);
        // blend func
        CONST(CONSTANT_COLOR);
        CONST(DST_COLOR);
        CONST(SRC_COLOR);
        CONST(CONSTANT_ALPHA);
        CONST(DST_ALPHA);
        CONST(SRC_ALPHA);
        CONST(ONE);
        CONST(ONE_MINUS_SRC_ALPHA);
        CONST(ONE_MINUS_SRC_COLOR);
        CONST(ONE_MINUS_CONSTANT_ALPHA);
        CONST(ONE_MINUS_CONSTANT_COLOR);
        CONST(ONE_MINUS_DST_ALPHA);
        CONST(ONE_MINUS_DST_COLOR);
        CONST(SRC_ALPHA_SATURATE);
        CONST(ZERO);
        CONST(SRC1_COLOR);
        CONST(ONE_MINUS_SRC1_COLOR);
        CONST(SRC1_ALPHA);
        CONST(ONE_MINUS_SRC1_ALPHA);
        // blend equation
        CONST(MAX);
        CONST(MIN);
        CONST(FUNC_ADD);
        CONST(FUNC_SUBTRACT);
        CONST(FUNC_REVERSE_SUBTRACT);
        // stencil & depth funcs
        CONST(ALWAYS);
        CONST(EQUAL);
        CONST(GEQUAL);
        CONST(GREATER);
        CONST(LEQUAL);
        CONST(LESS);
        CONST(NEVER);
        CONST(NOTEQUAL);
        // stencil op
        CONST(DECR);
        CONST(DECR_WRAP);
        CONST(INCR);
        CONST(INCR_WRAP);
        CONST(INVERT);
        CONST(KEEP);
        CONST(REPLACE);
        // ZERO already defined (blend func);
        // bind framebuffer
        CONST(DRAW_FRAMEBUFFER);
        CONST(READ_FRAMEBUFFER);
        CONST(FRAMEBUFFER);
        // framebuffer texture2D
        CONST(COLOR_ATTACHMENT0);
        CONST(COLOR_ATTACHMENT1);
        CONST(COLOR_ATTACHMENT2);
        CONST(COLOR_ATTACHMENT3);
        CONST(STENCIL_ATTACHMENT);
        CONST(DEPTH_ATTACHMENT);
        CONST(DEPTH_STENCIL_ATTACHMENT);
        // drawbuffer
        CONST(NONE);
        // Image data internal formats
        CONST(R8);
        CONST(R16);
        CONST(RG8);
        CONST(RG16);
        CONST(R3_G3_B2);
        CONST(RGB4);
        CONST(RGB5);
        CONST(RGB8);
        CONST(RGB10);
        CONST(RGB12);
        CONST(RGBA2);
        CONST(RGBA4);
        CONST(RGB5_A1);
        CONST(RGBA8);
        CONST(RGB10_A2);
        CONST(RGBA12);
        CONST(RGBA16);
        CONST(SRGB8);
        CONST(SRGB8_ALPHA8);
        CONST(R16F);
        CONST(RG16F);
        CONST(RGB16F);
        CONST(RGBA16F);
        CONST(R32F);
        CONST(RG32F);
        CONST(RGB32F);
        CONST(RGBA32F);
        CONST(R11F_G11F_B10F);
        CONST(RGB9_E5);
        CONST(R8I);
        CONST(R8UI);
        CONST(R16I);
        CONST(R16UI);
        CONST(R32I);
        CONST(R32UI);
        CONST(RG8I);
        CONST(RG8UI);
        CONST(RG16I);
        CONST(RG16UI);
        CONST(RG32I);
        CONST(RG32UI);
        CONST(RGB8I);
        CONST(RGB8UI);
        CONST(RGB16I);
        CONST(RGB16UI);
        CONST(RGB32I);
        CONST(RGB32UI);
        CONST(RGBA8I);
        CONST(RGBA8UI);
        CONST(RGBA16I);
        CONST(RGBA16UI);
        CONST(RGBA32I);
        CONST(RGBA32UI);
    }


};

#undef CONST
}}
#endif // GL_FUNCTIONS_H
