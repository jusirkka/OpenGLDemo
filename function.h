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

#include <QList>
#include <QVector>
#include <QVariant>

using Math3D::Vector4;
using Math3D::Matrix4;

namespace Demo {


class Function: public Symbol {

    public:

        typedef Symbol::TypeList TypeList;

        int type() const {return mRetType;}
        const TypeList& argTypes() const {return mArgTypes;}

        unsigned index() const {return mIndex;}
        void setIndex(unsigned idx) {mIndex = idx;}

        virtual const QVariant& execute(const QVector<QVariant>& vals, int start) = 0;

        virtual ~Function() {}

    protected:

        Function(const QString& name, int type):
            Symbol(name), mArgTypes(), mValue(), mRetType(type), mIndex(0) {}

    protected:

        TypeList mArgTypes;
        QVariant mValue;

    private:

        int mRetType;
        unsigned  mIndex;
};

class StdFunction: public Function {

    typedef Math3D::Real (*stdfun) (Math3D::Real);

    public:

        StdFunction(const QString& name, stdfun fun): Function(name, Symbol::Real), mFun(fun) {
            int argt = Symbol::Real;
            mArgTypes.append(argt);
        }

        const QVariant& execute(const QVector<QVariant>& vals, int start) {
            mValue.setValue(mFun(vals[start].value<Math3D::Real>()));
            return mValue;
        }

        ~StdFunction() {}

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

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        Vector4 v(vals[start].value<Math3D::Real>(),
                  vals[start+1].value<Math3D::Real>(),
                  vals[start+2].value<Math3D::Real>(),
                  vals[start+3].value<Math3D::Real>());
        qDebug() << "Vec" << v.getArray();

        mValue.setValue(v);
        return mValue;
    }

    ~Vecx() {}
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

    const QVariant& execute(const QVector<QVariant>& vals, int start) {
        Math3D::Real values[16];
        for (int i = 0; i < 4; i++) {
            Vector4 v = vals[start + i].value<Math3D::Vector4>();
            for (int j = 0; j < 4; j++) {
                values[4 * i + j] = v.readArray()[j];
            }
        }
        Matrix4 m(values);
        qDebug() << "Mat" << m.getArray();


        mValue.setValue(m);
        return mValue;
    }

    ~Mat() {}
};

} // namespace DEMO
#endif // DEMO_FUNCTION_H
