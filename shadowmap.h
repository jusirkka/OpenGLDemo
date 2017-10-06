#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include <QObject>
#include <QtPlugin>
#include <QGLWidget>

#include "texblob.h"

namespace Demo {
namespace GL {

class ShadowMap : public QObject, public TexBlob {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "net.kvanttiapina.OpenGLDemos.TexBlob/1.0")
    Q_INTERFACES(Demo::GL::TexBlob)

public:


    ShadowMap();

    // tex blob implementation
    const void* readData(const QString& key) const override;
    TexBlobSpec spec(const QString& key) const override;

public slots:

    void viewportChanged(GLuint w, GLuint h);

private:

    TexBlobSpec mSpec;
};

}} // namespace Demo::GL

#endif // SHADOWMAP_H
