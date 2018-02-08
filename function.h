// -----------------------------------------------------------------------
//   Copyright (C) 2009 by Jukka Sirkka
//   jukka.sirkka@iki.fi
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the
//   Free Software Foundation, Inc.,
//   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------


#ifndef DEMO_FUNCTION_H
#define DEMO_FUNCTION_H

#include "symbol.h"
#include "math3d.h"

#include <QVector>
#include <QVector>
#include <QVariant>

using Math3D::Vector4;
using Math3D::Matrix4;



namespace Demo {


class Function: public Symbol {

    public:

        using TypeVector = Symbol::TypeVector;

        int type() const override {return mRetType;}
        const TypeVector& argTypes() const {return mArgTypes;}

        unsigned index() const {return mIndex;}
        void setIndex(unsigned idx) {mIndex = idx;}

        virtual const QVariant& execute(const QVector<QVariant>& vals, int start) = 0;

    protected:

        Function(const QString& name, int type):
            Symbol(name), mArgTypes(), mValue(), mRetType(type), mIndex(0) {}

    protected:

        TypeVector mArgTypes;
        QVariant mValue;

    private:

        int mRetType;
        unsigned  mIndex;
};

class StdFunction: public Function {

    using stdfun = Math3D::Real (*)(Math3D::Real);

    public:

        StdFunction(const QString& name, stdfun fun): Function(name, Symbol::Real), mFun(fun) {
            int argt = Symbol::Real;
            mArgTypes.append(argt);
        }

        const QVariant& execute(const QVector<QVariant>& vals, int start) override {
            mValue.setValue(mFun(vals[start].value<Math3D::Real>()));
            return mValue;
        }

        CLONEMETHOD(StdFunction)

    private:

        stdfun mFun;

};

class Vecx: public Function {

public:

    Vecx(): Function("vec", Symbol::Vector) {
        int argt = Symbol::Real;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 v(vals[start].value<Math3D::Real>(),
                  vals[start+1].value<Math3D::Real>(),
                  vals[start+2].value<Math3D::Real>(),
                  vals[start+3].value<Math3D::Real>());
        // qDebug() << "Vec";

        mValue.setValue(v);
        return mValue;
    }

    CLONEMETHOD(Vecx)

};


class Mat: public Function {

public:

    Mat(): Function("mat", Symbol::Matrix) {
        int argt = Symbol::Vector;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real values[16];
        for (int i = 0; i < 4; i++) {
            Vector4 v = vals[start + i].value<Math3D::Vector4>();
            for (int j = 0; j < 4; j++) {
                values[4 * i + j] = v.readArray()[j];
            }
        }
        Matrix4 m(values);
        // qDebug() << "Mat";


        mValue.setValue(m);
        return mValue;
    }

    CLONEMETHOD(Mat)

};

class Rot: public Function {

public:

    Rot(): Function("rotation", Symbol::Matrix) {
        int argt = Symbol::Real;
        mArgTypes.append(argt);
        argt = Symbol::Vector;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real angle = vals[start].value<Math3D::Real>() * Math3D::PI / 180;
        Vector4 axis = vals[start + 1].value<Vector4>();
        Matrix4 m;
        m.setRotation(angle, axis);
        // qDebug() << "Rot";
        mValue.setValue(m);
        return mValue;
    }

    CLONEMETHOD(Rot)
};

class Tr: public Function {

public:

    Tr(): Function("translation", Symbol::Matrix) {
        int argt = Symbol::Vector;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 tr = vals[start].value<Vector4>();
        Matrix4 m;
        m.setTranslation(tr);
        // qDebug() << "Tr";
        mValue.setValue(m);
        return mValue;
    }

    CLONEMETHOD(Tr)
};

class Sc: public Function {

public:

    Sc(): Function("scaling", Symbol::Matrix) {
        int argt = Symbol::Real;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real x = vals[start].value<Math3D::Real>();
        Math3D::Real y = vals[start+1].value<Math3D::Real>();
        Math3D::Real z = vals[start+2].value<Math3D::Real>();
        Matrix4 m;
        m.setScaling(x, y, z);
        // qDebug() << "Sc";
        mValue.setValue(m);
        return mValue;
    }

    CLONEMETHOD(Sc)
};

class Norm: public Function {

public:

    Norm(): Function("normalize", Symbol::Vector) {
        int argt = Symbol::Vector;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 x = vals[start].value<Vector4>();
        Vector4 u = x.normalized3();
        // qDebug() << "Norm";
        mValue.setValue(u);
        return mValue;
    }

    CLONEMETHOD(Norm)
};

class NormalT: public Function {

public:

    NormalT(): Function("normal_transform", Symbol::Matrix) {
        int argt = Symbol::Matrix;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Matrix4 m = vals[start].value<Matrix4>();
        // qDebug() << "normal transform: check" << m.comatrix() * m.transpose3();
        // qDebug() << "NormalT";
        mValue.setValue(m.comatrix());
        return mValue;
    }

    CLONEMETHOD(NormalT)
};

class Inverse: public Function {

public:

    Inverse(): Function("affine_inverse", Symbol::Matrix) {
        int argt = Symbol::Matrix;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Matrix4 m = vals[start].value<Matrix4>();
        mValue.setValue(m.inverse());
        return mValue;
    }

    CLONEMETHOD(Inverse)
};

class Refl: public Function {

public:

    Refl(): Function("reflection", Symbol::Matrix) {
        int argt = Symbol::Vector;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 normal = vals[start].value<Vector4>();
        normal.normalize3();
        Vector4 point = vals[start + 1].value<Vector4>();
        Matrix4 t;
        t.setTranslation(2 * Math3D::dot3(point, normal) * normal);
        Matrix4 r;
        r.setIdentity();
        r = r - 2 * Math3D::projection3(normal);
        // qDebug() << "Refl";
        mValue.setValue(t * r);
        return mValue;
    }

    CLONEMETHOD(Refl)
};


class Length: public Function {

public:

    Length(): Function("length", Symbol::Real) {
        int argt = Symbol::Vector;
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 v = vals[start].value<Vector4>();
        mValue.setValue(v.length3());
        return mValue;
    }

    CLONEMETHOD(Length)
};


class LookAt: public Function {

public:

    LookAt(): Function("lookat", Symbol::Matrix) {
        int argt = Symbol::Vector;
        mArgTypes.append(argt);
        mArgTypes.append(argt);
        mArgTypes.append(argt);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        auto eye = vals[start].value<Vector4>();
        auto center = vals[start + 1].value<Vector4>();
        auto up = vals[start + 2].value<Vector4>();

        auto z = (eye - center).normalized3();
        auto y = up - dot3(z, up) * z;

        Matrix4 rot, tr;

        rot.setBasis(y, z);
        tr.setTranslation(- eye);
        // qDebug() << rot*tr;
        mValue.setValue(rot * tr);
        return mValue;
    }

    CLONEMETHOD(LookAt)
};

class Functions {

public:

    QVector<Demo::Symbol*> contents;

#define FUN(fun) contents.append(new StdFunction(#fun, std::fun))

    Functions() {
        contents.append(new Vecx());
        contents.append(new Mat());
        contents.append(new Rot());
        contents.append(new Tr());
        contents.append(new Sc());
        contents.append(new Norm());
        contents.append(new NormalT());
        contents.append(new Inverse());
        contents.append(new Refl());
        contents.append(new LookAt());
        contents.append(new Length());
        FUN(sin);
        FUN(cos);
        FUN(tan);
        FUN(asin);
        FUN(acos);
        FUN(atan);
        FUN(exp);
        FUN(log);
        FUN(log10);
        FUN(sqrt);
        FUN(abs);
        FUN(ceil);
        FUN(floor);
        FUN(sinh);
        FUN(cosh);
        FUN(tanh);
    }

#undef FUN

};

using FunctionVector = QVector<Demo::Function*>;

} // namespace DEMO
#endif // DEMO_FUNCTION_H
