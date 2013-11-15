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

private:


    typedef QList<GL::Blob*> BlobList;

signals:

    void init();
    void draw();

private:

    void defaults();
    void addBlob(QObject*);

private slots:

    void spin();


private:

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
    Project* mProject;
    Project* mDefaultProject;


};

}

#endif
