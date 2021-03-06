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
#include <random>

using Math3D::Vector4;
using Math3D::Matrix4;


#define COPY_AND_CLONE(T) T(const T& f): Function(f) {} \
                          T* clone() const override {return new T(*this);}


namespace Demo {

using Real_T = BaseType<Math3D::Real>;
using Integer_T = BaseType<Math3D::Integer>;
using Vector_T = BaseType<Vector4>;
using Matrix_T = BaseType<Matrix4>;
using Text_T = BaseType<QString>;

class Function: public Symbol {

    public:

        using TypeList = Type::List;

        const TypeList& argTypes() const {return mArgTypes;}

        unsigned index() const {return mIndex;}
        void setIndex(unsigned idx) {mIndex = idx;}

        virtual const QVariant& execute(const QVector<QVariant>& vals, int start) = 0;

    protected:

        Function(const QString& name, Type* type):
            Symbol(name, type), mArgTypes(), mValue(), mIndex(0) {}

        Function(const Function& f)
            : Symbol(f)
            , mValue()
            , mIndex(f.index()) {
            for (auto t: f.argTypes()) {
                mArgTypes << t->clone();
            }
        }

        ~Function() {qDeleteAll(mArgTypes);}



    protected:

        TypeList mArgTypes;
        QVariant mValue;

    private:

        unsigned  mIndex;
};

class StdFunction: public Function {

    using stdfun = Math3D::Real (*)(Math3D::Real);

    public:

        StdFunction(const QString& name, stdfun fun): Function(name, new Real_T), mFun(fun) {
            mArgTypes.append(new Real_T);
        }

        const QVariant& execute(const QVector<QVariant>& vals, int start) override {
            mValue.setValue(mFun(vals[start].value<Math3D::Real>()));
            return mValue;
        }

        StdFunction(const StdFunction& f)
            : Function(f)
            , mFun(f.mFun) {}

        CLONE(StdFunction)

    private:

        stdfun mFun;

};



class Vecx: public Function {

public:

    Vecx(): Function("vec", new Vector_T) {
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Real_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 v(vals[start].value<Math3D::Real>(),
                  vals[start+1].value<Math3D::Real>(),
                  vals[start+2].value<Math3D::Real>(),
                  vals[start+3].value<Math3D::Real>());

        mValue.setValue(v);
        return mValue;
    }

    COPY_AND_CLONE(Vecx)

};


class VecPos: public Function {

public:

    VecPos(): Function("pos", new Vector_T) {
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Real_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 v(vals[start].value<Math3D::Real>(),
                  vals[start+1].value<Math3D::Real>(),
                  vals[start+2].value<Math3D::Real>());

        mValue.setValue(v);
        return mValue;
    }

    COPY_AND_CLONE(VecPos)

};

class VecDir: public Function {

public:

    VecDir(): Function("dir", new Vector_T) {
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Real_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 v(vals[start].value<Math3D::Real>(),
                  vals[start+1].value<Math3D::Real>(),
                  vals[start+2].value<Math3D::Real>(),
                  0);

        mValue.setValue(v);
        return mValue;
    }

    COPY_AND_CLONE(VecDir)

};


class Random: public Function {

public:

    Random()
        : Function("random", new Real_T)
        , mDevice()
        , mEngine(mDevice())
        , mDist(0, 1)
    {}

    const QVariant& execute(const QVector<QVariant>&, int) override {
        mValue.setValue(mDist(mEngine));
        return mValue;
    }

    COPY_AND_CLONE(Random)

private:
    std::random_device mDevice;
    std::default_random_engine mEngine;
    std::uniform_real_distribution<Math3D::Real> mDist;
};

class RandomPos: public Function {

public:

    RandomPos()
        : Function("randompos", new Vector_T)
        , mDevice()
        , mEngine(mDevice())
        , mDist(0, 1)
    {}

    const QVariant& execute(const QVector<QVariant>&, int) override {
        Vector4 v(mDist(mEngine), mDist(mEngine), mDist(mEngine), 1);
        mValue.setValue(v);
        return mValue;
    }

    COPY_AND_CLONE(RandomPos)

private:
    std::random_device mDevice;
    std::default_random_engine mEngine;
    std::uniform_real_distribution<Math3D::Real> mDist;
};


class FMod: public Function {

public:

    FMod(): Function("fmodf", new Real_T) {
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Real_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real x = vals[start].value<Math3D::Real>();
        Math3D::Real y = vals[start + 1].value<Math3D::Real>();
        mValue.setValue(static_cast<Math3D::Real>(fmodf(x, y)));
        return mValue;
    }

    COPY_AND_CLONE(FMod)
};

class Mod: public Function {

public:

    Mod(): Function("mod", new Integer_T) {
        mArgTypes.append(new Integer_T);
        mArgTypes.append(new Integer_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Integer x = vals[start].value<Math3D::Real>();
        Math3D::Integer y = vals[start + 1].value<Math3D::Real>();
        mValue.setValue(x % y);
        return mValue;
    }

    COPY_AND_CLONE(Mod)
};

class MatRow: public Function {

public:

    MatRow(): Function("matrow", new Matrix_T) {
        mArgTypes.append(new Vector_T);
        mArgTypes.append(new Vector_T);
        mArgTypes.append(new Vector_T);
        mArgTypes.append(new Vector_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Matrix4 m;

        for (int r = 0; r < 4; r++) {
            Vector4 v = vals[start + r].value<Math3D::Vector4>();
            for (int c = 0; c < 4; c++) {
                m(c)[r] = v.readArray()[c];
            }
        }

        mValue.setValue(m);
        return mValue;
    }

    COPY_AND_CLONE(MatRow)

};


class MatCol: public Function {

public:

    MatCol(): Function("matcol", new Matrix_T) {
        mArgTypes.append(new Vector_T);
        mArgTypes.append(new Vector_T);
        mArgTypes.append(new Vector_T);
        mArgTypes.append(new Vector_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Matrix4 m;

        for (int c = 0; c < 4; c++) {
            Vector4 v = vals[start + c].value<Math3D::Vector4>();
            for (int r = 0; r < 4; r++) {
                m(c)[r] = v.readArray()[r];
            }
        }

        mValue.setValue(m);
        return mValue;
    }

    COPY_AND_CLONE(MatCol)

};


class Rot: public Function {

public:

    Rot(): Function("rotation", new Matrix_T) {
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Vector_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real angle = vals[start].value<Math3D::Real>() * Math3D::PI / 180;
        Vector4 axis = vals[start + 1].value<Vector4>();
        Matrix4 m;
        m.setRotation(angle, axis);
        // qCDebug(OGL) << "Rot";
        mValue.setValue(m);
        return mValue;
    }

    COPY_AND_CLONE(Rot)
};

class Tr: public Function {

public:

    Tr(): Function("translation", new Matrix_T) {
        mArgTypes.append(new Vector_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 tr = vals[start].value<Vector4>();
        Matrix4 m;
        m.setTranslation(tr);
        // qCDebug(OGL) << "Tr";
        mValue.setValue(m);
        return mValue;
    }

    COPY_AND_CLONE(Tr)

};

class Sc: public Function {

public:

    Sc(): Function("scaling", new Matrix_T) {
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Real_T);
        mArgTypes.append(new Real_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Math3D::Real x = vals[start].value<Math3D::Real>();
        Math3D::Real y = vals[start+1].value<Math3D::Real>();
        Math3D::Real z = vals[start+2].value<Math3D::Real>();
        Matrix4 m;
        m.setScaling(x, y, z);
        // qCDebug(OGL) << "Sc";
        mValue.setValue(m);
        return mValue;
    }

    COPY_AND_CLONE(Sc)
};

class Norm: public Function {

public:

    Norm(): Function("normalize", new Vector_T) {
        mArgTypes.append(new Vector_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 x = vals[start].value<Vector4>();
        Vector4 u = x.normalized3();
        // qCDebug(OGL) << "Norm";
        mValue.setValue(u);
        return mValue;
    }

    COPY_AND_CLONE(Norm)

};

class NormalT: public Function {

public:

    NormalT(): Function("normal_transform", new Matrix_T) {
        mArgTypes.append(new Matrix_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Matrix4 m = vals[start].value<Matrix4>();
        // qCDebug(OGL) << "normal transform: check" << m.comatrix() * m.transpose3();
        // qCDebug(OGL) << "NormalT";
        mValue.setValue(m.comatrix());
        return mValue;
    }

    COPY_AND_CLONE(NormalT)
};

class Inverse: public Function {

public:

    Inverse(): Function("affine_inverse", new Matrix_T) {
        mArgTypes.append(new Matrix_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Matrix4 m = vals[start].value<Matrix4>();
        mValue.setValue(m.inverse());
        return mValue;
    }

    COPY_AND_CLONE(Inverse)
};

class Refl: public Function {

public:

    Refl(): Function("reflection", new Matrix_T) {
        mArgTypes.append(new Vector_T);
        mArgTypes.append(new Vector_T);
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
        // qCDebug(OGL) << "Refl";
        mValue.setValue(t * r);
        return mValue;
    }

    COPY_AND_CLONE(Refl)
};


class Length: public Function {

public:

    Length(): Function("length", new Real_T) {
        mArgTypes.append(new Vector_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 v = vals[start].value<Vector4>();
        mValue.setValue(v.length3());
        return mValue;
    }

    COPY_AND_CLONE(Length)
};


class LookAt: public Function {

public:

    LookAt(): Function("lookat", new Matrix_T) {
        mArgTypes.append(new Vector_T);
        mArgTypes.append(new Vector_T);
        mArgTypes.append(new Vector_T);
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
        // qCDebug(OGL) << rot*tr;
        mValue.setValue(rot * tr);
        return mValue;
    }

    COPY_AND_CLONE(LookAt)
};


class Trace: public Function {

public:

    Trace(): Function("trace", new Integer_T) {
        mArgTypes.append(new NullType);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        qCDebug(OGL) << vals[start];
        mValue.setValue(0);
        return mValue;
    }

    COPY_AND_CLONE(Trace)
};

class Size: public Function {

public:

    Size(): Function("size", new Integer_T) {
        mArgTypes.append(new NullType);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        QVariant v = vals[start];
        if (v.userType() != QMetaType::QVariantList) {
            throw RunError("Only Records and Arrays accepted", 0);
        }
        mValue.setValue(v.toList().size());
        return mValue;
    }

    COPY_AND_CLONE(Size)
};

class Functions {

public:

    QVector<Demo::Symbol*> contents;

#define FUN(fun) contents.append(new StdFunction(#fun, std::fun))

    Functions() {
        contents.append(new Vecx());
        contents.append(new VecPos());
        contents.append(new VecDir());
        contents.append(new Random());
        contents.append(new RandomPos());
        contents.append(new Mod());
        contents.append(new FMod());
        contents.append(new MatRow());
        contents.append(new MatCol());
        contents.append(new Rot());
        contents.append(new Tr());
        contents.append(new Sc());
        contents.append(new Norm());
        contents.append(new NormalT());
        contents.append(new Inverse());
        contents.append(new Refl());
        contents.append(new Length());
        contents.append(new LookAt());
        contents.append(new Trace());
        contents.append(new Size());
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

#undef COPY_AND_CLONE

} // namespace DEMO
#endif // DEMO_FUNCTION_H
