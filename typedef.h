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


#ifndef DEMO_TYPEDEF_H
#define DEMO_TYPEDEF_H

#include "symbol.h"
#include "math3d.h"


namespace Demo {

using Real_T = BaseType<Math3D::Real>;
using Integer_T = BaseType<Math3D::Integer>;
using Vector_T = BaseType<Math3D::Vector4>;
using Matrix_T = BaseType<Math3D::Matrix4>;
using Text_T = BaseType<QString>;


class Typedef: public Symbol {

public:

    Typedef(QString name, Type* type): Symbol(name, type) {}
    ~Typedef() = default;

    Typedef(const Typedef& t): Symbol(t) {}
    CLONE(Typedef)

};


class Basetypes {

public:

    QVector<Demo::Symbol*> contents;

    Basetypes() {
        contents.append(new Typedef("Real", new Real_T));
        contents.append(new Typedef("Natural", new Integer_T));
        contents.append(new Typedef("Vector", new Vector_T));
        contents.append(new Typedef("Matrix", new Matrix_T));
        contents.append(new Typedef("Text", new Text_T));
    }
};

} // namespace DEMO
#endif
