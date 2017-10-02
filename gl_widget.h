#ifndef demo_glwidget_h
#define demo_glwidget_h

#include <QGLWidget>
#include <QOpenGLExtraFunctions>
#include <QDebug>
#include "math3d.h"
#include "gl_lang_compiler.h"

class QMouseEvent;

class Camera;


namespace Demo {

namespace GL {
    class Blob;
    class TexBlob;
    class Emitter;
}

class Variable;

class GLWidget : public QGLWidget, public QOpenGLExtraFunctions {

    Q_OBJECT

public:

    using ResourceList = QList<unsigned int>;

public:


    GLWidget(QWidget *parent = nullptr);
    void addGLSymbols(SymbolMap& globals, VariableMap& exports);

    ResourceList& resources() {return mResources;}
    ResourceList& buffers() {return mBuffers;}
    ResourceList& textures() {return mTextures;}
    const GL::Blob& blob(int index) const {return *mBlobs[index];}
    const GL::TexBlob& texBlob(int index) const {return *mTexBlobs[index];}
    GL::Blob* blob(const SymbolMap& globals, const QString& name) const;
    GL::TexBlob* texBlob(const SymbolMap& globals, const QString& name) const;

    void animStart();
    void animStop();
    void animReset(int);

    void setProjection(float near, float far);

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

private:


    friend class GL::Emitter;

    using BlobList = QList<GL::Blob *>;
    using TexBlobList = QList<GL::TexBlob *>;

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

signals:

    void init();
    void draw();

private:

    void defaults();
    void addBlob(QObject* blob, SymbolMap& globals);

    void zoom();
    void spin();
    void pan();

private slots:

    void move();
    void anim();


private:

    bool mInitialized;
    ResourceList mResources;
    ResourceList mBuffers;
    ResourceList mTextures;
    BlobList mBlobs;
    TexBlobList mTexBlobs;
    Variable* mCameraVar;
    Variable* mProjectionVar;
    Camera* mCamera;
    int mTime;
    Variable* mTimeVar;
    Math3D::Matrix4 mProj;
    int mDx, mDy, mDim;
    QPoint mLastPos;
    bool mGravity;
    QTimer* mTimer;
    QTimer* mAnimTimer;
    Mover* mMover;
    float mNear;
    float mFar;

};

}

#endif
