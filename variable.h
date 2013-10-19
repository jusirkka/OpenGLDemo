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

    virtual Variable* clone() const = 0;

    bool used() const {return mUsed;}
    void setUsed(bool used) {mUsed = used;}

    virtual bool shared() const {return false;}

    virtual ~Variable() {}

protected:

    Variable(const QString name)
        : Symbol(name),
          mIndex(0),
          mUsed(false)
    {}

protected:

    int mIndex;
    bool mUsed;

};

class SharedData: public QSharedData {

public:

    SharedData() {}


    ~SharedData() {}

public:

    QVariant value;
};



class SharedVar: public Variable {

public:

    const QVariant& value() const {return d->value;}

    bool shared() const {return true;}

    virtual ~SharedVar() {}

protected:

    SharedVar(const QString& name)
        : Variable(name)
    {d = new SharedData;}



protected:

    QExplicitlySharedDataPointer<SharedData> d;

};

class LocalVar: public Variable {

public:

    const QVariant& value() const {return mValue;}


    virtual ~LocalVar() {}

protected:

    LocalVar(const QString& name):
        Variable(name),
        mValue() {}

protected:

    QVariant mValue;
};




namespace Var {

namespace Local {

class Matrix: public LocalVar {

public:

    Matrix(const QString& name): LocalVar(name) {}

    int type() const {return Symbol::Matrix;}
    void setValue(const QVariant& val) {mValue.setValue(val.value<Math3D::Matrix4>());}

    Matrix* clone() const {return new Matrix(*this);}
    ~Matrix() {}

};

class Vector: public LocalVar {

public:

    Vector(const QString& name): LocalVar(name) {}

    int type() const {return Symbol::Vector;}
    void setValue(const QVariant& val) {mValue.setValue(val.value<Math3D::Vector4>());}

    Vector* clone() const {return new Vector(*this);}

    ~Vector() {}

};

class Real: public LocalVar {

public:

    Real(const QString& name): LocalVar(name) {}

    int type() const {return Symbol::Real;}
    void setValue(const QVariant& val) {mValue.setValue(val.value<Math3D::Real>());}

    Real* clone() const {return new Real(*this);}

    ~Real() {}

};

class Natural: public LocalVar {

public:

    Natural(const QString& name): LocalVar(name) {}

    int type() const {return Symbol::Integer;}
    void setValue(const QVariant& val) {mValue.setValue(val.value<Math3D::Integer>());}

    Natural* clone() const {return new Natural(*this);}

    ~Natural() {}

};

class Text: public LocalVar {

public:

    Text(const QString& name): LocalVar(name) {}

    int type() const {return Symbol::Text;}
    void setValue(const QVariant& val) {mValue.setValue(val.value<QString>());}


    Text* clone() const {return new Text(*this);}

    ~Text() {}

};

} // namespace Local

namespace Shared {

class Matrix: public SharedVar {

public:

    Matrix(const QString& name): SharedVar(name) {}

    int type() const {return Symbol::Matrix;}
    void setValue(const QVariant& val) {d->value.setValue(val.value<Math3D::Matrix4>());}

    Matrix* clone() const {return new Matrix(*this);}

    ~Matrix() {}

};

class Vector: public SharedVar {

public:

    Vector(const QString& name): SharedVar(name) {}

    int type() const {return Symbol::Vector;}
    void setValue(const QVariant& val) {d->value.setValue(val.value<Math3D::Vector4>());}

    Vector* clone() const {return new Vector(*this);}

    ~Vector() {}

};

class Real: public SharedVar {

public:

    Real(const QString& name): SharedVar(name) {}

    int type() const {return Symbol::Real;}
    void setValue(const QVariant& val) {d->value.setValue(val.value<Math3D::Real>());}

    Real* clone() const {return new Real(*this);}

    ~Real() {}

};

class Natural: public SharedVar {

public:

    Natural(const QString& name): SharedVar(name) {}

    int type() const {return Symbol::Integer;}
    void setValue(const QVariant& val) {d->value.setValue(val.value<Math3D::Integer>());}

    Natural* clone() const {return new Natural(*this);}

    ~Natural() {}

};

class Text: public SharedVar {

public:

    Text(const QString& name): SharedVar(name) {}

    int type() const {return Symbol::Text;}
    void setValue(const QVariant& val) {d->value.setValue(val.value<QString>());}


    Text* clone() const {return new Text(*this);}

    ~Text() {}

};

} // namespace Shared

}} // namespace Demo::Var
#endif // DEMO_VARIABLE_H
