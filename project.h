#ifndef PROJECT_H
#define PROJECT_H

#include <QAbstractListModel>
#include <QDir>

#include "codeeditor.h"

namespace Demo {

class GLWidget;

class BadProject {

public:
    BadProject(const QString& msg): mMsg(msg) {}
    QString msg() {return mMsg;}

private:
    QString mMsg;
};




class Project : public QAbstractListModel
{
    Q_OBJECT

public:

    static const int FileRole = Qt::UserRole;
    static const int GroupRole = Qt::UserRole + 1;
    static const int EditorRole = Qt::UserRole + 2;

public:
    // create new project
    Project(const QDir& pdir, GLWidget* target);
    // parse existing project
    Project(const QString& fullpath, GLWidget* target);

    const QDir& directory() const {return mProjectDir;}
    const QString& projectFile() const {return mProjectIni;}
    void setProjectFile(const QString& fname);
    bool modified() const {return mModified;}

    void saveProject();

    QString initGroup() const;
    QString drawGroup() const;

    //! Reimplemented from QAbstractItemModel
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const;

    //! Reimplemented from QAbstractItemModel
    int rowCount(const QModelIndex &parent) const;

    //! Reimplemented from QAbstractItemModel
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    //! Reimplemented from QAbstractItemModel
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    //! Reimplemented from QAbstractItemModel
    Qt::ItemFlags flags(const QModelIndex &index) const;

    //! Reimplemented from QAbstractItemModel
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    bool appendRow(const QString& name, const QString& file, const QString& group);

    virtual ~Project();

public slots:

    void runnerReady();
    void groupModified(bool);
    void setInitGroup(QString);
    void setDrawGroup(QString);

signals:

    void initChanged();
    void drawChanged();

private:

    typedef QList<CodeEditor*> EditorList;

private:

    QDir mProjectDir;
    QString mProjectIni;
    QStringList mNames;
    QStringList mFiles;
    EditorList mEditors;
    CodeEditor* mInit;
    CodeEditor* mDraw;
    GLWidget* mTarget;
    bool mModified;
};

}
#endif // PROJECT_H
