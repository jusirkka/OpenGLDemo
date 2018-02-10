#include "type.h"
#include "math3d.h"

using namespace Demo;

const int Type::Integer = qMetaTypeId<Math3D::Integer>();
const int Type::Real = qMetaTypeId<Math3D::Real>();
const int Type::Text = qMetaTypeId<QString>();
const int Type::Matrix = qMetaTypeId<Math3D::Matrix4>();
const int Type::Vector = qMetaTypeId<Math3D::Vector4>();

bool Type::assignable(const Type* rhs) const {
    if (rhs->id() == 0) return true; // null type
    if (id() == rhs->id()) return true;
    if (id() == Real && rhs->id() == Integer) return true;
    return false;
}

bool ArrayType::assignable(const Type* rhs) const {
    if (rhs->id() == 0) return true; // null type
    auto arr = dynamic_cast<const ArrayType*>(rhs);
    if (!arr) return false;
    return subtypes().first()->assignable(arr->subtypes().first());
}

bool RecordType::assignable(const Type* rhs) const {
    if (rhs->id() == 0) return true; // null type
    auto rec = dynamic_cast<const RecordType*>(rhs);
    if (!rec) return false;
    if (subtypes().size() != rec->subtypes().size()) return false;
    for (int i = 0; i < subtypes().size(); i++) {
        if (!subtypes()[i]->assignable(rec->subtypes()[i])) return false;
    }
    return true;
}
