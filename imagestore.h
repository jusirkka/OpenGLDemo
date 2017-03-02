#ifndef IMAGESTORE_H
#define IMAGESTORE_H

#include <QObject>
#include <QtPlugin>
#include <QImage>
#include <QStringList>

#include "texblob.h"

namespace Demo {
namespace GL {

class ImageStore : public QObject, public TexBlob {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "net.kvanttiapina.OpenGLDemos.TexBlob/1.0")
    Q_INTERFACES(Demo::GL::TexBlob)

public:


    ImageStore();

    // tex blob implementation
    const void* data(const QString& key) const;
    const TexBlobSpec spec(const QString& key) const;

    ~ImageStore();

    void rename(const QString& from, const QString& to);
    void remove(int index);
    void setImage(const QString& key, const QString& path = QString(""));
    void clean();
    int size();
    const QString& fileName(int);
    const QString& imageName(int);


private:

    typedef QMap<QString, QImage> ImageMap;

private:

    ImageMap mImages;
    QStringList mNames;
    QStringList mFileNames;
};

}} // namespace Demo::GL

#endif
