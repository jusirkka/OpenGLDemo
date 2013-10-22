#ifndef demo_glwidget_h
#define demo_glwidget_h

#include <QGLWidget>
#include <QGLFunctions>
#include <QDebug>

class QMouseEvent;

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
    const GL::Blob& blob(int index) const {return *mBlobs[index];}

    ~GLWidget();


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

private:

    RunnerMap mRunners;
    bool mInitialized;
    ResourceList mResources;
    BlobList mBlobs;
    Variable* mCamera;

};

}

#endif
