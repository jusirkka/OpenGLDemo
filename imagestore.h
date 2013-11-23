#ifndef IMAGESTORE_H
#define IMAGESTORE_H

#include <QObject>
#include <QtPlugin>
#include <QImage>
#include <QStringList>

#include "texblob.h"

namespace GL {

class ImageStore : public QObject, public TexBlob
{
    Q_OBJECT
    Q_INTERFACES(GL::TexBlob)

public:

    ImageStore();

    const void* data(const QString& key) const;
    const TexBlobSpec spec(const QString& key) const;

    void rename(const QString& from, const QString& to);
    void remove(int index);
    void setImage(const QString& key, const QString& path = QString(""));
    void clean();
    int size();
    const QString& fileName(int);
    const QString& imageName(int);

    ~ImageStore();

private:

    typedef QMap<QString, QImage> ImageMap;

private:

    ImageMap mImages;
    QStringList mNames;
    QStringList mFileNames;
};

} // namespace GL

#endif
