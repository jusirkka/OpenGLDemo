#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H

#include <QThread>

struct AVCodecContext;
struct AVFrame;

namespace Demo {


namespace GL {
    class Downloader;
}

class VideoEncoder: public QThread {

    Q_OBJECT

public:

    VideoEncoder(GL::Downloader* parent, const QString& basePath);
    void stop();

    ~VideoEncoder();

protected:

    void run() override;

private:

    void frame_yuv_from_rgb(AVFrame* frame, char* data) const;

private:

    GL::Downloader* mParent;
    bool mReady;
    AVCodecContext* mContext;
    QString mFileName;

};

}
#endif // VIDEOENCODER_H
