#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QThread>
#include <QSemaphore>

#include "math3d.h"

namespace Demo {

class DataSource : public QThread
{
    Q_OBJECT

public:

    DataSource(int id, const QString& device, int maxFrames);

    using DataVector = QVector<Math3D::Real>;
    using FrameVector = QVector<DataVector>;


    bool hasFilledFrames() const;
    void acquireFilledFrame();
    void releaseEmptyFrame();

    void stop();

    FrameVector& buffer();

    int id() const;

    ~DataSource();

protected:

    void run() override;

private:

    DataVector readData(int fd, qint32 n);
    qint32 readSize(int fd);

private:

    bool mReady;
    int mBufferSize;
    int mBufferIndex;
    FrameVector mBuffer;
    QSemaphore mFilled;
    QSemaphore mEmpty;
    int mId;
    QString mFifo;
    bool mRemoveFifo;
};

}
#endif // DATASOURCE_H
