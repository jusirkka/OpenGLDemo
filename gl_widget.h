#ifndef demo_glwidget_h
#define demo_glwidget_h

#include <QGLWidget>
#include <QGLFunctions>
#include <QDebug>
#include "math3d.h"

class QMouseEvent;

class Camera;

namespace GL {class Blob;}

namespace Demo {

class Variable;
class Runner;

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

    virtual ~GLWidget();


public slots:

    void parse(int key, const QString&);

protected:

    void paintGL();
    void initializeGL();
    void resizeGL(int w, int h);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:



    typedef QMap<int, Runner*> RunnerMap;
    typedef QList<GL::Blob*> BlobList;

    enum {InitKey, DrawKey};

private:

    void defaults();
    void addBlob(QObject*);

private slots:

    void spin();

private:

    RunnerMap mRunners;
    bool mInitialized;
    ResourceList mResources;
    ResourceList mBuffers;
    ResourceList mTextures;
    BlobList mBlobs;
    Variable* mCameraVar;
    Variable* mProjectionVar;
    Camera* mCamera;
    Math3D::Matrix4 mProj;
    int mDx, mDy, mDim;
    QPoint mLastPos;
    bool mGravity;
    QTimer* mTimer;

};

}

#endif
