#include "gl_widget.h"
#include "gl_lang_compiler.h"
#include "gl_lang_runner.h"
#include "gl_functions.h"
#include "camera.h"
#include "imagestore.h"
#include "modelstore.h"
#include "videoencoder.h"
#include "downloader.h"
#include "logging.h"

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
    CameraConstraint(GLWidget* p)
        : Function("cameraconstraint", new Integer_T)
        , mParent(p)
    {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        bool doit = ! vals[start].toBool();
        // qCDebug(OGL) << "cameraconstraint" << doit << vals[start].toInt();
        if (doit) {
            mParent->cameraStop();
        }
        mValue.setValue(doit);
        return mValue;
    }

    CameraConstraint* clone() const override {
        return new CameraConstraint(*this);
    }

private:

    GLWidget* mParent;
};

class DefaultFrameBuffer: public Function {
public:
    DefaultFrameBuffer(GLWidget* p)
        : Function("defaultframebuffer", new Integer_T)
        , mParent(p) {}

    const QVariant& execute(const QVector<QVariant>&, int) override {
        GLuint fbo = mParent->defaultFramebufferObject();
        // qCDebug(OGL) << "defaultframebuffer" << fbo;
        mValue.setValue(fbo);
        return mValue;
    }

    DefaultFrameBuffer* clone() const override {
        return new DefaultFrameBuffer(*this);
    }

private:
    GLWidget* mParent;
};

class Paused: public Function {
public:
    Paused(GLWidget* p)
        : Function("paused", new Integer_T)
        , mParent(p) {}

    const QVariant& execute(const QVector<QVariant>&, int) override {
        int v = mParent->animRunning() ? 0 : 1;
        // qCDebug(OGL) << "paused" << v;
        mValue.setValue(v);
        return mValue;
    }

    Paused* clone() const override {
        return new Paused(*this);
    }

private:
    GLWidget* mParent;
};

class RegisterSource: public Function {
public:
    RegisterSource(GLWidget* p)
        : Function("registerdatasource", new Integer_T)
        , mParent(p) {
        mArgTypes.append(new Text_T);
    }

    const QVariant& execute(const QVector<QVariant>& values, int start) override {
        QString device = values[start].toString();
        mValue.setValue(mParent->registerDataSource(device));
        return mValue;
    }

    RegisterSource* clone() const override {
        return new RegisterSource(*this);
    }

private:
    GLWidget* mParent;
};

class ReadFromSource: public Function {
public:
    ReadFromSource(GLWidget* p)
        : Function("readfromsource", new ArrayType(new Real_T))
        , mParent(p) {
        mArgTypes.append(new Integer_T);
    }

    const QVariant& execute(const QVector<QVariant>& values, int start) override {
        int device = values[start].toInt();
        auto data = mParent->readData(device);
        QVariantList list;
        for (auto v: data) list.append(QVariant::fromValue(v));
        mValue.setValue(list);
        return mValue;
    }

    ReadFromSource* clone() const override {
        return new ReadFromSource(*this);
    }

private:
    GLWidget* mParent;
};


#define ALT(item) case item: return QString(#item);

static QString checkError() {
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
    return QString();
}

#undef ALT

#define CHECK_GL do {QString msg = checkError(); if (!msg.isEmpty()) {qCWarning(OGL) << msg;}} while (false)


#define updateGL update

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , OpenGLFunctions()
    , mInitialized(false)
    , mDim(500)
    , mMover(new Mover(this))
    , mNear(.1)
    , mFar(500)
    , mLastOp(None)
    , mEncoder(nullptr)
    , mDownloader(nullptr)
    , mRecording(false)
    , mMaxFrames(25*60)
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
    func = new Paused(this);
    globals[func->name()] = func;
    func = new RegisterSource(this);
    globals[func->name()] = func;
    func = new ReadFromSource(this);
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
    for (DataCache& c: mDataCache.values()) {
        c.source->stop();
        c.source->wait(1000);
        delete c.source;
    }
}


void Demo::GLWidget::initializeGL() {
    if (!mInitialized) {
        qCInfo(OGL) << "initializeOpenGLFunctions";
        if (!initializeOpenGLFunctions()) {
            qFatal("initializeOpenGLFunctions failed");
        }
        mInitialized = true;
        emit openGLReady(mInitialized);
    }
}

void Demo::GLWidget::saveToDisk(bool on, const QString& basePath) {
    if (on) {
        if (mRecording) return;
        mRecording = true;
        mDownloader = new GL::Downloader(this);
        mEncoder = new VideoEncoder(mDownloader, basePath);
        connect(mEncoder, SIGNAL(finished()), this, SLOT(encodingFinished()));
        mEncoder->start();
    } else {
        mEncoder->stop();
    }
}

void Demo::GLWidget::encodingFinished() {
    qCDebug(OGL) << "encoding finished";
    mRecording = false;
    delete mDownloader;
    delete mEncoder;
}


int Demo::GLWidget::registerDataSource(const QString& device) {
    int id = 0;
    while (mDataCache.contains(id)) id++;
    mDataCache[id] = DataCache(id, device, mMaxFrames);
    connect(mDataCache[id].source, SIGNAL(finished()), this, SLOT(dataSourceClosed()));
    mDataCache[id].source->start();
    return id;
}

const Demo::GLWidget::DataVector& Demo::GLWidget::readData(int device) {
    if (!mDataCache.contains(device)) {
        throw RunError(QString("%1: no such cache id").arg(device), 0);
    }
    auto src = mDataCache[device].source;

    if (!src->hasFilledFrames()) {
        if (mDataCache[device].data.isEmpty()) {
            throw RunError(QString("%1: no data").arg(device), 0);
        }
        return mDataCache[device].data;
    }

    src->acquireFilledFrame();
    int slot = mDataCache[device].currFrame;
    mDataCache[device].currFrame = (slot + 1) % mMaxFrames;

    DataVector& data = src->buffer()[slot];
    mDataCache[device].data = data;
    data.clear();

    src->releaseEmptyFrame();

    return mDataCache[device].data;
}

void Demo::GLWidget::dataSourceClosed() {
    qCDebug(OGL) << "data source closed";
    auto src = qobject_cast<DataSource*>(sender());
    mDataCache.remove(src->id());
    delete src;
}


void Demo::GLWidget::paintGL()
{
    emit draw();
    if (!mRecording) return;
    mDownloader->readFrame();
}

void Demo::GLWidget::initChanged() {
    if (!mInitialized) return;
    makeCurrent();
    CHECK_GL;
    defaults();
    emit init();
    updateGL();
}

void Demo::GLWidget::drawChanged() {
    makeCurrent();
    CHECK_GL;
    updateGL();
}

void Demo::GLWidget::resizeGL(int w, int h) {
    mWidthVar->setValue(QVariant::fromValue(w));
    mHeightVar->setValue(QVariant::fromValue(h));
    mDim = w; if (h > w) mDim = h;
    mResizeTimer->start();
}

void Demo::GLWidget::realResize() {
    // qCDebug(OGL) << "resizing";
    int w = mWidthVar->value().toInt();
    int h = mHeightVar->value().toInt();
    glViewport(0, 0, w, h);
    CHECK_GL;
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

bool Demo::GLWidget::animRunning() const {
    return mAnimTimer->isActive();
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
    } else if (ev->key() == Qt::Key_Escape) {
        hide();
    }
}


GLuint Demo::GLWidget::resource(const QString &res, GLenum param) {
    if (res == "texture") {
        auto r = new Texture(this);
        CHECK_GL;
        mResources[QString("texture_%1").arg(r->name)] = r;
        return r->name;
    }
    if (res == "shader") {
        auto r = new Shader(this, param);
        mResources[QString("shader_%1").arg(r->name)] = r;
        CHECK_GL;
        return r->name;
    }
    if (res == "program") {
        auto r = new Program(this);
        mResources[QString("program_%1").arg(r->name)] = r;
        CHECK_GL;
        return r->name;
    }
    if (res == "buffer") {
        auto r = new Buffer(this);
        mResources[QString("buffer_%1").arg(r->name)] = r;
        CHECK_GL;
        return r->name;
    }
    if (res == "frame_buffer") {
        auto r = new FrameBuffer(this);
        mResources[QString("frame_buffer_%1").arg(r->name)] = r;
        CHECK_GL;
        return r->name;
    }
    if (res == "vertex_array") {
        auto r = new VertexArray(this);
        mResources[QString("vertex_array_%1").arg(r->name)] = r;
        CHECK_GL;
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
    CHECK_GL;
}


void Demo::GLWidget::defaults() {
    // qCDebug(OGL) << "resetting to defaults";
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
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_1D, 0);
    glBindVertexArray(0);

    qDeleteAll(mResources);
    mResources.clear();

    CHECK_GL;

    for (DataCache& c: mDataCache.values()) {
        c.source->stop();
    }
}


