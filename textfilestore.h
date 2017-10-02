#ifndef TEXTFILESTORE_H
#define TEXTFILESTORE_H


#include <QObject>
#include <QMap>
#include <QStringList>

namespace Demo {

class Scope;

class TextFileStore : public QObject {

    Q_OBJECT

public:


    TextFileStore(const QString& name, Scope* globals, QObject* parent = nullptr);


    void rename(const QString& from, const QString& to);
    void remove(int index);
    void setText(const QString& key, const QString& path = QString(""));
    int size();
    const QString& fileName(int);
    const QString& itemName(int);
    QString text(const QString& key) const;
    QStringList itemSample(const QString& except = QString()) const;

private:

    typedef QMap<QString, QString> TextMap;

private:

    TextMap mTexts;
    QStringList mNames;
    QStringList mFileNames;
};

} // namespace Demo

#endif // TEXTFILESTORE_H
