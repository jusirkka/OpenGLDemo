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


#ifndef DEMO_CONSTANT_H
#define DEMO_CONSTANT_H

#include "symbol.h"
#include "math3d.h"

#include <QVariant>

namespace Demo {

using Real_T = BaseType<Math3D::Real>;
using Integer_T = BaseType<Math3D::Integer>;
using Vector_T = BaseType<Math3D::Vector4>;
using Matrix_T = BaseType<Math3D::Matrix4>;

class Constant: public Symbol {

    public:

        Constant(const QString& name, Math3D::Integer val):
            Symbol(name, new Integer_T), mValue(val) {}
        Constant(const QString& name, Math3D::Real val):
            Symbol(name, new Real_T), mValue(val) {}
        Constant(const QString& name, const Math3D::Vector4& val):
            Symbol(name, new Vector_T), mValue(QVariant::fromValue(val)) {}
        Constant(const QString& name, const Math3D::Matrix4& val):
            Symbol(name, new Matrix_T), mValue(QVariant::fromValue(val)) {}

        Constant(const Constant& c)
            : Symbol(c)
            , mValue(c.value()) {}


        const QVariant& value() const {return mValue;}

        CLONE(Constant)

    private:

        QVariant mValue;
};


class Constants {

public:

    QVector<Demo::Symbol*> contents;

    Constants() {
        contents.append(new Constant("true", 1));
        contents.append(new Constant("false", 0));
    }

};


} // namespace Demo
#endif // DEMO_CONSTANT_H
