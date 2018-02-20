#include "videoencoder.h"
#include "downloader.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}


static AVCodec* getCodec() {
    static AVCodec* codec = nullptr;
    if (!codec) {
        avcodec_register_all();
        codec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
    }
    return codec;
}

using namespace Demo;

VideoEncoder::VideoEncoder(GL::Downloader *parent, const QString& basePath)
    : QThread()
    , mParent(parent)
    , mReady(false)
    , mContext(nullptr)
{
    AVCodec* codec = getCodec();
    mContext = avcodec_alloc_context3(codec);
    mContext->width = mParent->width();
    mContext->height = mParent->height();
    mContext->bit_rate = 400000;
    mContext->time_base = {1, 25};
    mContext->framerate = {25, 1};
    mContext->gop_size = 10;
    mContext->max_b_frames = 1;
    mContext->pix_fmt = AV_PIX_FMT_YUV420P;
    if (avcodec_open2(mContext, codec, nullptr) < 0) {
        qWarning("Could not open codec");
        mReady = true;
    } else {
        auto dt = QDateTime::currentDateTimeUtc();
        mFileName = QString("%1-%2.mp4").arg(basePath).arg(dt.toString(Qt::ISODate));
    }
}


void VideoEncoder::stop() {mReady = true;}

static uint8_t endcode[] = {0, 0, 1, 0xb7};

static SwsContext* getSWSContext(int w, int h) {
    static SwsContext* ctx = nullptr;

    ctx = sws_getCachedContext(ctx,
                               w, h, AV_PIX_FMT_RGB24,
                               w, h, AV_PIX_FMT_YUV420P,
                               0, 0, 0, 0);
    return ctx;
}


void VideoEncoder::frame_yuv_from_rgb(AVFrame* frame, char* data) const {
    int w = mContext->width;
    int h = mContext->height;

    // swap rows
    for (int i = 0; i < h / 2; i++) {
        char tmp[3 * w];
        memcpy(tmp, &data[3 * w * i], 3 * w);
        memcpy(&data[3 * w * i], &data[3 * w * (h - 1 - i)], 3 * w);
        memcpy(&data[3 * w * (h - 1 - i)], tmp, 3 * w);
    }

    const int in_linesize[1] = {3 * w};

    sws_scale(getSWSContext(w, h), reinterpret_cast<const uint8_t * const *>(&data), in_linesize, 0,
              h, frame->data, frame->linesize);
}

void VideoEncoder::run() {
    if (mReady) return; // Ctor failed to get context
    QFile sample(mFileName);
    bool ok = sample.open(QIODevice::WriteOnly);
    if (!ok) {
        qWarning() << "Cannot open" << mFileName;
        return;
    }
    AVFrame *frame = av_frame_alloc();
    int w = mContext->width;
    int h = mContext->height;
    frame->format = mContext->pix_fmt;
    frame->width = w;
    frame->height = h;
    int ret = av_image_alloc(frame->data, frame->linesize, w, h, mContext->pix_fmt, 32);
    AVPacket* pkt = av_packet_alloc();
    int slot = 0;
    int frameNum = 0;
    while (!mReady) {
        mParent->acquireFilledFrame();

        QByteArray& bytes = mParent->buffer()[slot];
        // qDebug() << "encoding frame" << frameNum << "size =" << bytes.size();
        frame_yuv_from_rgb(frame, bytes.data());
        bytes.clear();
        frame->pts = frameNum;
        avcodec_send_frame(mContext, frame);

        ret = avcodec_receive_packet(mContext, pkt);
        while (ret >= 0) {
            // qDebug() << "write packet, size =" << pkt->size;
            sample.write(reinterpret_cast<const char*>(pkt->data), pkt->size);
            av_packet_unref(pkt);
            ret = avcodec_receive_packet(mContext, pkt);
        }
        // qDebug() << "receive status = " << ret;


        frameNum++;
        slot = frameNum % mParent->bufferSize();

        mParent->releaseEmptyFrame();
    }

    // delayed frames
    avcodec_send_frame(mContext, nullptr);

    ret = avcodec_receive_packet(mContext, pkt);
    while (ret >= 0) {
        // qDebug() << "write delayed packet, size =" << pkt->size;
        sample.write(reinterpret_cast<const char*>(pkt->data), pkt->size);
        av_packet_unref(pkt);
        ret = avcodec_receive_packet(mContext, pkt);
    }

    qDebug() << "encoding ready, closing. Last receive status = " << ret;

    sample.write(reinterpret_cast<const char*>(endcode), sizeof(endcode));
    sample.close();

    av_frame_free(&frame);
    av_packet_free(&pkt);

}

VideoEncoder::~VideoEncoder() {
    avcodec_free_context(&mContext);
}
