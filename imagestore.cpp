#include "imagestore.h"
#include <QGLWidget>
#include "logging.h"
#include <QPluginLoader>

using namespace Demo::GL;

ImageStore::ImageStore():
    ProjectFolder("imagestore"),
    TexBlob() {}



const void* ImageStore::readData(const QString& key) const {
    // qCDebug(OGL) << "GL::ImageStore::data" << key;
    if (mImages.contains(key)) {
        // qCDebug(OGL) << "has" << mImages[key].size() << "bytes";
        return mImages[key].bits();
    }
    return nullptr;
}


TexBlobSpec ImageStore::spec(const QString& key) const {
    if (mImages.contains(key)) {
        const QImage& image = mImages[key];
        return TexBlobSpec(image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE);
    }
    return TexBlobSpec();
}

void ImageStore::rename(const QString& from, const QString& to) {
    if (mImages.contains(from)) {
        QImage image = mImages[from];
        mImages.remove(from);
        mImages[to] = image;
        mNames[mNames.indexOf(from)] = to;
    }
}

void ImageStore::remove(int index) {
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


void ImageStore::setItem(const QString& key, const QString& path) {
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



void ImageStore::clean() {
    mNames.clear();
    mFileNames.clear();
    mImages.clear();
}

int ImageStore::size() const {
    return mNames.size();
}

QString ImageStore::fileName(int index) const {
    return mFileNames[index];
}

QString ImageStore::itemName(int index) const {
    return mNames[index];
}

QStringList ImageStore::items() const {
    return mNames;
}
