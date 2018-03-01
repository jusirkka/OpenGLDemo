#ifndef demo_glwidget_h
#define demo_glwidget_h

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include "logging.h"
#include "math3d.h"
#include "gl_lang_compiler.h"

class QMouseEvent;

class Camera;

#define OpenGLFunctions QOpenGLFunctions_4_5_Core

namespace Demo {

namespace GL {
    class Blob;
    class TexBlob;
    class Downloader;
}

class VideoEncoder;
class Variable;

class GLWidget : public QOpenGLWidget, public OpenGLFunctions {

    Q_OBJECT


public:


    GLWidget(QWidget *parent = nullptr);
    void addGLSymbols(SymbolMap& globals, VariableMap& exports);

    const GL::Blob& blob(int index) const {return *mBlobs[index];}
    const GL::TexBlob& texBlob(int index) const {return *mTexBlobs[index];}
    GL::Blob* blob(const SymbolMap& globals, const QString& name) const;
    GL::TexBlob* texBlob(const SymbolMap& globals, const QString& name) const;

    void animStart();
    void animStop();
    void animReset(int);
    bool animRunning() const;
    void cameraStop();
    void saveToDisk(bool on, const QString& basePath);

    void setProjection(float near, float far);

    GLuint resource(const QString& res, GLenum param = 0);
    void deresource(const QString& res, GLuint name);

    bool initialized() const {return mInitialized;}

    ~GLWidget() override;


public slots:

    void initChanged();
    void drawChanged();

protected:

    void paintGL() override;
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void hideEvent(QHideEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private:


    using BlobVector = QVector<GL::Blob *>;
    using TexBlobVector = QVector<GL::TexBlob *>;

    class Mover {
    public:
        Mover(Demo::GLWidget* owner):
            parent(owner) {}

        Demo::GLWidget* parent;

        virtual void move() {}
        virtual ~Mover() = default;
    };

    class Zoomer: public Mover {
    public:
        Zoomer(Demo::GLWidget* owner):
            Mover(owner) {}

        void move() override {parent->zoom();}
    };

    class Spinner: public Mover {
    public:
        Spinner(Demo::GLWidget* owner):
            Mover(owner) {}

        void move() override {parent->spin();}
    };

    class Panner: public Mover {
    public:
        Panner(Demo::GLWidget* owner):
            Mover(owner) {}

        void move() override {parent->pan();}
    };

    friend class Spinner;
    friend class Zoomer;
    friend class Panner;

    enum LastOp {Spin, Zoom, Pan, None};

    class Resource {
    public:
        Resource(Demo::GLWidget* owner)
            : parent(owner) {}

        Demo::GLWidget* parent;
        GLuint name;

        virtual ~Resource() = default;
    };

    class Texture: public Resource {
    public:
        Texture(Demo::GLWidget* owner)
            : Resource(owner) {
            parent->glGenTextures(1, &name);
        }
        ~Texture() {
            parent->glDeleteTextures(1, &name);
        }
    };

    class Shader: public Resource {
    public:
        Shader(Demo::GLWidget* owner, GLenum type)
            : Resource(owner) {
            name = parent->glCreateShader(type);
        }
        ~Shader() {
            parent->glDeleteShader(name);
        }
    };

    class Program: public Resource {
    public:
        Program(Demo::GLWidget* owner)
            : Resource(owner) {
            name = parent->glCreateProgram();
        }
        ~Program() {
            parent->glDeleteProgram(name);
        }
    };

    class Buffer: public Resource {
    public:
        Buffer(Demo::GLWidget* owner)
            : Resource(owner) {
            parent->glGenBuffers(1, &name);
        }
        ~Buffer() {
            parent->glDeleteBuffers(1, &name);
        }
    };

    class FrameBuffer: public Resource {
    public:
        FrameBuffer(Demo::GLWidget* owner)
            : Resource(owner) {
            parent->glGenFramebuffers(1, &name);
        }
        ~FrameBuffer() {
            parent->glDeleteFramebuffers(1, &name);
        }
    };

    class VertexArray: public Resource {
    public:
        VertexArray(Demo::GLWidget* owner)
            : Resource(owner) {
            parent->glGenVertexArrays(1, &name);
        }
        ~VertexArray() {
            parent->glDeleteVertexArrays(1, &name);
        }
    };

    using ResourceMap = QMap<QString, Resource*>;


signals:

    void init();
    void draw();
    void hidden();
    void toggleAnimate();
    void openGLReady(bool);

private:

    void defaults();
    void addBlob(QObject* blob, SymbolMap& globals);

    void zoom();
    void spin();
    void pan();

private slots:

    void move();
    void anim();
    void realResize();
    void encodingFinished();

private:

    bool mInitialized;
    ResourceMap mResources;
    BlobVector mBlobs;
    TexBlobVector mTexBlobs;
    Variable* mCameraVar;
    Variable* mProjectionVar;
    Variable* mInvProjVar;
    Camera* mCamera;
    int mTime;
    Variable* mTimeVar;
    Variable* mWidthVar;
    Variable* mHeightVar;
    int mDx, mDy, mDim;
    QPoint mLastPos;
    bool mGravity;
    QTimer* mTimer;
    QTimer* mAnimTimer;
    QTimer* mResizeTimer;
    Mover* mMover;
    float mNear;
    float mFar;
    LastOp mLastOp;
    VideoEncoder* mEncoder;
    GL::Downloader* mDownloader;
    bool mRecording;

};

}

#endif
