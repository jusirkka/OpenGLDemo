#include "gl_widget.h"
#include "gl_lang_compiler.h"
#include "gl_lang_runner.h"
#include "gl_functions.h"
#include "camera.h"
#include "imagestore.h"
#include "modelstore.h"

#include <QDebug>
#include <QMouseEvent>
#include <QHideEvent>
#include <QPluginLoader>
#include <QDir>
#include <QApplication>
#include <QTimer>

using Math3D::Matrix4;
using Math3D::Vector4;
using Math3D::Real;

using namespace Demo;


class CameraConstraint: public Function {
public:
    CameraConstraint(GLWidget* p);
    const QVariant& execute(const QVector<QVariant>& vals, int start) override;
    CameraConstraint* clone() const override;
private:
    GLWidget* mParent;
};


CameraConstraint::CameraConstraint(GLWidget* p)
    : Function("cameraconstraint", new Integer_T)
    , mParent(p)
{
    mArgTypes.append(new Integer_T);
}

const QVariant& CameraConstraint::execute(const QVector<QVariant>& vals, int start) {
    bool doit = ! vals[start].toBool();
    // qDebug() << "cameraconstraint" << doit << vals[start].toInt();
    if (doit) {
        mParent->cameraStop();
    }
    mValue.setValue(doit);
    return mValue;
}

CameraConstraint* CameraConstraint::clone() const {
    return new CameraConstraint(*this);
}

class DefaultFrameBuffer: public Function {
public:
    DefaultFrameBuffer(GLWidget* p);
    const QVariant& execute(const QVector<QVariant>& vals, int start) override;
    DefaultFrameBuffer* clone() const override;
private:
    GLWidget* mParent;
};


DefaultFrameBuffer::DefaultFrameBuffer(GLWidget* p)
    : Function("defaultframebuffer", new Integer_T)
    , mParent(p)
{
}

const QVariant& DefaultFrameBuffer::execute(const QVector<QVariant>&, int) {
    GLuint fbo = mParent->defaultFramebufferObject();
    // qDebug() << "defaultframebuffer" << fbo;
    mValue.setValue(fbo);
    return mValue;
}

DefaultFrameBuffer* DefaultFrameBuffer::clone() const {
    return new DefaultFrameBuffer(*this);
}


#define updateGL update

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , OpenGLFunctions()
    , mInitialized(false)
    , mDim(500)
    , mMover(new Mover(this))
    , mNear(1)
    , mFar(500)
    , mLastOp(None)
{

    mTime = 0;
    mCamera = new Camera(Vector4(0, 0, 30), Vector4(0, 0, 0), Vector4(0, 1, 0));

    mTimer = new QTimer(this);
    mTimer->setInterval(1000/25);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(move()));
    mAnimTimer = new QTimer(this);
    mAnimTimer->setInterval(1000/25);
    connect(mAnimTimer, SIGNAL(timeout()), this, SLOT(anim()));
    mResizeTimer = new QTimer(this);
    mResizeTimer->setInterval(500);
    mResizeTimer->setSingleShot(true);
    connect(mResizeTimer, SIGNAL(timeout()), this, SLOT(realResize()));
}

void GLWidget::addGLSymbols(SymbolMap& globals, VariableMap& exports) {

    GL::Functions funcs(this);
    for (auto func: qAsConst(funcs.contents)) globals[func->name()] = func;
    GL::Constants constants;
    for (auto c: qAsConst(constants.contents)) globals[c->name()] = c;

    Function* func = new CameraConstraint(this);
    globals[func->name()] = func;
    func = new DefaultFrameBuffer(this);
    globals[func->name()] = func;


    // shared matrices
    exports["camera"] = new SharedVar("camera", new Matrix_T);
    exports["projection"] = new SharedVar("projection", new Matrix_T);
    exports["inverse_projection"] = new SharedVar("inverse_projection", new Matrix_T);
    // shared time variable
    exports["time"] = new SharedVar("time", new Integer_T);
    // shared dimensions
    exports["width"] = new SharedVar("width", new Integer_T);
    exports["height"] = new SharedVar("height", new Integer_T);

    mCameraVar = exports["camera"]->clone();
    mCameraVar->setValue(QVariant::fromValue(mCamera->trans()));

    mProjectionVar = exports["projection"]->clone();

    mInvProjVar = exports["inverse_projection"]->clone();

    mTimeVar = exports["time"]->clone();
    mTimeVar->setValue(QVariant::fromValue(mTime));

    mWidthVar = exports["width"]->clone();
    mWidthVar->setValue(QVariant::fromValue(width()));

    mHeightVar = exports["height"]->clone();
    mHeightVar->setValue(QVariant::fromValue(height()));


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
        // qDebug() << "initializeOpenGLFunctions";
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
    if (!mInitialized) return;
    makeCurrent();
    defaults();
    emit init();
    updateGL();
}

void Demo::GLWidget::drawChanged() {
    updateGL();
}

void Demo::GLWidget::resizeGL(int w, int h) {
    mWidthVar->setValue(QVariant::fromValue(w));
    mHeightVar->setValue(QVariant::fromValue(h));
    mDim = w; if (h > w) mDim = h;
    mResizeTimer->start();
}

void Demo::GLWidget::realResize() {
    // qDebug() << "resizing";
    int w = mWidthVar->value().toInt();
    int h = mHeightVar->value().toInt();
    glViewport(0, 0, w, h);

    Real a = Real(w) / Real(h);
    Real ct = 1 / tan(Math3D::PI / 180 * 45 / 2);
    Real d = (mNear + mFar) / (mNear - mFar);
    Real e = 2*mNear*mFar / (mNear - mFar);

    Matrix4 proj;
    proj.setIdentity();
    // column first
    proj(0)[0] = ct / a;
    proj(1)[1] = ct;
    proj(2)[2] = d;
    proj(3)[3] = 0;
    proj(3)[2] = e;
    proj(2)[3] = -1;
    mProjectionVar->setValue(QVariant::fromValue(proj));

    Matrix4 invproj;
    invproj.setIdentity();
    // column first
    invproj(0)[0] = a / ct;
    invproj(1)[1] = 1 / ct;
    invproj(2)[2] = 0;
    invproj(3)[3] = d / e;
    invproj(3)[2] = -1;
    invproj(2)[3] = 1 / e;
    mInvProjVar->setValue(QVariant::fromValue(invproj));

    // init statements might depend on the viewport or projection
    initChanged();
}

void Demo::GLWidget::setProjection(float near, float far) {
    mNear = near;
    mFar = far;
    // check sanity
    if (mNear < Math3D::EPSILON) {
        mNear = Math3D::EPSILON;
    }
    if (mFar - mNear < Math3D::EPSILON) {
        mFar = mNear + Math3D::EPSILON;
    }
    if (mInitialized) { // setProjection might be called before we have been initialized
        realResize();
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
    mLastOp = Spin;
    updateGL();
}

void Demo::GLWidget::pan() {
    float theta = sqrt(mDx*mDx+mDy*mDy) / mDim * 4 * Math3D::PI;
    float phi = atan2f(mDy, mDx);
    mCamera->pan(phi, theta);
    mCameraVar->setValue(QVariant::fromValue(mCamera->trans()));
    mLastOp = Pan;
    updateGL();
}

void Demo::GLWidget::zoom() {
    float dz = 0.05 * mDy;
    mCamera->zoom(dz);
    mCameraVar->setValue(QVariant::fromValue(mCamera->trans()));
    mLastOp = Zoom;
    updateGL();
}

void Demo::GLWidget::move() {
    mMover->move();
}

void Demo::GLWidget::cameraStop() {
    if (mTimer->isActive()) mTimer->stop();
    // revert last op
    mDx = -mDx;
    mDy = -mDy;
    switch (mLastOp) {
    case Pan: pan(); break;
    case Spin: spin(); break;
    case Zoom: zoom(); break;
    default: break;
    }
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

void Demo::GLWidget::hideEvent(QHideEvent*) {
    if (mTimer->isActive()) mTimer->stop();
    emit hidden();
}

void Demo::GLWidget::keyPressEvent(QKeyEvent* ev) {
    if (ev->key() == Qt::Key_Space) {
        emit toggleAnimate();
    }
}


GLuint Demo::GLWidget::resource(const QString &res, GLenum param) {
    if (res == "texture") {
        auto r = new Texture(this);
        mResources[QString("texture_%1").arg(r->name)] = r;
        return r->name;
    }
    if (res == "shader") {
        auto r = new Shader(this, param);
        mResources[QString("shader_%1").arg(r->name)] = r;
        return r->name;
    }
    if (res == "program") {
        auto r = new Program(this);
        mResources[QString("program_%1").arg(r->name)] = r;
        return r->name;
    }
    if (res == "buffer") {
        auto r = new Buffer(this);
        mResources[QString("buffer_%1").arg(r->name)] = r;
        return r->name;
    }
    if (res == "frame_buffer") {
        auto r = new FrameBuffer(this);
        mResources[QString("frame_buffer_%1").arg(r->name)] = r;
        return r->name;
    }
    if (res == "vertex_array") {
        auto r = new VertexArray(this);
        mResources[QString("vertex_array_%1").arg(r->name)] = r;
        return r->name;
    }
    return 0;
}


void Demo::GLWidget::deresource(const QString &res, GLuint name) {
    QString key = QString("%1_%2").arg(res).arg(name);
    if (mResources.contains(key)) {
        delete mResources[key];
        mResources.remove(key);
    }
}

/*
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
*/

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
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    qDeleteAll(mResources);
    mResources.clear();
}


