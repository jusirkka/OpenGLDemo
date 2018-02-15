/*
 * The original ktx loader copyright in the OpenGL superbible v7:
 *
 * Copyright (c) 2012-2015 Graham Sellers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Adapted to Qt by Jukka Sirkka (c) 2018
 */


#include <QFile>
#include "texturestore.h"
#include "scope.h"
#include "gl_widget.h"

using namespace Demo;

class TextureSource: public Function {
public:
    TextureSource(TextureStore* p);
    const QVariant& execute(const QVector<QVariant>& vals, int start) override;
    TextureSource* clone() const override;
private:
    TextureStore* mParent;
};


TextureSource::TextureSource(TextureStore* p):
    Function("ktx", new Integer_T),
    mParent(p) {
    mArgTypes.append(new Text_T);
}

const QVariant& TextureSource::execute(const QVector<QVariant>& vals, int start) {
    QString key = vals[start].toString();
    mValue.setValue(mParent->texture(key));
    return mValue;
}

TextureSource* TextureSource::clone() const {
    return new TextureSource(*this);
}


TextureStore::TextureStore(const QString& name, Scope* globals, GLWidget* ctx, QObject* parent)
    : ProjectFolder(name, parent)
    , mTarget(ctx)
{
    globals->addFunction(new TextureSource(this));
}

uint TextureStore::texture(const QString& key) {
    if (mTextures.contains(key)) {
        if (mTextures[key] == 0) {
            mTextures[key] = load_ktx(mFileNames[mNames.indexOf(key)]);
        }
        return mTextures[key];
    }
    return 0;
}

void TextureStore::rename(const QString& from, const QString& to) {
    if (mTextures.contains(from)) {
        uint tex = mTextures.take(from);
        mTextures[to] = tex;
        mNames[mNames.indexOf(from)] = to;
    }
}

void TextureStore::remove(int index) {
    mTextures.remove(mNames[index]);
    mNames.removeAt(index);
    mFileNames.removeAt(index);
}


void TextureStore::setItem(const QString& key, const QString& path) {
    if (mTextures.contains(key)) {
        mFileNames[mNames.indexOf(key)] = path;
    } else {
        mNames.append(key);
        mFileNames.append(path);
    }
    uint new_tex = 0;
    if (!path.isEmpty()) {
        new_tex = load_ktx(path);
    }
    if (mTextures.contains(key)) {
        uint tex = mTextures[key];
        if (tex != 0) {
            mTarget->glDeleteTextures(1, &tex);
        }
    }
    mTextures[key] = new_tex;
}

int TextureStore::size() const {
    return mNames.size();
}

QString TextureStore::fileName(int index) const {
    return mFileNames[index];
}

QString TextureStore::itemName(int index) const {
    return mNames[index];
}

QStringList TextureStore::items() const {
    return mNames;
}

struct header {
    uchar       identifier[12];
    quint32        endianness;
    quint32        gltype;
    quint32        gltypesize;
    quint32        glformat;
    quint32        glinternalformat;
    quint32        glbaseinternalformat;
    quint32        pixelwidth;
    quint32        pixelheight;
    quint32        pixeldepth;
    quint32        arrayelements;
    quint32        faces;
    quint32        miplevels;
    quint32        keypairbytes;
};

union keyvaluepair {
    quint32        size;
    uchar       rawbytes[4];
};

static const unsigned char identifier[] = {
    0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

static uint swap32(const quint32 u32) {
    union {
        quint32 u32;
        unsigned char u8[4];
    } a, b;

    a.u32 = u32;
    b.u8[0] = a.u8[3];
    b.u8[1] = a.u8[2];
    b.u8[2] = a.u8[1];
    b.u8[3] = a.u8[0];

    return b.u32;
}

static quint32 calculate_stride(const header& h, quint32 width, quint32 pad = 4)
{
    quint32 channels = 0;

    switch (h.glbaseinternalformat) {
    case GL_RED: channels = 1;
        break;
    case GL_RG: channels = 2;
        break;
    case GL_BGR:
    case GL_RGB: channels = 3;
        break;
    case GL_BGRA:
    case GL_RGBA: channels = 4;
        break;
    default:
        ;
    }

    quint32 stride = h.gltypesize * channels * width;

    stride = (stride + (pad - 1)) & ~(pad - 1);

    return stride;
}

static quint32 calculate_face_size(const header& h)
{
    quint32 stride = calculate_stride(h, h.pixelwidth);

    return stride * h.pixelheight;
}


uint TextureStore::load_ktx(const QString& path) const {

    if (!mTarget->initialized()) return 0;

    QFile file(path);
    bool ok = file.open(QFile::ReadOnly);
    if (!ok) return 0;

    header h;

    if (file.read((char*) &h, sizeof(h)) != sizeof(h)) {
        file.close();
        throw KtxError("Header read failed");
    }

    if (memcmp(h.identifier, identifier, sizeof(identifier)) != 0) {
        file.close();
        throw KtxError("Not a KTX file");
    }

    if (h.endianness == 0x04030201) {
        // No swap needed
    } else if (h.endianness == 0x01020304) {
        // Swap needed
        h.endianness            = swap32(h.endianness);
        h.gltype                = swap32(h.gltype);
        h.gltypesize            = swap32(h.gltypesize);
        h.glformat              = swap32(h.glformat);
        h.glinternalformat      = swap32(h.glinternalformat);
        h.glbaseinternalformat  = swap32(h.glbaseinternalformat);
        h.pixelwidth            = swap32(h.pixelwidth);
        h.pixelheight           = swap32(h.pixelheight);
        h.pixeldepth            = swap32(h.pixeldepth);
        h.arrayelements         = swap32(h.arrayelements);
        h.faces                 = swap32(h.faces);
        h.miplevels             = swap32(h.miplevels);
        h.keypairbytes          = swap32(h.keypairbytes);
    } else {
        file.close();
        throw KtxError("Corrupt endianness in the header");
    }

    // Guess target (texture type)
    GLenum target = GL_NONE;
    if (h.pixelheight == 0) {
        if (h.arrayelements == 0) {
            target = GL_TEXTURE_1D;
        } else {
            target = GL_TEXTURE_1D_ARRAY;
        }
    } else if (h.pixeldepth == 0) {
        if (h.arrayelements == 0) {
            if (h.faces == 0) {
                target = GL_TEXTURE_2D;
            } else {
                target = GL_TEXTURE_CUBE_MAP;
            }
        } else {
            if (h.faces == 0) {
                target = GL_TEXTURE_2D_ARRAY;
            } else {
                target = GL_TEXTURE_CUBE_MAP_ARRAY;
            }
        }
    } else {
        target = GL_TEXTURE_3D;
    }

    // Check for insanity...
    if (target == GL_NONE ||                                    // Couldn't figure out target
        h.pixelwidth == 0 ||                                  // Texture has no width???
        (h.pixelheight == 0 && h.pixeldepth != 0)) {            // Texture has depth but no height???
        file.close();
        throw KtxError("Corrupt header");
    }

    GLuint tex = 0;
    mTarget->glGenTextures(1, &tex);
    mTarget->glBindTexture(target, tex);

    file.seek(file.pos() + h.keypairbytes);
    auto data = file.readAll();
    file.close();

    if (h.miplevels == 0) {
        h.miplevels = 1;
    }

    switch (target) {
    case GL_TEXTURE_1D:
        mTarget->glTexStorage1D(GL_TEXTURE_1D, h.miplevels, h.glinternalformat, h.pixelwidth);
        mTarget->glTexSubImage1D(GL_TEXTURE_1D, 0, 0, h.pixelwidth, h.glformat, h.glinternalformat, data.constData());
        break;
    case GL_TEXTURE_2D:
        if (h.gltype == GL_NONE) {
            mTarget->glCompressedTexImage2D(GL_TEXTURE_2D, 0, h.glinternalformat, h.pixelwidth, h.pixelheight, 0, 420 * 380 / 2, data.constData());
        } else {
            mTarget->glTexStorage2D(GL_TEXTURE_2D, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight);
            {
                const char * ptr = data.constData();
                uint height = h.pixelheight;
                uint width = h.pixelwidth;
                mTarget->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                for (uint i = 0; i < h.miplevels; i++) {
                    mTarget->glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, width, height, h.glformat, h.gltype, ptr);
                    ptr += height * calculate_stride(h, width, 1);
                    height >>= 1;
                    width >>= 1;
                    if (!height) height = 1;
                    if (!width) width = 1;
                }
            }
        }
        break;
    case GL_TEXTURE_3D:
        mTarget->glTexStorage3D(GL_TEXTURE_3D, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight, h.pixeldepth);
        mTarget->glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, h.pixelwidth, h.pixelheight, h.pixeldepth, h.glformat, h.gltype, data.constData());
        break;
    case GL_TEXTURE_1D_ARRAY:
        mTarget->glTexStorage2D(GL_TEXTURE_1D_ARRAY, h.miplevels, h.glinternalformat, h.pixelwidth, h.arrayelements);
        mTarget->glTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, 0, h.pixelwidth, h.arrayelements, h.glformat, h.gltype, data.constData());
        break;
    case GL_TEXTURE_2D_ARRAY:
        mTarget->glTexStorage3D(GL_TEXTURE_2D_ARRAY, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight, h.arrayelements);
        mTarget->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, h.pixelwidth, h.pixelheight, h.arrayelements, h.glformat, h.gltype, data.constData());
        break;
    case GL_TEXTURE_CUBE_MAP:
        mTarget->glTexStorage2D(GL_TEXTURE_CUBE_MAP, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight);
        {
            uint face_size = calculate_face_size(h);
            for (uint i = 0; i < h.faces; i++) {
                mTarget->glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, h.pixelwidth, h.pixelheight, h.glformat, h.gltype, data.constData() + face_size * i);
            }
        }
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY:
        mTarget->glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, h.miplevels, h.glinternalformat, h.pixelwidth, h.pixelheight, h.arrayelements);
        mTarget->glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 0, h.pixelwidth, h.pixelheight, h.faces * h.arrayelements, h.glformat, h.gltype, data.constData());
        break;
    default: // Should never happen
        throw KtxError("Unknown texture target");
    }

    if (h.miplevels == 1) {
        mTarget->glGenerateMipmap(target);
    }

    return tex;
}

