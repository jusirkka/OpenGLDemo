#ifndef TEXTURESTORE_H
#define TEXTURESTORE_H

#include <QObject>
#include <QMap>
#include <QStringList>

#include "projectfolder.h"


namespace Demo {

class Scope;
class GLWidget;

class KtxError {

public:
    KtxError(QString msg)
        : emsg(std::move(msg))
    {}


    const QString msg() const {return emsg;}

private:

    QString emsg;

};




class TextureStore : public ProjectFolder {

    Q_OBJECT

public:


    TextureStore(const QString& name, Scope* globals, GLWidget* context, QObject* parent = nullptr);


    void rename(const QString& from, const QString& to) override;
    void remove(int index) override;
    void setItem(const QString& key, const QString& path = QString("")) override;
    int size() const override;
    QString fileName(int) const override;
    QString itemName(int) const override;
    QStringList items() const override;

    uint texture(const QString& name);

private:

    using TextureMap = QMap<QString, uint>;

    uint load_ktx(const QString& path) const;

private:

    TextureMap mTextures;
    QStringList mNames;
    QStringList mFileNames;
    GLWidget* mTarget;
};

} // namespace Demo

#endif // TEXTURESTORE_H
