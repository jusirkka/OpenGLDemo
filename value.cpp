#include "value.h"
#include "type.h"

using namespace Demo;

Value* Value::Create(const Type* t) {
    auto arr = dynamic_cast<const ArrayType*>(t);
    if (arr) {
        auto v = new ListValue;
        v->kids << Create(arr->subtypes().first());
        return v;
    }
    auto rec = dynamic_cast<const RecordType*>(t);
    if (rec) {
        auto v = new RecValue;
        for (auto s: rec->subtypes()) {
            v->kids << Create(s);
        }
        return v;
    }
    auto mat = dynamic_cast<const BaseType<Matrix4>*>(t);
    if (mat) {
        return new MatrixValue;
    }
    auto vec = dynamic_cast<const BaseType<Vector4>*>(t);
    if (vec) {
        return new VectorValue;
    }
    return new LeafValue;
}

ListValue* ListValue::tmpl() const {
    auto v = new ListValue;
    v->kids << kids.first()->tmpl();
    return v;
}

void ListValue::set(const QVariant& v, Path p) {
    if (!p.isEmpty()) {
        int index = p.takeFirst();
        if (0 > index) throw ValueError("out of bounds error");
        while (kids.size() <= index) {
            kids << kids.first()->tmpl();
        }
        kids[index]->set(v, p);
        return;
    }
    QList<QVariant> list = v.toList();
    while (kids.size() <= list.size()) {
        kids << kids.first()->tmpl();
    }
    for (auto kid: kids) {
        QVariant q = list.takeFirst();
        kid->set(q, p);
    }
}

QVariant ListValue::get(Path p) const {
    if (!p.isEmpty()) {
        int index = p.takeFirst();
        if (kids.size() <= index || 0 > index) throw ValueError("out of bounds error");
        return kids[index]->get(p);
    }
    QList<QVariant> list;
    for (auto kid: kids) {
        list << kid->get(p);
    }
    return QVariant::fromValue(list);
}


void RecValue::set(const QVariant& v, Path p) {
    if (!p.isEmpty()) {
        int index = p.takeFirst();
        kids[index]->set(v, p);
        return;
    }
    QList<QVariant> list = v.toList();
    for (auto kid: kids) {
        QVariant q = list.takeFirst();
        kid->set(q, p);
    }
}

QVariant RecValue::get(Path p) const {
    if (!p.isEmpty()) {
        int index = p.takeFirst();
        return kids[index]->get(p);
    }
    QList<QVariant> list;
    for (auto kid: kids) {
        list << kid->get(p);
    }
    return QVariant::fromValue(list);
}



void VectorValue::set(const QVariant& v, Path p) {
    if (!p.isEmpty()) {
        int index = p.takeFirst();
        if (!p.isEmpty()) throw ValueError("path too long when setting vector value");
        if (index < 0 || index > 3) throw ValueError("out of bounds error");
        vec(index) = v.value<Math3D::Real>();
        return;
    }
    vec = v.value<Vector4>();
}

QVariant VectorValue::get(Path p) const {
    if (!p.isEmpty()) {
        int index = p.takeFirst();
        if (!p.isEmpty()) throw ValueError("path too long when setting vector value");
        if (index < 0 || index > 3) throw ValueError("out of bounds error");
        return QVariant::fromValue(vec[index]);
    }
    return QVariant::fromValue(vec);
}

void MatrixValue::set(const QVariant& v, Path p) {
    if (p.size() > 2) throw ValueError("path too long when setting matrix value");
    if (!p.isEmpty()) {
        int index = p.takeFirst();
        if (index < 0 || index > 3) throw ValueError("out of bounds error");
        if (!p.isEmpty()) {
            int index2 = p.takeFirst();
            if (index2 < 0 || index2 > 3) throw ValueError("out of bounds error");
            mat(index)[index2] = v.value<Math3D::Real>();
        } else {
            Vector4 col = v.value<Vector4>();
            for (int i = 0; i < 4; i++) {
                mat(index)[i] = col[i];
            }
        }
        return;
    }
    mat = v.value<Matrix4>();
}

QVariant MatrixValue::get(Path p) const {
    if (p.size() > 2) throw ValueError("path too long when setting matrix value");
    if (!p.isEmpty()) {
        int index = p.takeFirst();
        if (index < 0 || index > 3) throw ValueError("out of bounds error");
        if (!p.isEmpty()) {
            int index2 = p.takeFirst();
            if (index2 < 0 || index2 > 3) throw ValueError("out of bounds error");
            return QVariant::fromValue(mat[index][index2]);
        }
        return QVariant::fromValue(Vector4(mat[index], 4));
    }
    return QVariant::fromValue(mat);
}

