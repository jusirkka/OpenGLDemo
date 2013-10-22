#include "gl_widget.h"
#include "parser.h"
#include "runner.h"
#include "gl_functions.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPluginLoader>
#include <QDir>
#include <QApplication>
#include <QMatrix4x4>


Demo::GLWidget::GLWidget(QWidget *parent):
    QGLWidget(parent),
    QGLFunctions(),
    mInitialized(false)
{
    mRunners[InitKey] = 0;
    mRunners[DrawKey] = 0;

    GL::Functions funcs(this);
    foreach(Symbol* func, funcs.contents) Parser::AddSymbol(func);
    GL::Constants constants;
    foreach(Symbol* c, constants.contents) Parser::AddSymbol(c);

    mCamera = dynamic_cast<Variable*>(Parser::Symbols()["camera"])->clone();

    QMatrix4x4 m;
    m.ortho(-4.0f, +4.0f, -4.0f, +4.0f, 0.1f, 100.0f);
    m.translate(0.0f, -0.5f, -7.5f);
    m.rotate(25, 1, 0, 0);

    Math3D::Matrix4 c(m.data());
    mCamera->setValue(QVariant::fromValue(c));

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

}


void Demo::GLWidget::addBlob(QObject* plugin) {
    GL::Blob* blob = qobject_cast<GL::Blob*>(plugin);
    if (!blob) return;
    if (Parser::Symbols().contains(blob->name())) {
        qWarning() << "Cannot load blob:" << blob->name() << "is a reserved symbol";
        return;
    }
    int index = mBlobs.length();
    Parser::AddSymbol(new Demo::Constant(blob->name(), index));
    mBlobs.append(blob);
}

Demo::GLWidget::~GLWidget() {
    foreach (Runner* r, mRunners.values()) {
        delete r;
    }
}


void Demo::GLWidget::initializeGL () {
    initializeGLFunctions();
}

void Demo::GLWidget::paintGL()
{
    qDebug() << "Entering paintGL";
    foreach(int key, mRunners.keys()) {
        Runner* runner = mRunners[key];
        if (!runner) continue;

        if (key == InitKey) {
            if (mInitialized) continue;
            defaults();
            mInitialized = runner->evaluate();
        } else {
            runner->evaluate();
        }
    }
    qDebug() << "Leaving paintGL";
}


void Demo::GLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    paintGL();
}


void Demo::GLWidget::mousePressEvent(QMouseEvent *event) {
//    mDx = mDy = 0;
//    mLastPos = event->pos();
//    mGravity = false;
}

void Demo::GLWidget::mouseReleaseEvent(QMouseEvent *event) {
//    mGravity = (mDx != 0) || (mDy != 0);
}

static float gravity(int dx) {
    float threshold = 3;
    float k = 2.0;
    float cutoff = 10;

    if (abs(dx) >= cutoff) return dx / abs(dx) * k * cutoff ;
    if (abs(dx) >= threshold) return k * dx;

    return 0;
}

void Demo::GLWidget::mouseMoveEvent(QMouseEvent *event) {

//    if (event->buttons() & Qt::LeftButton) {
//        mDx = gravity(event->x() - mLastPos.x());
//        mDy = gravity(event->y() - mLastPos.y());
//        qDebug() << mDx << mDy;
//    }
//    mLastPos = event->pos();
}



void Demo::GLWidget::parse(int key, const QString& commands) {
//    qDebug() << "GLWidget::parse";
//    qDebug() << commands;

    if (key == InitKey) {
        mInitialized = false;
    }

    delete mRunners[key];
    mRunners[key] = 0;

    if (Parser::ParseIt(commands)) {
        mRunners[key] = Parser::CreateRunner();
        updateGL();
    }
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

    ResourceList remaining;
    foreach (int name, mResources) {
        if (glIsShader(name)) {
            glDeleteShader(name);
        } else if (glIsProgram(name)) {
           glDeleteProgram(name);
        } else {
            remaining.append(name);
        }
    }

    glDeleteTextures(remaining.size(), remaining.toVector().constData());
    glDeleteBuffers(remaining.size(), remaining.toVector().constData());

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
