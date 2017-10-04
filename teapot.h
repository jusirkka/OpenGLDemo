#ifndef TEAPOT_H
#define TEAPOT_H

#include <QObject>
#include <QtPlugin>
#include <QVector>

#include "blob.h"

namespace Demo {
namespace GL {

class Teapot : public QObject, public Blob
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "net.kvanttiapina.OpenGLDemos.Blob/1.0")
    Q_INTERFACES(Demo::GL::Blob)

public:

    Teapot();

    void draw(unsigned int mode, const QString& attr) const override;

    ~Teapot() override;

private:

    class Element {
        public:
            Element() = default;
            Element(unsigned int m, unsigned int cnt, unsigned short index):
                mode(m),
                count(cnt),
                offset(index)
                {}
            unsigned int mode;
            unsigned int count;
            unsigned short offset;
    };

    using ElementVector = QVector<Demo::GL::Teapot::Element>;
    using ModeMap = QMap<unsigned int, unsigned int>;
    using ModeMapMap = QMap<unsigned int, ModeMap>;

private:

    ElementVector mElements;
    ModeMapMap mModes;
    ModeMap mSupported;
};

}} // namespace Demo::GL

#endif // TEAPOT_H
