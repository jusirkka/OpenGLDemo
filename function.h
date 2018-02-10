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

#define COPY(T) T(const T& f): Function(f) {}


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

    COPY(Vecx)
    CLONE(Vecx)

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

    COPY(MatRow)
    CLONE(MatRow)

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

    COPY(MatCol)
    CLONE(MatCol)

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
        // qDebug() << "Rot";
        mValue.setValue(m);
        return mValue;
    }

    COPY(Rot)
    CLONE(Rot)
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
        // qDebug() << "Tr";
        mValue.setValue(m);
        return mValue;
    }

    COPY(Tr)
    CLONE(Tr)

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
        // qDebug() << "Sc";
        mValue.setValue(m);
        return mValue;
    }

    COPY(Sc)
    CLONE(Sc)
};

class Norm: public Function {

public:

    Norm(): Function("normalize", new Vector_T) {
        mArgTypes.append(new Vector_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Vector4 x = vals[start].value<Vector4>();
        Vector4 u = x.normalized3();
        // qDebug() << "Norm";
        mValue.setValue(u);
        return mValue;
    }

    COPY(Norm)
    CLONE(Norm)

};

class NormalT: public Function {

public:

    NormalT(): Function("normal_transform", new Matrix_T) {
        mArgTypes.append(new Matrix_T);
    }

    const QVariant& execute(const QVector<QVariant>& vals, int start) override {
        Matrix4 m = vals[start].value<Matrix4>();
        // qDebug() << "normal transform: check" << m.comatrix() * m.transpose3();
        // qDebug() << "NormalT";
        mValue.setValue(m.comatrix());
        return mValue;
    }

    COPY(NormalT)
    CLONE(NormalT)
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

    COPY(Inverse)
    CLONE(Inverse)
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
        // qDebug() << "Refl";
        mValue.setValue(t * r);
        return mValue;
    }

    COPY(Refl)
    CLONE(Refl)
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

    COPY(Length)
    CLONE(Length)
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
        // qDebug() << rot*tr;
        mValue.setValue(rot * tr);
        return mValue;
    }

    COPY(LookAt)
    CLONE(LookAt)
};


class Functions {

public:

    QVector<Demo::Symbol*> contents;

#define FUN(fun) contents.append(new StdFunction(#fun, std::fun))

    Functions() {
        contents.append(new Vecx());
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

#undef COPY

} // namespace DEMO
#endif // DEMO_FUNCTION_H
