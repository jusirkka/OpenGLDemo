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

    void draw(unsigned int mode, const QString& attr) const;

    ~Teapot();

private:

    class Element {
        public:
            Element() {}
            Element(unsigned int m, unsigned int cnt, unsigned short index):
                mode(m),
                count(cnt),
                offset(index)
                {}
            unsigned int mode;
            unsigned int count;
            unsigned short offset;
    };

    typedef QVector<Element> ElementVector;
    typedef QMap<unsigned int, unsigned int> ModeMap;
    typedef QMap<unsigned int, ModeMap> ModeMapMap;

private:

    ElementVector mElements;
    ModeMapMap mModes;
    ModeMap mSupported;
};

}} // namespace Demo::GL

#endif // TEAPOT_H
