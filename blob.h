#ifndef BLOB_H
#define BLOB_H

#include <QtPlugin>
#include <QMap>

namespace Demo {
namespace GL {


class BlobSpec {

public:

    BlobSpec(int siz, unsigned int t, bool n, int s, unsigned int off)
        : size(siz),
          type(t),
          normalized(n),
          stride(s),
          offset(off)
    {}

    BlobSpec()
        : size(0),
          type(0),
          normalized(false),
          stride(0),
          offset(0)
    {}

    int size;
    unsigned int type;
    bool normalized;
    int stride;
    unsigned int offset;
};


class Blob {

public:

    Blob() {}

    QString name() const {return dynamic_cast<const QObject*>(this)->objectName();}
    unsigned int bytelen(unsigned int target) const {return mData[target].length;}
    const void* bytes(unsigned int target) const {return mData[target].data;}
    const BlobSpec spec(const QString& key) const {return mSpecs[key];}

    virtual void draw(unsigned int mode, const QString& attr) const = 0;

    virtual ~Blob() = default;

protected:

    using SpecMap = QMap<QString, BlobSpec>;

    class Data {
    public:
        Data(char* d, unsigned int l)
            : data(d),
              length(l)
        {}

        Data()
            : data(nullptr),
              length(0)
        {}

        char* data;
        unsigned int length;
    };

    using DataMap = QMap<unsigned int, Data>;

protected:

    SpecMap mSpecs;
    DataMap mData;

};

}} // namespace Demo::GL

Q_DECLARE_INTERFACE(Demo::GL::Blob, "net.kvanttiapina.OpenGLDemos.Blob/1.0")


#endif // BLOB_H
