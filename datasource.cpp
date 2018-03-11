#include "datasource.h"
#include "logging.h"

#include <QFileInfo>
#include <QLocalSocket>
#include <QDataStream>
#include <QFile>
#include <QDir>
#include <QApplication>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
}

using namespace Demo;

DataSource::DataSource(int id, const QString& device, int maxFrames)
    : QThread()
    , mReady(false)
    , mBufferSize(maxFrames)
    , mBufferIndex(0)
    , mBuffer(mBufferSize)
    , mFilled(0)
    , mEmpty(mBufferSize)
    , mId(id)
    , mFifo(device)
    , mRemoveFifo(false)
{
    QFileInfo info(device);
    if (!info.exists()) {

        mFifo = QString("%1/%2-%3-%4-%5")
                .arg(QDir::tempPath())
                .arg(QApplication::applicationName())
                .arg(device)
                .arg(mId)
                .arg(QApplication::applicationPid());

        QFileInfo tmp(mFifo);
        if (tmp.exists()) {
            qCDebug(OGL) << "removing" << mFifo;
            QDir::temp().remove(mFifo);
        }
        qCDebug(OGL) << "creating" << mFifo;
        if (mkfifo(mFifo.toUtf8().constData(), 00600) < 0) {
            mReady = true;
        } else {
            mRemoveFifo = true;
        }
    }
}


DataSource::~DataSource() {
    if (mRemoveFifo) {
        qCDebug(OGL) << "cleaning" << mFifo;
        QDir::temp().remove(mFifo);
    }
}


bool DataSource::hasFilledFrames() const {
    return mFilled.available() > 0;
}

void DataSource::acquireFilledFrame() {
    mFilled.acquire();
}

void DataSource::releaseEmptyFrame() {
    mEmpty.release();
}

void DataSource::stop() {
    mReady = true;
    terminate();
}

DataSource::FrameVector& DataSource::buffer() {return mBuffer;}

int DataSource::id() const {return mId;}


qint32 DataSource::readSize(int fd) {
    char s[4];
    int count = read(fd, s, 4);
    if (count < 0) {
        if (errno != EAGAIN) {
            qCDebug(OGL) << "error reading size #1" << strerror(errno);
            mReady = true;
        }
        return 0;
    }

    if (count == 0) return 0;

    while (count < 4) {
        if (mReady) return 0;
        int ret = read(fd, s + count, 4 - count);
        if (ret < 0) {
            qCDebug(OGL) << "error reading size #2" << strerror(errno);
            mReady = true;
            return 0;
        }
        if (ret == 0) {
            // partially read size - keep trying
            sleep(1);
            continue;
        }
        count += ret;
    }

    QDataStream stream(QByteArray(s, 4));
    qint32 n;
    stream >> n;

    qCDebug(OGL) << "size =" << n;

    return n;
}

DataSource::DataVector DataSource::readData(int fd, qint32 n) {

    size_t nb = 8 * n;

    if (nb > 1000000) { // megabyte
        mReady = true;
        return DataVector();
    }
    char s[nb];

    size_t count = 0;

    while (count < nb) {
        if (mReady) return DataVector();
        int ret = read(fd, s + count, nb - count);
        if (ret < 0) {
            qCDebug(OGL) << "error reading data #1" << strerror(errno);
            mReady = true;
            return DataVector();
        }
        if (ret == 0) {
            // partially read data - keep trying
            usleep(100 * 1000);
            continue;
        }
        count += ret;
    }

    QDataStream stream(QByteArray(s, nb));

    DataVector data(n);
    for (int i = 0; i < n; i++) {
        stream >> data[i];
    }
    qCDebug(OGL) << data;
    return data;
}

void DataSource::run() {

    if (mReady) return;

    // int fd = open(mFifo.toUtf8().constData(), O_RDONLY | O_NONBLOCK);
    QFile f(mFifo);
    QDataStream stream;

    while (!mReady) {


        if (!f.isOpen()) {
            qCDebug(OGL) << "opening" << mFifo;
            f.open(QIODevice::ReadOnly);
            stream.setDevice(&f);
        } else if (f.atEnd()) {
            qCDebug(OGL) << "closing" << mFifo;
            f.close();
            continue;
        }

        qCDebug(OGL) << "reading" << mFifo;

        /*qint32 n = readSize(fd);
        if (n == 0) {
            usleep(500 * 1000);
            continue;
        }

        DataVector data = readData(fd, n);
        if (data.isEmpty()) {
            continue;
        } */


        qint32 n;
        stream >> n;
        if (n > 100000) { // ~ megabyte
            mReady = true;
            continue;
        }

        DataVector data(n);

        for (int i = 0; i < n; i++) {
            stream >> data[i];
        }
        // qCDebug(OGL) << data;


        mEmpty.acquire();

        qCDebug(OGL) << "filling" << mFifo;
        mBuffer[mBufferIndex] = data;
        mBufferIndex = (mBufferIndex + 1) % mBufferSize;

        mFilled.release();

    }

    if (f.isOpen()) f.close();
    // close(fd);
}

