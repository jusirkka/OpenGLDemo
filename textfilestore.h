#ifndef TEXTFILESTORE_H
#define TEXTFILESTORE_H


#include <QObject>
#include <QMap>
#include <QStringList>

#include "projectfolder.h"


namespace Demo {

class Scope;

class TextFileStore : public ProjectFolder {

    Q_OBJECT

public:


    TextFileStore(const QString& name, Scope* globals, QObject* parent = nullptr);


    void rename(const QString& from, const QString& to) override;
    void remove(int index) override;
    void setItem(const QString& key, const QString& path = QString("")) override;
    int size() const override;
    QString fileName(int) const override;
    QString itemName(int) const override;
    QStringList items() const override;

    QString text(const QString& key) const;

private:

    using TextMap = QMap<QString, QString>;

private:

    TextMap mTexts;
    QStringList mNames;
    QStringList mFileNames;
};

} // namespace Demo

#endif // TEXTFILESTORE_H
