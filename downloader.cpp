#include "downloader.h"
#include "gl_widget.h"

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

#define CHECK(msg) do {msg = checkError(); if (!msg.isEmpty()) {qCWarning(OGL) << msg;}} while (false)

using namespace Demo::GL;

Downloader::Downloader(GLWidget* p)
    : mParent(p)
    , mPBOSize(2)
    , mPBOIndex(0)
    , mPBOs(mPBOSize)
    , mBufferSize(25*10)
    , mBufferIndex(0)
    , mBuffer(mBufferSize)
    , mFilled(0)
    , mEmpty(mBufferSize)
    , mWidth(mParent->width())
    , mHeight(mParent->height())
    , mFormat(GL_RGB)
    , mType(GL_UNSIGNED_BYTE)
    , mFrameSize(mWidth * mHeight * 3) // RGB, 8 bits / color
{
    mParent->makeCurrent();
    QString err;

    // GLint buf;
    // mParent->glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &buf);
    // qCDebug(OGL) << "GL_PIXEL_PACK_BUFFER_BINDING" << buf;

    mParent->glGenBuffers(mPBOSize, mPBOs.data());
    for (auto pbo: mPBOs) {
        mParent->glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        CHECK(err);
        mParent->glBufferData(GL_PIXEL_PACK_BUFFER, mFrameSize, 0, GL_STREAM_READ);
        CHECK(err);
    }
    mParent->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

}

void Downloader::readFrame() {

    int nextPBO = (mPBOIndex + 1) % mPBOSize;

    QString err;

    mEmpty.acquire();

    mParent->glPixelStorei(GL_PACK_ALIGNMENT, 1);
    CHECK(err);
    // qCDebug(OGL) << "readframe" << mPBOs[mPBOIndex] << mBufferIndex;

    // GLint buf;
    // mParent->glGetIntegerv(GL_READ_BUFFER, &buf);
    // qCDebug(OGL) << "GL_READ_BUFFER" << buf << GL_COLOR_ATTACHMENT0;
    // mParent->glReadBuffer(GL_FRONT);
    mParent->glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[mPBOIndex]);
    CHECK(err);
    mParent->glReadPixels(0, 0, mWidth, mHeight, mFormat, mType, 0);
    CHECK(err);

    mParent->glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[nextPBO]);
    CHECK(err);
    auto src = reinterpret_cast<const char*>(mParent->glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
    CHECK(err);

    if (src) {
        mBuffer[mBufferIndex].append(src, mFrameSize);
    }

    mParent->glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    CHECK(err);

    mParent->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    CHECK(err);

    mPBOIndex = (mPBOIndex + 1) % mPBOSize;
    mBufferIndex = (mBufferIndex + 1) % mBufferSize;

    mFilled.release();
}

void Downloader::acquireFilledFrame() {
    mFilled.acquire();
}

void Downloader::releaseEmptyFrame() {
    mEmpty.release();
}

int Downloader::bufferSize() const {return mBufferSize;}

Downloader::FrameVector& Downloader::buffer() {return mBuffer;}

Downloader::~Downloader() {
    mParent->makeCurrent();
    mParent->glDeleteBuffers(mPBOSize, mPBOs.constData());
}
