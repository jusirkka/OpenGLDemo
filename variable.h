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


#ifndef DEMO_VARIABLE_H
#define DEMO_VARIABLE_H

#include "symbol.h"
#include "math3d.h"

#include <QVariant>
#include <QSharedData>

namespace Demo {

class Variable: public Symbol {

public:

    virtual void setValue(const QVariant& val) = 0;
    virtual const QVariant& value() const = 0;

    unsigned index() const {return mIndex;}
    void setIndex(unsigned idx) {mIndex = idx;}
    virtual bool shared() const = 0;

    Variable* clone() const override = 0;

    ~Variable() override = default;

protected:

    Variable(const QString& name): Symbol(name), mIndex(0) {}

protected:

    int mIndex;

};

template <typename T> class LocalVar: public Variable {
public:
    LocalVar(const QString& name): Variable(name), mValue() {}
    const QVariant& value() const override {return mValue;}
    void setValue(const QVariant& val) override {mValue.setValue(val.value<T>());}
    int type() const override {return qMetaTypeId<T>();}
    LocalVar* clone() const override {return new LocalVar(*this);}
    bool shared() const override {return false;}
protected:
    QVariant mValue;
};


class SharedData: public QSharedData {
public:
    SharedData() = default;
public:
    QVariant value;
};

template <typename T> class SharedVar: public Variable {
public:
    SharedVar(const QString& name): Variable(name) {d = new SharedData;}
    const QVariant& value() const override {return d->value;}
    void setValue(const QVariant& val) override {d->value.setValue(val.value<T>());}
    int type() const override {return qMetaTypeId<T>();}
    bool shared() const override {return true;}
    SharedVar* clone() const override {return new SharedVar(*this);}
protected:
    QExplicitlySharedDataPointer<SharedData> d;
};


namespace Var {

namespace Local {
using Natural = LocalVar<Math3D::Integer>;
using Real = LocalVar<Math3D::Real>;
using Matrix = LocalVar<Math3D::Matrix4>;
using Vector = LocalVar<Math3D::Vector4>;
using Text = LocalVar<QString>;
}

namespace Shared {
using Natural = SharedVar<Math3D::Integer>;
using Real = SharedVar<Math3D::Real>;
using Matrix = SharedVar<Math3D::Matrix4>;
using Vector = SharedVar<Math3D::Vector4>;
using Text = SharedVar<QString>;
}

Variable* Create(int kind, const QString& name, bool shared);

} // namespace Var

using VariableMap = QMap<QString, Variable*>;

} // namespace Demo
#endif // DEMO_VARIABLE_H
