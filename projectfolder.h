#ifndef PROJECTFOLDER_H
#define PROJECTFOLDER_H

#include <QObject>

namespace Demo {

class ProjectFolder: public QObject {

    Q_OBJECT

public:


    ProjectFolder(const QString& name, QObject* parent = nullptr);

    virtual void rename(const QString& from, const QString& to) = 0;
    virtual void remove(int index) = 0;
    virtual int size() const = 0;
    virtual QString fileName(int) const = 0;
    virtual QString itemName(int) const = 0;
    virtual QStringList items() const = 0;
    virtual void setItem(const QString& key, const QString& path = QString("")) = 0;

};

}

#endif // PROJECTFOLDER_H
