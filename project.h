#ifndef PROJECT_H
#define PROJECT_H

#include <QAbstractItemModel>
#include <QDir>

#include "codeeditor.h"

namespace GL {

class ImageStore;

}

namespace Demo {

class GLWidget;

class BadProject {

public:
    BadProject(const QString& msg): mMsg(msg) {}
    QString msg() {return mMsg;}

private:
    QString mMsg;
};




class Project : public QAbstractItemModel
{
    Q_OBJECT

public:

    static const int FileRole = Qt::UserRole;
    static const int GroupRole = Qt::UserRole + 1;
    static const int EditorRole = Qt::UserRole + 2;
    static const int FileNameRole = Qt::UserRole + 3;

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

    QModelIndex groupParent() const;
    QModelIndex modelParent() const;
    QModelIndex textureParent() const;

    //! Reimplemented from QAbstractItemModel
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    QModelIndex index(int row, const QModelIndex &parent) const;

    //! Reimplemented from QAbstractItemModel
    QModelIndex parent(const QModelIndex &index) const;

    //! Reimplemented from QAbstractItemModel
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

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

    bool appendRow(const QString& name, const QString& file, const QModelIndex& parent);

    virtual ~Project();

public slots:

    void runnerReady();
    void groupModified(bool);
    void setInitGroup(QString);
    void setDrawGroup(QString);

    void dispatcher(const QString& curr, const QString& other);

signals:

    void initChanged();
    void drawChanged();

private:

    class EditorTuple {
    public:
        CodeEditor* editor;
        QString filename;
    };

    class NameTuple {
    public:
        QString name;
        QString filename;
    };

    typedef QList<EditorTuple> EditorList;

    static const int GroupRow = 0;
    static const int ModelRow = 1;
    static const int TextureRow = 2;
    static const int NumRows = 3;

    typedef QList<NameTuple> NameList;

private:

    CodeEditor* appendEditor(const QString& name, const QString& group, const QString& file);
    void init_and_connect();


private:

    QDir mProjectDir;
    QString mProjectIni;
    EditorList mEditors;
    NameList mModels;
    CodeEditor* mInit;
    CodeEditor* mDraw;
    GLWidget* mTarget;
    bool mModified;
};

}
#endif // PROJECT_H
