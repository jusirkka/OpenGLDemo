#ifndef TYPE_H
#define TYPE_H

#include <QMetaType>
#include <QVector>
#include <QHash>

namespace Demo {

class Type {
public:

    using List = QVector<const Type*>;
    using NewList = QVector<Type*>;

    static const int Integer;
    static const int Real;
    static const int Vector;
    static const int Matrix;
    static const int Text;

    virtual bool assignable(const Type* rhs) const;
    virtual int id() const = 0;
    virtual Type* clone() const = 0;

    virtual ~Type() = default;


};

template <typename T> class BaseType: public Type {

public:

    int id() const override {return qMetaTypeId<T>();}
    BaseType<T>* clone() const override {return new BaseType<T>(*this);}

};


class ComboundType: public Type {
public:
    int id() const override {return -1000;}
    const List& subtypes() const {return mTypes;}
    ~ComboundType() {qDeleteAll(mTypes);}

protected:

    List mTypes;
};

class ArrayType: public ComboundType {
public:
    ArrayType(const Type* t) {mTypes.append(t);}
    ArrayType(const ArrayType& arr) {mTypes.append(arr.subtypes().first()->clone());}
    bool assignable(const Type* rhs) const override;
    ArrayType* clone() const override {return new ArrayType(*this);}
};

class RecordType: public ComboundType {
public:
    RecordType(const QStringList& names, const NewList& types) {
        for (int i = 0; i < names.size(); i++) mIndex[names[i]] = i;
        for (auto t: types) mTypes << t;
    }
    RecordType(const List& types) {
        for (auto t: types) mTypes << t->clone();
        QString templ("%1");
        for (int i = 0; i < mTypes.size(); i++) {
            mIndex[templ.arg(i)] = i;
        }
    }
    RecordType(const RecordType& arr) {
        mIndex = arr.mIndex;
        for (auto t: arr.subtypes()) mTypes << t->clone();
    }
    bool assignable(const Type* rhs) const override;
    RecordType* clone() const override {return new RecordType(*this);}
    const Type* type(const QString& name) const {
        if (!mIndex.contains(name)) return nullptr;
        return mTypes[mIndex[name]];
    }
    int index(const QString& name) const {
        if (!mIndex.contains(name)) return -1;
        return mIndex[name];
    }

private:

    QHash<QString, int> mIndex;
};

// Dummy type for selecting Record's member type
class Selector: public Type {

public:

    Selector(QString name): mName(std::move(name)) {}
    int id() const override {return -1;}
    Selector* clone() const override {return new Selector(*this);}
    bool assignable(const Type*) const override {return false;}
    const QString& name() const {return mName;}

private:

    QString mName;

};

// The type of the empty list expression
class NullType: public Type {

public:

    int id() const override {return 0;}
    NullType* clone() const override {return new NullType(*this);}
    bool assignable(const Type*) const override {return false;}
};

}

#endif // TYPE_H
