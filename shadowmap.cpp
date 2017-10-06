#include "shadowmap.h"

#include <QDebug>

using namespace Demo::GL;

ShadowMap::ShadowMap()
    : QObject()
    , TexBlob()
    , mSpec(1024, 1024, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT)
{
    setObjectName("shadowmap");
}


const void* ShadowMap::readData(const QString&) const {
    return nullptr;
}

void ShadowMap::viewportChanged(GLuint w, GLuint h) {
    mSpec.width = w;
    mSpec.height = h;
}

TexBlobSpec ShadowMap::spec(const QString&) const {
    return mSpec;
}

