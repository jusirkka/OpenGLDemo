#ifndef TEXBLOB_H
#define TEXBLOB_H

#include <QtPlugin>
#include <QMap>

namespace Demo {
namespace GL {


class TexBlobSpec {

public:

    TexBlobSpec(unsigned int w, unsigned int h, unsigned int f, unsigned int t)
        : width(w),
          height(h),
          format(f),
          type(t)
    {}

    TexBlobSpec()
        : width(0),
          height(0),
          format(0),
          type(0)
    {}

    unsigned int width;
    unsigned int height;
    unsigned int format;
    unsigned int type;
};


class TexBlob {

public:

    TexBlob() {}

    QString name() const {return dynamic_cast<const QObject*>(this)->objectName();}

    virtual const TexBlobSpec spec(const QString& key) const = 0;

    virtual const void* data(const QString& key) const = 0;

    virtual ~TexBlob() {}

};

}} // namespace Demo::GL

Q_DECLARE_INTERFACE(Demo::GL::TexBlob, "net.kvanttiapina.OpenGLDemos.TexBlob/1.0")

#endif // TEXBLOB_H
