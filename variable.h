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
#include "value.h"

#include <QVariant>
#include <QSharedData>

namespace Demo {

class Variable: public Symbol {

public:

    using Path = QVector<int>;

    virtual void setValue(const QVariant& val, const Path& path = Path()) = 0;
    virtual QVariant value(const Path& path = Path()) const = 0;

    unsigned index() const {return mIndex;}
    void setIndex(unsigned idx) {mIndex = idx;}
    virtual bool shared() const = 0;

    Variable* clone() const override = 0;

    ~Variable() override = default;

protected:

    Variable(QString name, Type* type): Symbol(name, type), mIndex(0) {}

protected:

    int mIndex;

};

class LocalVar: public Variable {
public:
    LocalVar(QString name, Type* type)
        : Variable(name, type)
        , mValue(Value::Create(type)) {}
    LocalVar(const LocalVar& v)
        : Variable(v.name(), v.type()->clone())
        , mValue(v.mValue->clone()) {}

    QVariant value(const Path& p = Path()) const override {return mValue->get(p);}
    void setValue(const QVariant& val, const Path& p = Path()) override {mValue->set(val, p);}

    LocalVar* clone() const override {return new LocalVar(*this);}

    bool shared() const override {return false;}

protected:

    Value* mValue;
};


class SharedData: public QSharedData {
public:
    Value* value;
    SharedData(Type* t): value(Value::Create(t)) {}
    SharedData(const SharedData& other): value(other.value->clone()) {}
    ~SharedData() {delete value;}
};

class SharedVar: public Variable {
public:
    SharedVar(QString name, Type* type)
        : Variable(name, type) {d = new SharedData(type);}
    SharedVar(const SharedVar& v)
        : Variable(v.name(), v.type()->clone())
        , d(v.d) {}

    QVariant value(const Path& p = Path()) const override {return d->value->get(p);}
    void setValue(const QVariant& val, const Path& p = Path()) override {d->value->set(val, p);}

    SharedVar* clone() const override {return new SharedVar(*this);}

    bool shared() const override {return true;}

protected:
    QExplicitlySharedDataPointer<SharedData> d;
};


using VariableMap = QMap<QString, Variable*>;

} // namespace Demo
#endif // DEMO_VARIABLE_H
