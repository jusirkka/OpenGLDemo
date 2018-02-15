#ifndef IMAGESTORE_H
#define IMAGESTORE_H

#include <QObject>
#include <QtPlugin>
#include <QImage>
#include <QStringList>

#include "texblob.h"
#include "projectfolder.h"

namespace Demo {
namespace GL {

class ImageStore : public ProjectFolder, public TexBlob {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "net.kvanttiapina.OpenGLDemos.TexBlob/1.0")
    Q_INTERFACES(Demo::GL::TexBlob)

public:


    ImageStore();

    // tex blob implementation
    const void* readData(const QString& key) const override;
    TexBlobSpec spec(const QString& key) const override;

    // Project interface
    void rename(const QString& from, const QString& to) override;
    void remove(int index) override;
    void setItem(const QString& key, const QString& path = QString("")) override;
    int size() const override;
    QString fileName(int) const override;
    QString itemName(int) const override;
    QStringList items() const override;

    void clean();

private:

    using ImageMap = QMap<QString, QImage>;

private:

    ImageMap mImages;
    QStringList mNames;
    QStringList mFileNames;
};

}} // namespace Demo::GL

#endif
