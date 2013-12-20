#include "gl_widget.h"
#include "parser.h"
#include "runner.h"
#include "gl_functions.h"
#include "camera.h"
#include "imagestore.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPluginLoader>
#include <QDir>
#include <QApplication>
#include <QTimer>

using Math3D::Matrix4;
using Math3D::Vector4;
using Math3D::Real;

Demo::GLWidget::GLWidget(QWidget *parent):
    QGLWidget(parent),
    QGLFunctions(),
    mInitialized(false),
    mDim(500),
    mMover(new Mover(this))
{
    GL::Functions funcs(this);
    foreach(Symbol* func, funcs.contents) Parser::AddSymbol(func);
    GL::Constants constants;
    foreach(Symbol* c, constants.contents) Parser::AddSymbol(c);

    mCameraVar = dynamic_cast<Variable*>(Parser::Symbols()["camera"])->clone();
    mProjectionVar = dynamic_cast<Variable*>(Parser::Symbols()["projection"])->clone();
    mCamera = new Camera(Vector4(0, 1.25, 10), Vector4(0, 0, 0), Vector4(0, 1, 0));
    mCameraVar->setValue(QVariant::fromValue(mCamera->trans()));

    mTime = 0;
    mTimeVar = dynamic_cast<Variable*>(Parser::Symbols()["time"])->clone();
    mTimeVar->setValue(QVariant::fromValue(mTime));

    // retrieve blobs
    foreach (QObject *plugin, QPluginLoader::staticInstances()) {
        addBlob(plugin);
    }

    QDir pluginsDir(qApp->applicationDirPath());
    pluginsDir.cd("plugins");

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        addBlob(loader.instance());
    }

    mTimer = new QTimer(this);
    mTimer->setInterval(1000/25);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(move()));
    mAnimTimer = new QTimer(this);
    mAnimTimer->setInterval(1000/25);
    connect(mAnimTimer, SIGNAL(timeout()), this, SLOT(anim()));
}


void Demo::GLWidget::addBlob(QObject* plugin) {
    GL::Blob* blob = qobject_cast<GL::Blob*>(plugin);
    if (blob) {
        if (Parser::Symbols().contains(blob->name())) {
            qWarning() << "Cannot load blob:" << blob->name() << "is a reserved symbol";
            return;
        }
        int index = mBlobs.length();
        Parser::AddSymbol(new Demo::Constant(blob->name(), index));
        mBlobs.append(blob);
        return;
    }

    GL::TexBlob* texBlob = qobject_cast<GL::TexBlob*>(plugin);
    if (texBlob) {
        if (Parser::Symbols().contains(texBlob->name())) {
            qWarning() << "Cannot load blob:" << texBlob->name() << "is a reserved symbol";
            return;
        }
        int index = mTexBlobs.length();
        Parser::AddSymbol(new Demo::Constant(texBlob->name(), index));
        mTexBlobs.append(texBlob);

        return;
    }
}

Demo::GLWidget::~GLWidget() {
    delete mCamera;
    delete mCameraVar;
}


void Demo::GLWidget::initializeGL() {
    initializeGLFunctions();
    defaults();
    emit init();
}

void Demo::GLWidget::paintGL()
{
    emit draw();
}

void Demo::GLWidget::initChanged() {
    makeCurrent();
    initializeGL();
    updateGL();
}

void Demo::GLWidget::drawChanged() {
    updateGL();
}

void Demo::GLWidget::resizeGL(int w, int h) {
    mDim = w; if (h > w) mDim = h;
    glViewport(0, 0, w, h);
    Real a = Real(w) / Real(h);
    Real ct = 1 / tan(Math3D::PI / 180 * 45 / 2);
    Real n = 1;
    Real f = 80;
    mProj.setIdentity();
    // column first
    mProj(0)[0] = ct / a;
    mProj(1)[1] = ct;
    mProj(2)[2] = (n + f) / (n - f);
    mProj(3)[3] = 0;
    mProj(3)[2] = 2*n*f / (n - f);
    mProj(2)[3] = -1;
    mProjectionVar->setValue(QVariant::fromValue(mProj));
    paintGL();
}



void Demo::GLWidget::mousePressEvent(QMouseEvent* event) {
    mDx = mDy = 0;
    mLastPos = event->pos();
    mTimer->stop();
}

void Demo::GLWidget::mouseDoubleClickEvent(QMouseEvent*) {
    mCamera->reset();
    mCameraVar->setValue(QVariant::fromValue(mCamera->trans()));
    mTime = 0;
    mTimeVar->setValue(QVariant::fromValue(mTime));
    updateGL();
}

void Demo::GLWidget::mouseReleaseEvent(QMouseEvent* event) {
    if ((mDx != 0) || (mDy != 0)) {
        delete mMover;
        if (event->modifiers() & Qt::ShiftModifier) {
            mMover = new Zoomer(this);
        } else if (event->modifiers() & Qt::ControlModifier) {
            mMover = new Panner(this);
        } else {
            mMover = new Spinner(this);
        }
        mTimer->start();
    }
}

static float gravity(int dx) {
    float threshold = 2;
    float k = 1.0;
    float cutoff = 50;

    if (abs(dx) >= cutoff) return dx / abs(dx) * k * cutoff ;
    if (abs(dx) >= threshold) return k * dx;

    return 0;
}

void Demo::GLWidget::mouseMoveEvent(QMouseEvent* event) {

    if (event->buttons() & Qt::LeftButton) {
        mDx = event->x() - mLastPos.x();
        mDy = event->y() - mLastPos.y();
        if (event->modifiers() & Qt::ShiftModifier) {
            zoom();
        } else if (event->modifiers() & Qt::ControlModifier) {
            pan();
        } else {
            spin();
        }
        mDx = gravity(mDx);
        mDy = gravity(mDy);
    }
    mLastPos = event->pos();
}


void Demo::GLWidget::spin() {
    float theta = sqrt(mDx*mDx+mDy*mDy) / mDim * 4 * Math3D::PI;
    float phi = atan2f(mDy, mDx);
    mCamera->rotate(phi, theta);
    mCameraVar->setValue(QVariant::fromValue(mCamera->trans()));
    updateGL();
}

void Demo::GLWidget::pan() {
    float theta = sqrt(mDx*mDx+mDy*mDy) / mDim * 4 * Math3D::PI;
    float phi = atan2f(mDy, mDx);
    mCamera->pan(phi, theta);
    mCameraVar->setValue(QVariant::fromValue(mCamera->trans()));
    updateGL();
}

void Demo::GLWidget::zoom() {
    float dz = 0.05 * mDy;
    mCamera->zoom(dz);
    mCameraVar->setValue(QVariant::fromValue(mCamera->trans()));
    updateGL();
}

void Demo::GLWidget::move() {
    mMover->move();
}

void Demo::GLWidget::animStart() {
    mAnimTimer->start();
}

void Demo::GLWidget::animStop() {
    mAnimTimer->stop();
}

void Demo::GLWidget::animReset(int fps) {
    mAnimTimer->setInterval(1000/fps);
}

void Demo::GLWidget::anim() {
    mTime += 1;
    mTimeVar->setValue(QVariant::fromValue(mTime));
    updateGL();
}

#define ALT(item) case item: qDebug() << #item; break


void Demo::GLWidget::defaults() {
    qDebug() << "resetting to defaults";
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_COVERAGE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);

    glEnable(GL_DITHER);

    glDepthRangef(0, 1);
    glLineWidth(1);

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glColorMask(1, 1, 1, 1);
    glDepthMask(1);

    glClearColor(0, 0, 0, 0);
    glClearDepth(1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(0);
    foreach (int name, mResources) {
        if (glIsShader(name)) {
            qDebug() << "deleting shader" << name;
            glDeleteShader(name);
        } else if (glIsProgram(name)) {
            qDebug() << "deleting program" << name;
           glDeleteProgram(name);
        }
    }

    QVector<GLuint> tmp = mBuffers.toVector();
    qDebug() << "deleting buffers" << mBuffers;
    glDeleteBuffers(tmp.size(), tmp.constData());
    tmp = mTextures.toVector();
    qDebug() << "deleting textures" << mTextures;
    glDeleteTextures(tmp.size(), tmp.constData());

    switch (glGetError()) {
        ALT(GL_INVALID_ENUM);
        ALT(GL_INVALID_VALUE);
        ALT(GL_INVALID_OPERATION);
        ALT(GL_STACK_UNDERFLOW);
        ALT(GL_STACK_OVERFLOW);
        ALT(GL_OUT_OF_MEMORY);
        ALT(GL_INVALID_FRAMEBUFFER_OPERATION);
    default: ;
    }

    mResources.clear();
}

#undef ALT

