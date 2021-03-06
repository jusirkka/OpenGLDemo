#ifndef PROJECT_H
#define PROJECT_H

#include <QAbstractItemModel>
#include <QDir>

#include "codeeditor.h"
#include "symbol.h"


class QFileSystemWatcher;

namespace Demo {

class GLWidget;
class Scope;
class ProjectFolder;

class TextFileStore;

namespace GL {

class ImageStore;
class ModelStore;

}

class BadProject {

public:
    BadProject(QString msg): mMsg(std::move(msg)) {}
    QString msg() {return mMsg;}

private:
    QString mMsg;
};




class Project : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum ProjectRoles {
        FileRole = Qt::UserRole,
        ScriptRole,
        EditorRole,
        FileNameRole
    };

    enum ItemType {
        ScriptItems = 0,
        ModelItems,
        ImageItems,
        TextureItems,
        ShaderItems,
        NumItemTypes
    };


public:
    // create new project
    Project(const QDir& pdir, GLWidget* target, const Scope* globals, bool autoCompileOn);
    // parse existing project
    Project(const QString& fullpath, GLWidget* target, const Scope* globals, bool autoCompileOn);

    const QDir& directory() const {return mProjectDir;}
    const QString& projectFile() const {return mProjectIni;}
    void setProjectFile(const QString& fname);
    bool autoCompileEnabled() const {return mAutoCompileOn;}
    void toggleAutoCompile(bool on);
    QString initScriptName() const {return INIT_NAME;}
    QString drawScriptName() const {return DRAW_NAME;}

    void saveProject();

    QModelIndex itemParent(ItemType key) const;

    //! Reimplemented from QAbstractItemModel
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, const QModelIndex &parent) const;

    QModelIndex index(int row, ItemType parent) const;

    //! Reimplemented from QAbstractItemModel
    QModelIndex parent(const QModelIndex &index) const override;

    //! Reimplemented from QAbstractItemModel
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    //! Reimplemented from QAbstractItemModel
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;

    //! Reimplemented from QAbstractItemModel
    int rowCount(const QModelIndex &parent) const override;

    //! Reimplemented from QAbstractItemModel
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    //! Reimplemented from QAbstractItemModel
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    //! Reimplemented from QAbstractItemModel
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    //! Reimplemented from QAbstractItemModel
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool appendRow(const QString& name, const QString& file, const QModelIndex& parent);


    ~Project() override;

public slots:

    void scriptCompiled();

    void scriptModification_changed(bool edited);
    void scriptStatus_changed();

    void recompileProject();


private slots:

    void fileChanged(const QString& path);


signals:

    void initChanged();
    void drawChanged();
    void scriptModificationChanged(bool edited);

private:

    using NameMap = QMap<QString, QString>;
    using NameIterator = QMapIterator<QString, QString>;
    using FolderMap = QMap<ItemType, ProjectFolder*>;
    using FolderIterator = QMapIterator<ItemType, ProjectFolder*>;

    QString fullpath(const QString& path) const;
    bool isReadable(const QString& path) const;
    bool isWritable(const QString& path) const;

private:

    QDir mProjectDir;
    QString mProjectIni;
    FolderMap mFolders;
    CodeEditor* mInit;
    CodeEditor* mDraw;
    GLWidget* mTarget;
    bool mAutoCompileOn;
    QFileSystemWatcher* mWatcher;

};

}
#endif // PROJECT_H
