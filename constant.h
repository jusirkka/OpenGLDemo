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


class Constant: public Symbol {

    public:

        Constant(const QString& name, Math3D::Integer val):
            Symbol(name), mType(Symbol::Integer), mValue(val) {}
        Constant(const QString& name, Math3D::Real val):
            Symbol(name), mType(Symbol::Real), mValue(val) {}
        Constant(const QString& name, const Math3D::Vector4& val):
            Symbol(name), mType(Symbol::Vector), mValue(QVariant::fromValue(val)) {}
        Constant(const QString& name, const Math3D::Matrix4& val):
            Symbol(name), mType(Symbol::Matrix), mValue(QVariant::fromValue(val)) {}

        int type() const  override {return mType;}
        const QVariant& value() const {return mValue;}

        CLONEMETHOD(Constant)

    private:

        int mType;
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
