#ifndef GL_FUNCTIONS_H
#define GL_FUNCTIONS_H

#include "gl_widget.h"
#include "function.h"
#include "constant.h"
#include "blob.h"

#include <QList>

using Math3D::X;
using Math3D::Y;
using Math3D::Z;
using Math3D::W;
using Math3D::Vector4;
using Math3D::Matrix4;


namespace GL {

class GLProc: public Demo::Function {
public:

    GLProc(const QString& name, int type, Demo::GLWidget* p)
        : Demo::Function(name, type),
        mParent(p)
    {}

    #define ALT(item) case item: qWarning() << #item; break

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
        qDebug() << "glEnable" << vals[start].value<Math3D::Integer>();
        ::glEnable(vals[start].value<Math3D::Integer>());
        mValue.setValue(0);
        return mValue;
    }

    ~Enable() {}
};

class Disable: public Demo::Function {

public:

    Disable(): Function("disable", Symbol::Integer) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        qDebug() << "glDisable" << vals[start].value<Math3D::Integer>();
        ::glDisable(vals[start].value<Math3D::Integer>());
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glDepthRange" << near << far;
        mParent->glDepthRangef(near, far);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glLineWidth" << w;
        ::glLineWidth(w);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glFrontFace" << face;
        ::glFrontFace(face);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glCullFace" << face;
        ::glCullFace(face);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glColorMask" << r << g << b << a;
        ::glColorMask(r, g, b, a);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glDepthMask" << d;
        ::glDepthMask(d);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glClear" << mask;
        ::glClear(mask);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glClearColor" << color[X] << color[Y] << color[Z] << color[W];
        ::glClearColor(color[X], color[Y], color[Z], color[W]);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glClearDepth" << depth;
        mParent->glClearDepthf(depth);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glCreateShader" << type;
        int ret = mParent->glCreateShader(type);
        mParent->resources().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

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
        qDebug() << "glShaderSource" << name;
        mParent->glShaderSource(name, 1, &data, 0);
        qDebug() << "glCompileShader" << name;
        mParent->glCompileShader(name);
        int status;
        mParent->glGetShaderiv(name, GL_COMPILE_STATUS, &status);
        if (!status) {
            int len;
            mParent->glGetShaderiv(name, GL_INFO_LOG_LENGTH, &len);
            char info[len];
            mParent->glGetShaderInfoLog(name, len, &len, info);
            qWarning() << info;
            char sh_src[1024];
            mParent->glGetShaderSource(name, 1024, &len, sh_src);
            qDebug() << QString(sh_src);
        }
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glDeleteShader" << name;
        mParent->glDeleteShader(name);
        mParent->resources().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

    virtual ~DeleteShader() {}
};


class CreateProgram: public GLProc {

public:

    CreateProgram(Demo::GLWidget* p): GLProc("createprogram", Symbol::Integer, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) {
        qDebug() << "glCreateProgram";
        int ret = mParent->glCreateProgram();
        mParent->resources().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

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
        qDebug() << "glAttachShader" << prog << shader;
        mParent->glAttachShader(prog, shader);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glDetachShader" << prog << shader;
        mParent->glDetachShader(prog, shader);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glLinkProgram" << name;
        mParent->glLinkProgram(name);
        int len;
        mParent->glGetProgramiv(name, GL_INFO_LOG_LENGTH, &len);
        char info[len];
        mParent->glGetProgramInfoLog(name, len, &len, info);
        qDebug() << QString(info);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glUseProgram" << name;
        // mParent->glUseProgram(name);
        QGLFunctions glFuncs(QGLContext::currentContext());
        glFuncs.glUseProgram(name);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glDeleteProgram" << name;
        mParent->glDeleteProgram(name);
        mParent->resources().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glGetAttribLocation" << prog << name;
        QByteArray bytes = name.toLatin1();
        const char* data = bytes.constData();
        int loc = mParent->glGetAttribLocation(prog, data);
        mValue.setValue(loc);
        return mValue;
    }

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
        qDebug() << "glGetUniformLocation" << prog << name;
        QByteArray bytes = name.toLatin1();
        const char* data = bytes.constData();
        int loc = mParent->glGetUniformLocation(prog, data);
        mValue.setValue(loc);
        return mValue;
    }

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
        qDebug() << "glUniform1f" << loc << uniform;
        mParent->glUniform1f(loc, uniform);
        mValue.setValue(0);
        return mValue;
    }

    virtual ~Uniform1F() {}
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
        qDebug() << "glUniform4f" << loc << uni[X] << uni[Y] << uni[Z] << uni[W];
        mParent->glUniform4f(loc, uni[X], uni[Y], uni[Z], uni[W]);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glUniformMatrix4F" << loc;
        mParent->glUniformMatrix4fv(loc, 1, GL_FALSE, uni.readGLFloat());
        mValue.setValue(0);
        return mValue;
    }

    virtual ~UniformMatrix4F() {}
};

class GenBuffer: public GLProc {

public:

    GenBuffer(Demo::GLWidget* p): GLProc("genbuffer", Symbol::Integer, p) {}

    const QVariant& gl_execute(const QVector<QVariant>&, int) {
        GLuint ret;
        qDebug() << "glGenBuffers";
        mParent->glGenBuffers(1, &ret);
        mParent->resources().append(ret);
        mValue.setValue(ret);
        return mValue;
    }

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
        qDebug() << "glDeleteBuffers" << name;
        mParent->glDeleteBuffers(1, &name);
        mParent->resources().removeOne(name);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glBindBuffer" << target << buffer;
        mParent->glBindBuffer(target, buffer);
        mValue.setValue(0);
        return mValue;
    }

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
        const Blob& blob =  mParent->blob(vals[start+1].value<int>());
        GLuint usage =  vals[start+2].value<int>();
        qDebug() << "glBufferData" << target << blob.name() << usage;
        mParent->glBufferData(target, blob.bytelen(target), blob.bytes(target), usage);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "VertexAttribPointer" << index << blob.name() << attr;
        const BlobSpec& spec = blob.spec(attr);
        mParent->glVertexAttribPointer(index, spec.size, spec.type, spec.normalized, spec.stride, (const void*) spec.offset);
        mValue.setValue(0);
        return mValue;
    }

    ~VertexAttribPointer() {}
};

class Draw: public GLProc {

public:

    Draw(Demo::GLWidget* p): GLProc("draw", Symbol::Integer, p) {
        int argt = Symbol::Integer;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& gl_execute(const QVector<QVariant>& vals, int start) {
        const Blob& blob =  mParent->blob(vals[start].value<int>());
        GLuint mode =  vals[start+1].value<int>();
        qDebug() << "Draw" << blob.name() << mode;
        blob.draw(mode);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glEnableVertexAttribArray" << name;
        mParent->glEnableVertexAttribArray(name);
        mValue.setValue(0);
        return mValue;
    }

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
        qDebug() << "glDisableVertexAttribArray" << name;
        mParent->glDisableVertexAttribArray(name);
        mValue.setValue(0);
        return mValue;
    }

    ~DisableVertexAttribArray() {}
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
    }


};

#undef CONST
}
#endif // GL_FUNCTIONS_H