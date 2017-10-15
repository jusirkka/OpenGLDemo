#include "gl_widget.h"
#include "gl_lang_compiler.h"
#include "gl_lang_runner.h"
#include "gl_functions.h"
#include "camera.h"
#include "imagestore.h"
#include "shadowmap.h"
#include "modelstore.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPluginLoader>
#include <QDir>
#include <QApplication>
#include <QTimer>

using Math3D::Matrix4;
using Math3D::Vector4;
using Math3D::Real;

using namespace Demo;

// #define updateGL update

GLWidget::GLWidget(QWidget *parent):
    QGLWidget(parent),
    QOpenGLFunctions_3_0(),
    mInitialized(false),
    mDim(500),
    mMover(new Mover(this)),
    mNear(1),
    mFar(500)
{

    mTime = 0;
    mCamera = new Camera(Vector4(10, 10, 60), Vector4(0, 0, 0), Vector4(0, 1, 0));

    mTimer = new QTimer(this);
    mTimer->setInterval(1000/25);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(move()));
    mAnimTimer = new QTimer(this);
    mAnimTimer->setInterval(1000/25);
    connect(mAnimTimer, SIGNAL(timeout()), this, SLOT(anim()));

}

void GLWidget::addGLSymbols(SymbolMap& globals, VariableMap& exports) {

    GL::Functions funcs(this);
    for (auto func: qAsConst(funcs.contents)) globals[func->name()] = func;
    GL::Constants constants;
    for (auto c: qAsConst(constants.contents)) globals[c->name()] = c;

    // shared matrices
    exports["camera"] = new Var::Shared::Matrix("camera");
    exports["projection"] = new Var::Shared::Matrix("projection");
    // shared time variable
    exports["time"] = new Var::Shared::Natural("time");

    mCameraVar = exports["camera"]->clone();
    mCameraVar->setValue(QVariant::fromValue(mCamera->trans()));

    mTimeVar = exports["time"]->clone();
    mTimeVar->setValue(QVariant::fromValue(mTime));

    mProjectionVar = exports["projection"]->clone();

    // retrieve blobs
    const auto& statics = QPluginLoader::staticInstances();
    for (auto plugin: statics) {
        addBlob(plugin, globals);
    }

    QDir pluginsDir(qApp->applicationDirPath());
    pluginsDir.cd("plugins");

    for (auto& fileName: pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        addBlob(loader.instance(), globals);
    }

    auto shadowmap = dynamic_cast<GL::ShadowMap*>(texBlob(globals, "shadowmap"));
    if (shadowmap) {
        connect(this, SIGNAL(viewportChanged(GLuint,GLuint)), shadowmap, SLOT(viewportChanged(GLuint,GLuint)));
        shadowmap->viewportChanged(width(), height());
    }

    auto modelstore = dynamic_cast<GL::ModelStore*>(blob(globals, "modelstore"));
    if (modelstore) {
        modelstore->setContext(this);
    }


}

void GLWidget::addBlob(QObject* plugin, SymbolMap& globals) {
    auto blob = qobject_cast<GL::Blob*>(plugin);
    if (blob) {
        if (globals.contains(blob->name())) {
            qWarning() << "Cannot load blob:" << blob->name() << "is a reserved symbol";
            return;
        }
        int index = mBlobs.length();
        globals[blob->name()] = new Demo::Constant(blob->name(), index);
        mBlobs.append(blob);
        return;
    }

    auto texBlob = qobject_cast<GL::TexBlob*>(plugin);
    if (texBlob) {
        if (globals.contains(texBlob->name())) {
            qWarning() << "Cannot load blob:" << texBlob->name() << "is a reserved symbol";
            return;
        }
        int index = mTexBlobs.length();
        globals[texBlob->name()] = new Demo::Constant(texBlob->name(), index);
        mTexBlobs.append(texBlob);

        return;
    }
}

static int findIndex(const SymbolMap& globals, const QString& name) {
    if (!globals.contains(name)) return -1;
    auto c = dynamic_cast<Constant*>(globals[name]);
    if (!c) return -1;
    bool ok;
    int index = c->value().toInt(&ok);
    if (!ok) return -1;
    return index;
}

GL::Blob* Demo::GLWidget::blob(const SymbolMap& globals, const QString& name) const {
    int index = findIndex(globals, name);
    if (index < 0 || index >= mBlobs.size()) return nullptr;
    return mBlobs[index];
}

GL::TexBlob* Demo::GLWidget::texBlob(const SymbolMap& globals, const QString& name) const {
    int index = findIndex(globals, name);
    if (index < 0 || index >= mTexBlobs.size()) return nullptr;
    return mTexBlobs[index];
}

Demo::GLWidget::~GLWidget() {
    delete mCamera;
}


void Demo::GLWidget::initializeGL() {
    if (!mInitialized) {
        qDebug() << "initializeOpenGLFunctions";
        if (!initializeOpenGLFunctions()) {
            qFatal("initializeOpenGLFunctions failed");
        }
        mInitialized = true;
    }
}

void Demo::GLWidget::paintGL()
{
    emit draw();
}

void Demo::GLWidget::initChanged() {
    if (mInitialized) {
        makeCurrent();
        defaults();
        emit init();
        updateGL();
    }
}

void Demo::GLWidget::drawChanged() {
    updateGL();
}

void Demo::GLWidget::resizeGL(int w, int h) {
    makeCurrent();
    mDim = w; if (h > w) mDim = h;
    glViewport(0, 0, w, h);
    Real a = Real(w) / Real(h);
    Real ct = 1 / tan(Math3D::PI / 180 * 45 / 2);
    mProj.setIdentity();
    // column first
    mProj(0)[0] = ct / a;
    mProj(1)[1] = ct;
    mProj(2)[2] = (mNear + mFar) / (mNear - mFar);
    mProj(3)[3] = 0;
    mProj(3)[2] = 2*mNear*mFar / (mNear - mFar);
    mProj(2)[3] = -1;
    mProjectionVar->setValue(QVariant::fromValue(mProj));
    emit viewportChanged(w, h);
    // init statements might depend on the viewport or projection
    initChanged();
}

void Demo::GLWidget::setProjection(float near, float far) {
    mNear = near;
    mFar = far;
    // check sanity
    if (mNear < 0.001) {
        mNear = 0.001;
    }
    if (mFar - mNear < 0.001) {
        mFar = mNear + 0.001;
    }
    if (mInitialized) { // setProjection might be called before we have been initialized
        resizeGL(width(), height());
        updateGL();
    }
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

#define ALT(item) case item: qFatal(#item); break

static void checkError() {
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
}

#undef ALT

void Demo::GLWidget::defaults() {
    // qDebug() << "resetting to defaults";
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDisable(GL_SAMPLE_COVERAGE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);

    glEnable(GL_DITHER);

    glDepthRange(0, 1);
    glLineWidth(1);

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    glColorMask(1, 1, 1, 1);
    glDepthMask(1);

    glClearColor(0, 0, 0, 0);
    glClearDepth(1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    // qDebug() << "clearing resources" << mResources;
    for (auto name: qAsConst(mResources)) {
        if (glIsShader(name)) {
            // qDebug() << "deleting shader" << name;
            glDeleteShader(name);
        } else if (glIsProgram(name)) {
            // qDebug() << "deleting program" << name;
           glDeleteProgram(name);
        } else if (glIsBuffer(name)) {
            // qDebug() << "deleting buffer" << name;
            glDeleteBuffers(1, &name);
        } else if (glIsFramebuffer(name)) {
            // qDebug() << "deleting frame buffer" << name;
            glDeleteFramebuffers(1, &name);
        } else if (glIsTexture(name)) {
            // qDebug() << "deleting texture" << name;
            glDeleteTextures(1, &name);
        } else {
            qWarning() << "Unknown resource" << name;
        }
        checkError();
    }
    // qDebug() << "resources cleared";
    mResources.clear();
}


