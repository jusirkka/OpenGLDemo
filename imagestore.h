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

    static void Clean();
    static void SetImage(const QString& key, const QString& path = QString(""));
    static int Size();
    static const QString& ImageName(int);
    static const QString& FileName(int);
    static void Rename(const QString& from, const QString& to);
    static void Remove(int index);

    ImageStore();

    // text blob implementation
    const void* data(const QString& key) const;
    const TexBlobSpec spec(const QString& key) const;

    ~ImageStore();

private:

    static ImageStore* instance();


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

} // namespace GL

#endif
