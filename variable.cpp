#include "variable.h"

using namespace Demo;

Variable* Var::Create(int kind, const QString& name, bool shared) {
    if (kind == Symbol::Integer) {
        if (shared) return new Shared::Natural(name);
        return new Local::Natural(name);
    }
    if (kind == Symbol::Real) {
        if (shared) return new Shared::Real(name);
        return new Local::Real(name);
    }
    if (kind == Symbol::Matrix) {
        if (shared) return new Shared::Matrix(name);
        return new Local::Matrix(name);
    }
    if (kind == Symbol::Vector) {
        if (shared) return new Shared::Vector(name);
        return new Local::Vector(name);
    }
    if (kind == Symbol::Text) {
        if (shared) return new Shared::Text(name);
        return new Local::Text(name);
    }
    return nullptr;
}
