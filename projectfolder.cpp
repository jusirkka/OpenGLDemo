#include "projectfolder.h"

using namespace Demo;

ProjectFolder::ProjectFolder(const QString &name, QObject *parent)
    : QObject(parent) {
    setObjectName(name);
}

