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
    const void* readData(const QString& key) const override;
    TexBlobSpec spec(const QString& key) const override;

    void rename(const QString& from, const QString& to);
    void remove(int index);
    void setImage(const QString& key, const QString& path = QString(""));
    void clean();
    int size();
    const QString& fileName(int);
    const QString& imageName(int);
    QStringList itemSample(const QString& except = QString()) const;


private:

    using ImageMap = QMap<QString, QImage>;

private:

    ImageMap mImages;
    QStringList mNames;
    QStringList mFileNames;
};

}} // namespace Demo::GL

#endif
