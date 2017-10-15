#ifndef GL_FUNCTIONS_H
#define GL_FUNCTIONS_H

#include "gl_widget.h"
#include "function.h"
#include "constant.h"
#include "blob.h"
#include "texblob.h"

#include <QList>

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

    GLProc(const QString& name, int type, Demo::GLWidget* p)
        : Demo::Function(name, type),
          mParent(p)
    {}

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

    Demo::GLWidget* mParent;

private:

    virtual const QVariant& gl_execute(const QVector<QVariant>& vals, int start) = 0;

};


class Enable: public GLProc {

public:

    Enable(Demo::GLWidget* p): GLProc("enable", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        // qDebug() << "glEnable" << vals[start].value<Math3D::Integer>();
        mParent->glEnable(vals[start].value<Math3D::Integer>());
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Enable)
};

class Disable: public GLProc {

public:

    Disable(Demo::GLWidget* p): GLProc("disable", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        // qDebug() << "glDisable" << vals[start].value<Math3D::Integer>();
        mParent->glDisable(vals[start].value<Math3D::Integer>());
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Disable)
};

class DepthRange: public GLProc {

public:

    DepthRange(Demo::GLWidget* p): GLProc("depthrange", Symbol::Integer, p) {
        int argt = Symbol::Real;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real near = vals[start].value<Math3D::Real>();
        Math3D::Real far = vals[start + 1].value<Math3D::Real>();
        // qDebug() << "glDepthRange" << near << far;
        mParent->glDepthRange(near, far);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DepthRange)
};

class LineWidth: public GLProc {

public:

    LineWidth(Demo::GLWidget* p): GLProc("linewidth", Symbol::Integer, p) {
        int argt = Symbol::Real;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real w = vals[start].value<Math3D::Real>();
        // qDebug() << "glLineWidth" << w;
        mParent->glLineWidth(w);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(LineWidth)
};

class FrontFace: public GLProc {

public:

    FrontFace(Demo::GLWidget* p): GLProc("frontface", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Integer face = vals[start].value<Math3D::Integer>();
        // qDebug() << "glFrontFace" << face;
        mParent->glFrontFace(face);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(FrontFace)
};

class CullFace: public GLProc {

public:

    CullFace(Demo::GLWidget* p): GLProc("cullface", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Integer face = vals[start].value<Math3D::Integer>();
        // qDebug() << "glCullFace" << face;
        mParent->glCullFace(face);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(CullFace)
};

class ColorMask: public GLProc {

public:

    ColorMask(Demo::GLWidget* p): GLProc("colormask", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
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

    CLONEMETHOD(ColorMask)
};

class DepthMask: public GLProc {

public:

    DepthMask(Demo::GLWidget* p): GLProc("depthmask", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Integer d = vals[start].value<Math3D::Integer>();
        // qDebug() << "glDepthMask" << d;
        mParent->glDepthMask(d);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DepthMask)
};

class Clear: public GLProc {

public:

    Clear(Demo::GLWidget* p): GLProc("clear", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Integer mask = vals[start].value<Math3D::Integer>();
        // qDebug() << "glClear" << mask;
        mParent->glClear(mask);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Clear)
};

class ClearColor: public GLProc {

public:

    ClearColor(Demo::GLWidget* p): GLProc("clearcolor", Symbol::Integer, p) {
        int argt = Symbol::Vector;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Vector4 color = vals[start].value<Vector4>();
        // qDebug() << "glClearColor" << color[X] << color[Y] << color[Z] << color[W];
        mParent->glClearColor(color[X], color[Y], color[Z], color[W]);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(ClearColor)
};

class ClearDepth: public GLProc {

public:

    ClearDepth(Demo::GLWidget* p): GLProc("cleardepth", Symbol::Integer, p) {
        int argt = Symbol::Real;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real depth = vals[start].value<Math3D::Real>();
        // qDebug() << "glClearDepth" << depth;
        mParent->glClearDepth(depth);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(ClearDepth)
};

class CreateShader: public GLProc {

public:

    CreateShader(Demo::GLWidget* p): GLProc("createshader", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int type = vals[start].value<int>();
        // qDebug() << "glCreateShader" << type;
        int ret = mParent->glCreateShader(type);
        // qDebug() << "shader" << ret;
        mParent->resources().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

    CLONEMETHOD(CreateShader)
};

class CompileShader: public GLProc {

public:

    CompileShader(Demo::GLWidget* p): GLProc("compileshader", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        argt = Symbol::Text;
        mArgTypes.append(argt);
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

    CLONEMETHOD(CompileShader)
};

class DeleteShader: public GLProc {

public:

    DeleteShader(Demo::GLWidget* p): GLProc("deleteshader", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int name = vals[start].value<int>();
        // qDebug() << "glDeleteShader" << name;
        if (!mParent->glIsShader(name)) {
            throw GLError(QString(R"("%1" is not a shader)").arg(name));
        }
        mParent->glDeleteShader(name);
        // qDebug() << "delete shader" << name;
        mParent->resources().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DeleteShader)
};


class CreateProgram: public GLProc {

public:

    CreateProgram(Demo::GLWidget* p): GLProc("createprogram", Symbol::Integer, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) override {
        // qDebug() << "glCreateProgram";
        int ret = mParent->glCreateProgram();
        // qDebug() << "program" << ret;
        mParent->resources().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

    CLONEMETHOD(CreateProgram)
};


class AttachShader: public GLProc {

public:

    AttachShader(Demo::GLWidget* p): GLProc("attachshader", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int prog = vals[start].value<int>();
        int shader = vals[start+1].value<int>();
        // qDebug() << "glAttachShader" << prog << shader;
        mParent->glAttachShader(prog, shader);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(AttachShader)
};


class DetachShader: public GLProc {

public:

    DetachShader(Demo::GLWidget* p): GLProc("detachshader", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int prog = vals[start].value<int>();
        int shader = vals[start+1].value<int>();
        // qDebug() << "glDetachShader" << prog << shader;
        mParent->glDetachShader(prog, shader);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DetachShader)
};


class LinkProgram: public GLProc {

public:

    LinkProgram(Demo::GLWidget* p): GLProc("linkprogram", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
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

    CLONEMETHOD(LinkProgram)
};


class UseProgram: public GLProc {

public:

    UseProgram(Demo::GLWidget* p): GLProc("useprogram", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int name = vals[start].value<int>();
        // qDebug() << "glUseProgram" << name;
        mParent->glUseProgram(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(UseProgram)
};


class DeleteProgram: public GLProc {

public:

    DeleteProgram(Demo::GLWidget* p): GLProc("deleteprogram", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int name = vals[start].value<int>();
        // qDebug() << "glDeleteProgram" << name;
        if (!mParent->glIsProgram(name)) {
            throw GLError(QString(R"("%1" is not a program)").arg(name));
        }
        mParent->glDeleteProgram(name);
        // qDebug() << "delete program" << name;
        mParent->resources().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DeleteProgram)
};

class GetAttribLocation: public GLProc {

public:

    GetAttribLocation(Demo::GLWidget* p): GLProc("getattriblocation", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        t = Symbol::Text;
        mArgTypes.append(t);
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

    CLONEMETHOD(GetAttribLocation)
};


class GetUniformLocation: public GLProc {

public:

    GetUniformLocation(Demo::GLWidget* p): GLProc("getuniformlocation", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        t = Symbol::Text;
        mArgTypes.append(t);
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

    CLONEMETHOD(GetUniformLocation)
};


class Uniform1F: public GLProc {

public:

    Uniform1F(Demo::GLWidget* p): GLProc("uniform1f", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        t = Symbol::Real;
        mArgTypes.append(t);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int loc = vals[start].value<int>();
        Math3D::Real uniform = vals[start+1].value<Math3D::Real>();
        // qDebug() << "glUniform1f" << loc << uniform;
        mParent->glUniform1f(loc, uniform);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Uniform1F)
};

class Uniform1I: public GLProc {

public:

    Uniform1I(Demo::GLWidget* p): GLProc("uniform1i", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        mArgTypes.append(t);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int loc = vals[start].value<int>();
        int uniform = vals[start+1].value<int>();
        // qDebug() << "glUniform1i" << loc << uniform;
        mParent->glUniform1i(loc, uniform);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Uniform1I)
};

class Uniform4F: public GLProc {

public:

    Uniform4F(Demo::GLWidget* p): GLProc("uniform4f", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        t = Symbol::Vector;
        mArgTypes.append(t);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int loc = vals[start].value<int>();
        Vector4 uni = vals[start+1].value<Vector4>();
        // qDebug() << "glUniform4f" << loc << uni[X] << uni[Y] << uni[Z] << uni[W];
        mParent->glUniform4f(loc, uni[X], uni[Y], uni[Z], uni[W]);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Uniform4F)
};


class UniformMatrix4F: public GLProc {

public:

    UniformMatrix4F(Demo::GLWidget* p): GLProc("uniformmatrix4f", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        t = Symbol::Matrix;
        mArgTypes.append(t);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        int loc = vals[start].value<int>();
        Matrix4 uni = vals[start+1].value<Matrix4>();
        // qDebug() << "glUniformMatrix4F" << loc;
        mParent->glUniformMatrix4fv(loc, 1, GL_FALSE, uni.readArray());
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(UniformMatrix4F)
};

class GenBuffer: public GLProc {

public:

    GenBuffer(Demo::GLWidget* p): GLProc("genbuffer", Symbol::Integer, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) override {
        GLuint ret;
        // qDebug() << "glGenBuffers";
        mParent->glGenBuffers(1, &ret);
        // qDebug() << "buffer" << ret;
        mParent->resources().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

    CLONEMETHOD(GenBuffer)
};

class DeleteBuffer: public GLProc {

public:

    DeleteBuffer(Demo::GLWidget* p): GLProc("deletebuffer", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "glDeleteBuffers" << name;
        if (!mParent->glIsBuffer(name)) {
            throw GLError(QString(R"("%1" is not a buffer)").arg(name));
        }
        mParent->glDeleteBuffers(1, &name);
        // qDebug() << "delete buffer" << name;
        mParent->resources().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DeleteBuffer)
};


class BindBuffer: public GLProc {

public:

    BindBuffer(Demo::GLWidget* p): GLProc("bindbuffer", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        GLuint buffer = vals[start+1].value<int>();
        // qDebug() << "glBindBuffer" << target << buffer;
        mParent->glBindBuffer(target, buffer);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(BindBuffer)
};

class BufferData: public GLProc {

public:

    BufferData(Demo::GLWidget* p): GLProc("bufferdata", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
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

    CLONEMETHOD(BufferData)
};

class VertexAttribPointer: public GLProc {

public:

    VertexAttribPointer(Demo::GLWidget* p): GLProc("vertexattribpointer", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        argt = Symbol::Text;
        mArgTypes.append(argt);
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
                                       (const void*) (size_t) spec.offset);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(VertexAttribPointer)
};

class Draw: public GLProc {

public:

    Draw(Demo::GLWidget* p): GLProc("draw", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        argt = Symbol::Text;
        mArgTypes.append(argt);
        argt = Symbol::Integer;
        mArgTypes.append(argt);
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

    CLONEMETHOD(Draw)
};


class EnableVertexAttribArray: public GLProc {

public:

    EnableVertexAttribArray(Demo::GLWidget* p): GLProc("enablevertexattribarray", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "glEnableVertexAttribArray" << name;
        mParent->glEnableVertexAttribArray(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(EnableVertexAttribArray)
};

class DisableVertexAttribArray: public GLProc {

public:

    DisableVertexAttribArray(Demo::GLWidget* p): GLProc("disablevertexattribarray", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "glDisableVertexAttribArray" << name;
        mParent->glDisableVertexAttribArray(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DisableVertexAttribArray)
};


class ActiveTexture: public GLProc {

public:

    ActiveTexture(Demo::GLWidget* p): GLProc("activetexture", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "ActiveTexture" << name;
        mParent->glActiveTexture(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(ActiveTexture)
};


class GenerateMipMap: public GLProc {

public:

    GenerateMipMap(Demo::GLWidget* p): GLProc("generatemipmap", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        // qDebug() << "GenerateMipMap" << target;
        mParent->glGenerateMipmap(target);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(GenerateMipMap)
};

class BindTexture: public GLProc {

public:

    BindTexture(Demo::GLWidget* p): GLProc("bindtexture", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        GLuint texture = vals[start + 1].value<int>();
        // qDebug() << "BindTexture" << target << texture;
        mParent->glBindTexture(target, texture);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(BindTexture)
};

class GenTexture: public GLProc {

public:

    GenTexture(Demo::GLWidget* p): GLProc("gentexture", Symbol::Integer, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) override {
        GLuint ret;
        // qDebug() << "GenTexture";
        mParent->glGenTextures(1, &ret);
        // qDebug() << "texture" << ret;
        mParent->resources().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

    CLONEMETHOD(GenTexture)
};

class DeleteTexture: public GLProc {

public:

    DeleteTexture(Demo::GLWidget* p): GLProc("deletetexture", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "DeleteTexture" << name;
        if (!mParent->glIsTexture(name)) {
            throw GLError(QString(R"("%1" is not a testure)").arg(name));
        }
        mParent->glDeleteTextures(1, &name);
        // qDebug() << "removing texture" << name;
        mParent->resources().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DeleteTexture)
};


class TexParameter: public GLProc {

public:

    TexParameter(Demo::GLWidget* p): GLProc("texparameter", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
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

    CLONEMETHOD(TexParameter)
};


class TexImage2D: public GLProc {

public:

    TexImage2D(Demo::GLWidget* p): GLProc("teximage2d", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        argt = Symbol::Text;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        GLuint level = vals[start + 1].value<int>();
        GLuint iformat = vals[start + 2].value<int>();
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

    CLONEMETHOD(TexImage2D)
};


class BlendFunc: public GLProc {

public:

    BlendFunc(Demo::GLWidget* p): GLProc("blendfunc", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint src = vals[start].value<int>();
        GLuint dst = vals[start + 1].value<int>();
        mParent->glBlendFunc(src, dst);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(BlendFunc)
;
};


class BlendEquation: public GLProc {

public:

    BlendEquation(Demo::GLWidget* p): GLProc("blendequation", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint mode = vals[start].value<int>();
        mParent->glBlendEquation(mode);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(BlendEquation)
};

class BlendColor: public GLProc {

public:

    BlendColor(Demo::GLWidget* p): GLProc("blendcolor", Symbol::Integer, p) {
        int argt = Symbol::Vector;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Vector4 c = vals[start].value<Vector4>();
        mParent->glBlendColor(c[X], c[Y], c[Z], c[W]);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(BlendColor)
};

class PolygonOffset: public GLProc {

public:

    PolygonOffset(Demo::GLWidget* p): GLProc("polygonoffset", Symbol::Integer, p) {
        int argt = Symbol::Real;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real factor = vals[start].value<Math3D::Real>();
        Math3D::Real units = vals[start + 1].value<Math3D::Real>();
        // qDebug() << "glPolygonOffset" << depth;
        mParent->glPolygonOffset(factor, units);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(PolygonOffset)
};

class DepthFunc: public GLProc {

public:

    DepthFunc(Demo::GLWidget* p): GLProc("depthfunc", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint func = vals[start].value<int>();
        mParent->glDepthFunc(func);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DepthFunc)
};


class StencilFunc: public GLProc {

public:

    StencilFunc(Demo::GLWidget* p): GLProc("stencilfunc", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint func = vals[start].value<int>();
        GLint ref = vals[start+1].value<int>();
        GLuint mask = vals[start+2].value<int>();
        mParent->glStencilFunc(func, ref, mask);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(StencilFunc)
};

class StencilOp: public GLProc {

public:

    StencilOp(Demo::GLWidget* p): GLProc("stencilop", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint sfail = vals[start].value<int>();
        GLuint dpfail = vals[start+1].value<int>();
        GLuint dppass = vals[start+2].value<int>();
        mParent->glStencilOp(sfail, dpfail, dppass);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(StencilOp)
};

class GenFrameBuffer: public GLProc {

public:

    GenFrameBuffer(Demo::GLWidget* p): GLProc("genframebuffer", Symbol::Integer, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) override {
        GLuint ret;
        // qDebug() << "glGenFrameBuffers";
        mParent->glGenFramebuffers(1, &ret);
        // qDebug() << "frame buffer" << ret;
        mParent->resources().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

    CLONEMETHOD(GenFrameBuffer)
};

class DeleteFrameBuffer: public GLProc {

public:

    DeleteFrameBuffer(Demo::GLWidget* p): GLProc("deleteframebuffer", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint name = vals[start].value<int>();
        // qDebug() << "glDeleteFrameBuffers" << name;
        if (!mParent->glIsFramebuffer(name)) {
            throw GLError(QString(R"("%1" is not a frame buffer)").arg(name));
        }
        mParent->glDeleteFramebuffers(1, &name);
        // qDebug() << "delete frame buffer" << name;
        mParent->resources().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DeleteFrameBuffer)
};


class BindFrameBuffer: public GLProc {

public:

    BindFrameBuffer(Demo::GLWidget* p): GLProc("bindframebuffer", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint target = vals[start].value<int>();
        GLuint buffer = vals[start+1].value<int>();
        // qDebug() << "glBindFrameBuffer" << target << buffer;
        mParent->glBindFramebuffer(target, buffer);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(BindFrameBuffer)
};


class FrameBufferTexture2D: public GLProc {

public:

    FrameBufferTexture2D(Demo::GLWidget* p): GLProc("framebuffertexture2d", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
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

    CLONEMETHOD(FrameBufferTexture2D)
};

class FrameBufferTextureLayer: public GLProc {

public:

    FrameBufferTextureLayer(Demo::GLWidget* p): GLProc("framebuffertexturelayer", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
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

    CLONEMETHOD(FrameBufferTextureLayer)
};

class CheckFrameBufferStatus: public GLProc {

public:

    CheckFrameBufferStatus(Demo::GLWidget* p): GLProc("checkframebufferstatus", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
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

    CLONEMETHOD(CheckFrameBufferStatus)
};

class DrawBuffer: public GLProc {

public:

    DrawBuffer(Demo::GLWidget* p): GLProc("drawbuffer", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) override {
        GLuint buf = vals[start].value<int>();
        // qDebug() << "glDrawBuffer" << target;
        mParent->glDrawBuffers(1, &buf);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DrawBuffer)
};

class Functions {

public:

    QList<Demo::Symbol*> contents;

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
        contents.append(new BufferData(p));
        contents.append(new VertexAttribPointer(p));
        contents.append(new Draw(p));
        contents.append(new EnableVertexAttribArray(p));
        contents.append(new DisableVertexAttribArray(p));
        contents.append(new ActiveTexture(p));
        contents.append(new GenerateMipMap(p));
        contents.append(new BindTexture(p));
        contents.append(new GenTexture(p));
        contents.append(new DeleteTexture(p));
        contents.append(new TexParameter(p));
        contents.append(new TexImage2D(p));
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
    }
};

#define CONST(value) contents.append(new Demo::Constant(QString(#value).toLower(), GL_ ## value))

class Constants {

public:

    QList<Demo::Symbol*> contents;

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
        // bindbuffer
        CONST(ARRAY_BUFFER);
        CONST(ELEMENT_ARRAY_BUFFER);
        // bufferdata
        CONST(STATIC_DRAW);
        CONST(STREAM_DRAW);
        CONST(DYNAMIC_DRAW);
        // draw
        CONST(POINTS);
        CONST(LINES);
        CONST(TRIANGLES);
        // active texture
        CONST(TEXTURE0);
        // texture targets
        CONST(TEXTURE_2D);
        CONST(TEXTURE_2D_MULTISAMPLE);
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
    }


};

#undef CONST
}}
#endif // GL_FUNCTIONS_H
