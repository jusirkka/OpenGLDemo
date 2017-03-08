#include "symbol.h"
#include "math3d.h"

const int Demo::Symbol::Integer = qMetaTypeId<Math3D::Integer>();
const int Demo::Symbol::Real = qMetaTypeId<Math3D::Real>();
const int Demo::Symbol::Text = qMetaTypeId<QString>();
const int Demo::Symbol::Matrix = qMetaTypeId<Math3D::Matrix4>();
const int Demo::Symbol::Vector = qMetaTypeId<Math3D::Vector4>();
