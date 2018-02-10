#include <QFile>
#include "textfilestore.h"
#include "scope.h"

using namespace Demo;

class TextSource: public Function {
public:
    TextSource(TextFileStore* p);
    const QVariant& execute(const QVector<QVariant>& vals, int start) override;
    TextSource* clone() const override;
private:
    TextFileStore* mParent;
};


TextSource::TextSource(TextFileStore* p):
    Function("source", new Text_T),
    mParent(p) {
    mArgTypes.append(new Text_T);
}

const QVariant& TextSource::execute(const QVector<QVariant>& vals, int start) {
    QString key = vals[start].toString();
    mValue.setValue(mParent->text(key));
    return mValue;
}

TextSource* TextSource::clone() const {
    return new TextSource(*this);
}


TextFileStore::TextFileStore(const QString& name, Scope* globals, QObject* parent)
    : QObject(parent)
{
    setObjectName(name);
    globals->addFunction(new TextSource(this));
}

QString TextFileStore::text(const QString& key) const {
    if (mTexts.contains(key)) {
        return mTexts[key];
    }
    return QString("");
}

void TextFileStore::rename(const QString& from, const QString& to) {
    if (mTexts.contains(from)) {
        QString txt = mTexts.take(from);
        mTexts[to] = txt;
        mNames[mNames.indexOf(from)] = to;
    }
}

void TextFileStore::remove(int index) {
    mTexts.remove(mNames[index]);
    mNames.removeAt(index);
    mFileNames.removeAt(index);
}


void TextFileStore::setText(const QString& key, const QString& path) {
    if (mTexts.contains(key)) {
        mFileNames[mNames.indexOf(key)] = path;
    } else {
        mNames.append(key);
        mFileNames.append(path);
    }
    QString txt("");
    if (!path.isEmpty()) {
        QFile file(path);
        file.open(QFile::ReadOnly);
        txt = QString(file.readAll());
        file.close();
    }
    mTexts[key] = txt;
}

int TextFileStore::size() {
    return mNames.size();
}

const QString& TextFileStore::fileName(int index) {
    return mFileNames[index];
}

const QString& TextFileStore::itemName(int index) {
    return mNames[index];
}

QStringList TextFileStore::itemSample(const QString& except) const {
    QStringList r;
    for (auto& n: mNames) {
        if (!except.isEmpty() && n == except) continue;
        r.append(n);
    }
    return r;
}
