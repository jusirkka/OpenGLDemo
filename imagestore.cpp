#include "imagestore.h"
#include <QGLWidget>
#include <QDebug>
#include <QPluginLoader>

GL::ImageStore::ImageStore()
    : QObject(),
      TexBlob()
{
    setObjectName("imagestore");
}

static GL::ImageStore* staticInstance() {
    foreach (QObject *plugin, QPluginLoader::staticInstances()) {
        GL::ImageStore* store = qobject_cast<GL::ImageStore*>(plugin);
        if (store) return store;
    }
    return 0;
}

GL::ImageStore* GL::ImageStore::instance() {
    static GL::ImageStore* store = staticInstance();
    return store;
}

void GL::ImageStore::Clean() {
    instance()->clean();
}

void GL::ImageStore::SetImage(const QString& key, const QString& path) {
    instance()->setImage(key, path);
}

int GL::ImageStore::Size() {
    return instance()->size();
}

const QString& GL::ImageStore::ImageName(int idx) {
    return instance()->imageName(idx);
}

const QString& GL::ImageStore::FileName(int idx) {
    return instance()->fileName(idx);
}

void GL::ImageStore::Rename(const QString& from, const QString& to) {
    instance()->rename(from, to);
}

void GL::ImageStore::Remove(int index) {
    instance()->remove(index);
}


const void* GL::ImageStore::data(const QString& key) const {
    qDebug() << "GL::ImageStore::data" << key;
    if (mImages.contains(key)) {
        qDebug() << "has" << mImages[key].size() << "bytes";
        return mImages[key].bits();
    }
    return 0;
}

const GL::TexBlobSpec GL::ImageStore::spec(const QString& key) const {
    if (mImages.contains(key)) {
        const QImage& image = mImages[key];
        return TexBlobSpec(image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE);
    }
    return TexBlobSpec();
}

void GL::ImageStore::rename(const QString& from, const QString& to) {
    if (mImages.contains(from)) {
        QImage image = mImages[from];
        mImages.remove(from);
        mImages[to] = image;
        mNames[mNames.indexOf(from)] = to;
    }
}

void GL::ImageStore::remove(int index) {
    mImages.remove(mNames[index]);
    mNames.removeAt(index);
    mFileNames.removeAt(index);
}


static const char* const plaid[] = {
    /* plaid pixmap
         * width height ncolors chars_per_pixel */
    "22 22 5 2",
    /* colors */
    ".  c red       m white  s light_color ",
    "Y  c green     m black  s lines_in_mix ",
    "+  c yellow    m white  s lines_in_dark ",
    "x              m black  s dark_color ",
    "   c none               s mask ",
    /* pixels */
    "x . x . x + x . x . x x x x x x + x x x x x ",
    ". x . x . x . x . x . x x x x x x x x x x x ",
    "x . x . x + x . x . x x x x x x + x x x x x ",
    ". x . x . x . x . x . x x x x x x x x x x x ",
    "x . x . x + x . x . x x x x x x + x x x x x ",
    "x x x x x x x x x x x + x + x + x + x + x + ",
    ". x . x . x x . x . x x x x x x + x x x x x ",
    "x . x . x + . x . x . x x x x x x x x x x x ",
    ". x . x . x . x . x . x x x x x + x x x x x ",
    "x . x . x + x . x . x x x x x x x x x x x x ",
    "x . x . x x x . x . x x x x x x + x x x x x ",
    ". . . . . x . . . . . x . x . x Y x . x . x ",
    ". . . . . x . . . . . . x . x . Y . x . x . ",
    ". . . . . x . . . . . x . x . x Y x . x . x ",
    ". . . . . x . . . . . . x . x . Y . x . x . ",
    ". . . . . x . . . . . x . x . x Y x . x . x ",
    "x x x x x x x x x x x x x x x x x x x x x x ",
    ". . . . . x . . . . . x . x . x Y x . x . x ",
    ". . . . . x . . . . . . x . x . Y . x . x . ",
    ". . . . . x . . . . . x . x . x Y x . x . x ",
    ". . . . . x . . . . . . x . x . Y . x . x . ",
    ". . . . . x . . . . . x . x . x Y x . x . x "
};


void GL::ImageStore::setImage(const QString& key, const QString& path) {
    if (mImages.contains(key)) {
        mFileNames[mNames.indexOf(key)] = path;
    } else {
        mNames.append(key);
        mFileNames.append(path);
    }
    QImage image(plaid);
    if (!path.isEmpty()) {
        image = QImage(path);
    }
    mImages[key] = QGLWidget::convertToGLFormat(image);
}



void GL::ImageStore::clean() {
    mNames.clear();
    mFileNames.clear();
    mImages.clear();
}

int GL::ImageStore::size() {
    return mNames.size();
}

const QString& GL::ImageStore::fileName(int index) {
    return mFileNames[index];
}

const QString& GL::ImageStore::imageName(int index) {
    return mNames[index];
}

GL::ImageStore::~ImageStore() {}

Q_EXPORT_PLUGIN2(pnp_gl_imagestore, GL::ImageStore)
