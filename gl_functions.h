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
    GLError(const QString& msg)
        :emsg(msg)
    {}

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

    const QVariant& execute(const QVector<QVariant>& vals, int start) {

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


class Enable: public Demo::Function {

public:

    Enable(): Function("enable", Symbol::Integer) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        // qDebug() << "glEnable" << vals[start].value<Math3D::Integer>();
        ::glEnable(vals[start].value<Math3D::Integer>());
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Enable)

    ~Enable() {}
};

class Disable: public Demo::Function {

public:

    Disable(): Function("disable", Symbol::Integer) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        // qDebug() << "glDisable" << vals[start].value<Math3D::Integer>();
        ::glDisable(vals[start].value<Math3D::Integer>());
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Disable)

    ~Disable() {}
};

class DepthRange: public GLProc {

public:

    DepthRange(Demo::GLWidget* p): GLProc("depthrange", Symbol::Integer, p) {
        int argt = Symbol::Real;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        Math3D::Real near =  vals[start].value<Math3D::Real>();
        Math3D::Real far =  vals[start + 1].value<Math3D::Real>();
        // qDebug() << "glDepthRange" << near << far;
        mParent->glDepthRangef(near, far);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DepthRange)

    ~DepthRange() {}
};

class LineWidth: public Demo::Function {

public:

    LineWidth(): Function("linewidth", Symbol::Integer) {
        int argt = Symbol::Real;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        Math3D::Real w =  vals[start].value<Math3D::Real>();
        // qDebug() << "glLineWidth" << w;
        ::glLineWidth(w);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(LineWidth)

    ~LineWidth() {}
};

class FrontFace: public Demo::Function {

public:

    FrontFace(): Function("frontface", Symbol::Integer) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        Math3D::Integer face =  vals[start].value<Math3D::Integer>();
        // qDebug() << "glFrontFace" << face;
        ::glFrontFace(face);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(FrontFace)

    ~FrontFace() {}
};

class CullFace: public Demo::Function {

public:

    CullFace(): Function("cullface", Symbol::Integer) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        Math3D::Integer face =  vals[start].value<Math3D::Integer>();
        // qDebug() << "glCullFace" << face;
        ::glCullFace(face);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(CullFace)

    ~CullFace() {}
};

class ColorMask: public Demo::Function {

public:

    ColorMask(): Function("colormask", Symbol::Integer) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        Math3D::Integer r =  vals[start].value<Math3D::Integer>();
        Math3D::Integer g =  vals[start+1].value<Math3D::Integer>();
        Math3D::Integer b =  vals[start+2].value<Math3D::Integer>();
        Math3D::Integer a =  vals[start+3].value<Math3D::Integer>();
        // qDebug() << "glColorMask" << r << g << b << a;
        ::glColorMask(r, g, b, a);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(ColorMask)

    ~ColorMask() {}
};

class DepthMask: public Demo::Function {

public:

    DepthMask(): Function("depthmask", Symbol::Integer) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        Math3D::Integer d =  vals[start].value<Math3D::Integer>();
        // qDebug() << "glDepthMask" << d;
        ::glDepthMask(d);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DepthMask)

    ~DepthMask() {}
};

class Clear: public Demo::Function {

public:

    Clear(): Function("clear", Symbol::Integer) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        Math3D::Integer mask =  vals[start].value<Math3D::Integer>();
        // qDebug() << "glClear" << mask;
        ::glClear(mask);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Clear)

    ~Clear() {}
};

class ClearColor: public Demo::Function {

public:

    ClearColor(): Function("clearcolor", Symbol::Integer) {
        int argt = Symbol::Vector;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        Vector4 color =  vals[start].value<Vector4>();
        // qDebug() << "glClearColor" << color[X] << color[Y] << color[Z] << color[W];
        ::glClearColor(color[X], color[Y], color[Z], color[W]);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(ClearColor)

    ~ClearColor() {}
};

class ClearDepth: public GLProc {

public:

    ClearDepth(Demo::GLWidget* p): GLProc("cleardepth", Symbol::Integer, p) {
        int argt = Symbol::Real;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        Math3D::Real depth =  vals[start].value<Math3D::Real>();
        // qDebug() << "glClearDepth" << depth;
        mParent->glClearDepthf(depth);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(ClearDepth)

    virtual ~ClearDepth() {}
};

class CreateShader: public GLProc {

public:

    CreateShader(Demo::GLWidget* p): GLProc("createshader", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int type =  vals[start].value<int>();
        // qDebug() << "glCreateShader" << type;
        int ret = mParent->glCreateShader(type);
        mParent->resources().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

    CLONEMETHOD(CreateShader)

    virtual ~CreateShader() {}
};

class CompileShader: public GLProc {

public:

    CompileShader(Demo::GLWidget* p): GLProc("compileshader", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        argt = Symbol::Text;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint name =  vals[start].value<int>();
        QByteArray bytes = vals[start+1].value<QString>().toLatin1();
        const char *data = bytes.constData();
        // qDebug() << "glShaderSource" << name;
        mParent->glShaderSource(name, 1, &data, 0);
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

    virtual ~CompileShader() {}
};

class DeleteShader: public GLProc {

public:

    DeleteShader(Demo::GLWidget* p): GLProc("deleteshader", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int name =  vals[start].value<int>();
        // qDebug() << "glDeleteShader" << name;
        mParent->glDeleteShader(name);
        mParent->resources().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DeleteShader)

    virtual ~DeleteShader() {}
};


class CreateProgram: public GLProc {

public:

    CreateProgram(Demo::GLWidget* p): GLProc("createprogram", Symbol::Integer, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) {
        // qDebug() << "glCreateProgram";
        int ret = mParent->glCreateProgram();
        mParent->resources().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

    CLONEMETHOD(CreateProgram)

    virtual ~CreateProgram() {}
};


class AttachShader: public GLProc {

public:

    AttachShader(Demo::GLWidget* p): GLProc("attachshader", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int prog =  vals[start].value<int>();
        int shader =  vals[start+1].value<int>();
        // qDebug() << "glAttachShader" << prog << shader;
        mParent->glAttachShader(prog, shader);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(AttachShader)

    virtual ~AttachShader() {}
};


class DetachShader: public GLProc {

public:

    DetachShader(Demo::GLWidget* p): GLProc("detachshader", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int prog =  vals[start].value<int>();
        int shader =  vals[start+1].value<int>();
        // qDebug() << "glDetachShader" << prog << shader;
        mParent->glDetachShader(prog, shader);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DetachShader)

    virtual ~DetachShader() {}
};


class LinkProgram: public GLProc {

public:

    LinkProgram(Demo::GLWidget* p): GLProc("linkprogram", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int name =  vals[start].value<int>();
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

    virtual ~LinkProgram() {}
};


class UseProgram: public GLProc {

public:

    UseProgram(Demo::GLWidget* p): GLProc("useprogram", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int name =  vals[start].value<int>();
        // qDebug() << "glUseProgram" << name;
        mParent->glUseProgram(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(UseProgram)

    virtual ~UseProgram() {}
};


class DeleteProgram: public GLProc {

public:

    DeleteProgram(Demo::GLWidget* p): GLProc("deleteprogram", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int name =  vals[start].value<int>();
        // qDebug() << "glDeleteProgram" << name;
        mParent->glDeleteProgram(name);
        mParent->resources().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DeleteProgram)

    virtual ~DeleteProgram() {}
};

class GetAttribLocation: public GLProc {

public:

    GetAttribLocation(Demo::GLWidget* p): GLProc("getattriblocation", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        t = Symbol::Text;
        mArgTypes.append(t);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int prog =  vals[start].value<int>();
        QString name =  vals[start+1].value<QString>();
        // qDebug() << "glGetAttribLocation" << prog << name;
        QByteArray bytes = name.toLatin1();
        const char* data = bytes.constData();
        int loc = mParent->glGetAttribLocation(prog, data);
        mValue.setValue(loc);
        return mValue;
    }

    CLONEMETHOD(GetAttribLocation)

    virtual ~GetAttribLocation() {}
};


class GetUniformLocation: public GLProc {

public:

    GetUniformLocation(Demo::GLWidget* p): GLProc("getuniformlocation", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        t = Symbol::Text;
        mArgTypes.append(t);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int prog =  vals[start].value<int>();
        QString name =  vals[start+1].value<QString>();
        // qDebug() << "glGetUniformLocation" << prog << name;
        QByteArray bytes = name.toLatin1();
        const char* data = bytes.constData();
        int loc = mParent->glGetUniformLocation(prog, data);
        mValue.setValue(loc);
        return mValue;
    }

    CLONEMETHOD(GetUniformLocation)

    virtual ~GetUniformLocation() {}
};


class Uniform1F: public GLProc {

public:

    Uniform1F(Demo::GLWidget* p): GLProc("uniform1f", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        t = Symbol::Real;
        mArgTypes.append(t);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int loc =  vals[start].value<int>();
        Math3D::Real uniform =  vals[start+1].value<Math3D::Real>();
        // qDebug() << "glUniform1f" << loc << uniform;
        mParent->glUniform1f(loc, uniform);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Uniform1F)

    virtual ~Uniform1F() {}
};

class Uniform1I: public GLProc {

public:

    Uniform1I(Demo::GLWidget* p): GLProc("uniform1i", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        mArgTypes.append(t);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int loc =  vals[start].value<int>();
        int uniform =  vals[start+1].value<int>();
        // qDebug() << "glUniform1i" << loc << uniform;
        mParent->glUniform1i(loc, uniform);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Uniform1I)

    virtual ~Uniform1I() {}
};

class Uniform4F: public GLProc {

public:

    Uniform4F(Demo::GLWidget* p): GLProc("uniform4f", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        t = Symbol::Vector;
        mArgTypes.append(t);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int loc =  vals[start].value<int>();
        Vector4 uni =  vals[start+1].value<Vector4>();
        // qDebug() << "glUniform4f" << loc << uni[X] << uni[Y] << uni[Z] << uni[W];
        mParent->glUniform4f(loc, uni[X], uni[Y], uni[Z], uni[W]);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Uniform4F)

    virtual ~Uniform4F() {}
};


class UniformMatrix4F: public GLProc {

public:

    UniformMatrix4F(Demo::GLWidget* p): GLProc("uniformmatrix4f", Symbol::Integer, p) {
        int t = Symbol::Integer;
        mArgTypes.append(t);
        t = Symbol::Matrix;
        mArgTypes.append(t);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        int loc =  vals[start].value<int>();
        Matrix4 uni =  vals[start+1].value<Matrix4>();
        // qDebug() << "glUniformMatrix4F" << loc;
        mParent->glUniformMatrix4fv(loc, 1, GL_FALSE, uni.readArray());
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(UniformMatrix4F)

    virtual ~UniformMatrix4F() {}
};

class GenBuffer: public GLProc {

public:

    GenBuffer(Demo::GLWidget* p): GLProc("genbuffer", Symbol::Integer, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) {
        GLuint ret;
        // qDebug() << "glGenBuffers";
        mParent->glGenBuffers(1, &ret);
        mParent->buffers().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

    CLONEMETHOD(GenBuffer)

    virtual ~GenBuffer() {}
};

class DeleteBuffer: public GLProc {

public:

    DeleteBuffer(Demo::GLWidget* p): GLProc("deletebuffer", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint name =  vals[start].value<int>();
        // qDebug() << "glDeleteBuffers" << name;
        mParent->glDeleteBuffers(1, &name);
        mParent->buffers().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DeleteBuffer)

    virtual ~DeleteBuffer() {}
};


class BindBuffer: public GLProc {

public:

    BindBuffer(Demo::GLWidget* p): GLProc("bindbuffer", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint target =  vals[start].value<int>();
        GLuint buffer =  vals[start+1].value<int>();
        // qDebug() << "glBindBuffer" << target << buffer;
        mParent->glBindBuffer(target, buffer);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(BindBuffer)

    ~BindBuffer() {}
};

class BufferData: public GLProc {

public:

    BufferData(Demo::GLWidget* p): GLProc("bufferdata", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint target =  vals[start].value<int>();
        const Blob& blob = mParent->blob(vals[start+1].value<int>());
        GLuint usage =  vals[start+2].value<int>();
        // qDebug() << "glBufferData" << target << blob.name() << usage;
        mParent->glBufferData(target, blob.bytelen(target), blob.bytes(target), usage);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(BufferData)

    ~BufferData() {}
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

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint index =  vals[start].value<int>();
        const Blob& blob =  mParent->blob(vals[start+1].value<int>());
        QString attr =  vals[start+2].value<QString>();
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

    ~VertexAttribPointer() {}
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

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        const Blob& blob =  mParent->blob(vals[start].value<int>());
        QString attr =  vals[start + 1].value<QString>();
        GLuint mode =  vals[start + 2].value<int>();
        // qDebug() << "Draw" << blob.name() << attr << mode;
        blob.draw(mode, attr);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(Draw)

    ~Draw() {}
};


class EnableVertexAttribArray: public GLProc {

public:

    EnableVertexAttribArray(Demo::GLWidget* p): GLProc("enablevertexattribarray", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint name =  vals[start].value<int>();
        // qDebug() << "glEnableVertexAttribArray" << name;
        mParent->glEnableVertexAttribArray(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(EnableVertexAttribArray)

    ~EnableVertexAttribArray() {}
};

class DisableVertexAttribArray: public GLProc {

public:

    DisableVertexAttribArray(Demo::GLWidget* p): GLProc("disablevertexattribarray", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint name =  vals[start].value<int>();
        // qDebug() << "glDisableVertexAttribArray" << name;
        mParent->glDisableVertexAttribArray(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DisableVertexAttribArray)

    ~DisableVertexAttribArray() {}
};


class ActiveTexture: public GLProc {

public:

    ActiveTexture(Demo::GLWidget* p): GLProc("activetexture", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint name =  vals[start].value<int>();
        // qDebug() << "ActiveTexture" << name;
        mParent->glActiveTexture(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(ActiveTexture)

    ~ActiveTexture() {}
};


class GenerateMipMap: public GLProc {

public:

    GenerateMipMap(Demo::GLWidget* p): GLProc("generatemipmap", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint target =  vals[start].value<int>();
        // qDebug() << "GenerateMipMap" << target;
        mParent->glGenerateMipmap(target);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(GenerateMipMap)

    ~GenerateMipMap() {}
};

class BindTexture: public GLProc {

public:

    BindTexture(Demo::GLWidget* p): GLProc("bindtexture", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint target =  vals[start].value<int>();
        GLuint texture =  vals[start + 1].value<int>();
        // qDebug() << "BindTexture" << target << texture;
        glBindTexture(target, texture);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(BindTexture)

    ~BindTexture() {}
};

class GenTexture: public GLProc {

public:

    GenTexture(Demo::GLWidget* p): GLProc("gentexture", Symbol::Integer, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) {
        GLuint ret;
        // qDebug() << "GenTexture";
        glGenTextures(1, &ret);
        mParent->textures().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

    CLONEMETHOD(GenTexture)

    virtual ~GenTexture() {}
};

class DeleteTexture: public GLProc {

public:

    DeleteTexture(Demo::GLWidget* p): GLProc("deletetexture", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint name =  vals[start].value<int>();
        // qDebug() << "DeleteTexture" << name;
        glDeleteTextures(1, &name);
        mParent->textures().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(DeleteTexture)

    virtual ~DeleteTexture() {}
};


class TexParameter: public GLProc {

public:

    TexParameter(Demo::GLWidget* p): GLProc("texparameter", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint target =  vals[start].value<int>();
        GLuint name =  vals[start + 1].value<int>();
        GLuint param =  vals[start + 2].value<int>();
        // qDebug() << "TexParameter" << target << name << param;
        glTexParameteri(target, name, param);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(TexParameter)

    virtual ~TexParameter() {}
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

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint target =  vals[start].value<int>();
        GLuint level =  vals[start + 1].value<int>();
        GLuint iformat =  vals[start + 2].value<int>();
        const TexBlob& blob =  mParent->texBlob(vals[start + 3].value<int>());
        QString attr =  vals[start + 4].value<QString>();
        // qDebug() << "TexImage2D" << target << level << iformat << blob.name() << attr;
        const TexBlobSpec spec = blob.spec(attr);
        // qDebug() << "TexImage2D" << spec.width << spec.height << spec.type;
        glTexImage2D(target, level, iformat, spec.width, spec.height, 0, spec.format, spec.type, blob.data(attr));
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(TexImage2D)

    ~TexImage2D() {}
};


class BlendFunc: public GLProc {

public:

    BlendFunc(Demo::GLWidget* p): GLProc("blendfunc", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint src =  vals[start].value<int>();
        GLuint dst =  vals[start + 1].value<int>();
        glBlendFunc(src, dst);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(BlendFunc)

    ~BlendFunc() {}
};


class BlendEquation: public GLProc {

public:

    BlendEquation(Demo::GLWidget* p): GLProc("blendequation", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        GLuint mode =  vals[start].value<int>();
        mParent->glBlendEquation(mode);
        mValue.setValue(0);
        return mValue;
    }

    CLONEMETHOD(BlendEquation)

    ~BlendEquation() {}
};


class Functions {

public:

    QList<Demo::Symbol*> contents;

    Functions(Demo::GLWidget* p) {
        contents.append(new Enable());
        contents.append(new Disable());
        contents.append(new DepthRange(p));
        contents.append(new LineWidth());
        contents.append(new FrontFace());
        contents.append(new CullFace());
        contents.append(new ColorMask());
        contents.append(new DepthMask());
        contents.append(new Clear());
        contents.append(new ClearColor());
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
    }


};

#undef CONST
}}
#endif // GL_FUNCTIONS_H
