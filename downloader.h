#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QtOpenGL/QGL>
#include <QSemaphore>

namespace Demo {

class GLWidget;

namespace GL {

class Downloader {

public:
    using FrameVector = QVector<QByteArray>;

    Downloader(GLWidget* parent);

    void readFrame();

    void acquireFilledFrame();
    void releaseEmptyFrame();

    int bufferSize() const;
    FrameVector& buffer();

    GLsizei width() const {return mWidth;}
    GLsizei height() const {return mHeight;}
    GLsizeiptr frameSize() const {return mFrameSize;}


    ~Downloader();

private:

    using PBOVector = QVector<GLuint>;

    GLWidget* mParent;
    int mPBOSize;
    int mPBOIndex;
    PBOVector mPBOs;
    int mBufferSize;
    int mBufferIndex;
    FrameVector mBuffer;
    QSemaphore mFilled;
    QSemaphore mEmpty;
    GLsizei mWidth;
    GLsizei mHeight;
    GLenum mFormat;
    GLenum mType;
    GLsizeiptr mFrameSize;

};

}}

#endif // DOWNLOADER_H
