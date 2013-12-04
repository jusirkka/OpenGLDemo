#ifndef demo_glwidget_h
#define demo_glwidget_h

#include <QGLWidget>
#include <QGLFunctions>
#include <QDebug>
#include "math3d.h"

class QMouseEvent;

class Camera;

namespace GL {
    class Blob;
    class TexBlob;
    class Emitter;
    class ImageStore;
}

namespace Demo {

class Variable;
class Runner;
class Project;

class GLWidget : public QGLWidget, public QGLFunctions {

    Q_OBJECT

public:

    typedef QList<unsigned int> ResourceList;

public:


    GLWidget(QWidget *parent = 0);

    ResourceList& resources() {return mResources;}
    ResourceList& buffers() {return mBuffers;}
    ResourceList& textures() {return mTextures;}
    const GL::Blob& blob(int index) const {return *mBlobs[index];}
    const GL::TexBlob& texBlob(int index) const {return *mTexBlobs[index];}
    void setProject(Project* p);

    virtual ~GLWidget();


public slots:

    void initChanged();
    void drawChanged();

protected:

    void paintGL();
    void initializeGL();
    void resizeGL(int w, int h);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:


    friend class GL::Emitter;

    typedef QList<GL::Blob*> BlobList;
    typedef QList<GL::TexBlob*> TexBlobList;

    class Mover {
    public:
        Mover(Demo::GLWidget* owner):
            parent(owner) {}

        Demo::GLWidget* parent;

        virtual void move() {}
        virtual ~Mover() {}
    };

    class Zoomer: public Mover {
    public:
        Zoomer(Demo::GLWidget* owner):
            Mover(owner) {}

        void move() {parent->zoom();}
        virtual ~Zoomer() {}
    };

    class Spinner: public Mover {
    public:
        Spinner(Demo::GLWidget* owner):
            Mover(owner) {}

        void move() {parent->spin();}
        virtual ~Spinner() {}
    };

    class Panner: public Mover {
    public:
        Panner(Demo::GLWidget* owner):
            Mover(owner) {}

        void move() {parent->pan();}
        virtual ~Panner() {}
    };

    friend class Spinner;
    friend class Zoomer;
    friend class Panner;

signals:

    void init();
    void draw();
    void evaluate(const QString& curr, const QString& other);

private:

    void defaults();
    void addBlob(QObject*);

    void zoom();
    void spin();
    void pan();

private slots:

    void move();


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
    Math3D::Matrix4 mProj;
    int mDx, mDy, mDim;
    QPoint mLastPos;
    bool mGravity;
    QTimer* mTimer;
    Project* mProject;
    Project* mDefaultProject;
    Mover* mMover;

};

}

#endif
