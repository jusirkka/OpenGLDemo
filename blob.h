#ifndef BLOB_H
#define BLOB_H

#include <QtPlugin>
#include <QMap>

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

    int size;
    unsigned int type;
    bool normalized;
    int stride;
    unsigned int offset;
};


class Blob {

public:

    Blob(const QString& name)
        : mName(name)
    {}

    const QString& name() const {return mName;}
    unsigned int bytelen(unsigned int target) const {return mData[target]->length;}
    const void* bytes(unsigned int target) const {return mData[target]->data;}
    const BlobSpec& spec(const QString& key) const {return *mSpecs[key];}

    virtual void draw(unsigned int mode) const = 0;

    virtual ~Blob() {}

protected:

    typedef QMap<QString, BlobSpec*> SpecMap;

    class Data {
    public:
        Data(void* d, unsigned int l)
            : data(d),
              length(l)
        {}

        void* data;
        unsigned int length;
    };

    typedef QMap<unsigned int, Data*> DataMap;

protected:

    SpecMap mSpecs;
    DataMap mData;

private:

    QString mName;

};

} // namespace GL

Q_DECLARE_INTERFACE(GL::Blob, "org.kvanttiapina.OpenGLDemos.Blob/1.0")


#endif // BLOB_H
