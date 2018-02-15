#include "project.h"
#include "gl_widget.h"
#include "gl_lang_runner.h"
#include "imagestore.h"
#include "modelstore.h"
#include "textfilestore.h"
#include "scope.h"
#include "texturestore.h"

#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QIcon>
#include <QRegExp>
#include <QFileSystemWatcher>


static QString uniqueName(const QString& key, QStringList names, const QString& k = QString()) {
    QRegExp ex(R"((.*)_(\d+)$)");
    QString pkey = key;
    int pindex = 0;
    if (ex.indexIn(key) != -1) {
        pkey = ex.cap(1);
        pindex = ex.cap(2).toInt();
    }

    if (!k.isNull() && names.contains(k)) {
        names.removeOne(k);
    }

    QSet<int> hits;
    for (auto& name: names) {
        QString pname = name;
        int hitindex = 0;
        if (ex.indexIn(name) != -1) {
            pname = ex.cap(1);
            hitindex = ex.cap(2).toInt();
        }
        if (pname == pkey) {
            hits.insert(hitindex);
        }
    }

    if (hits.isEmpty()) return key;

    while (hits.contains(pindex)) pindex += 1;

    return QString("%1_%2").arg(pkey).arg(pindex);

}


using Demo::GL::ImageStore;
using Demo::GL::ModelStore;
using Demo::CodeEditor;
using Demo::TextFileStore;
using Demo::TextureStore;

Demo::Project::~Project() {}


Demo::Project::Project(const QDir& pdir, GLWidget* target, const Scope* globals, bool autoCompileOn)
    : QAbstractItemModel(target)
    , mProjectDir(pdir)
    , mProjectIni("")
    , mTarget(target)
    , mAutoCompileOn(autoCompileOn)
{
    if (!mProjectDir.exists() || !mProjectDir.isReadable())
        throw BadProject(QString("Project dir %1 is not readable").arg(mProjectDir.absolutePath()));

    mWatcher = new QFileSystemWatcher(this);
    auto editors = globals->clone(this);

    connect(this, SIGNAL(initChanged()), mTarget, SLOT(initChanged()));
    connect(this, SIGNAL(drawChanged()), mTarget, SLOT(drawChanged()));
    connect(mWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChanged(const QString&)));

    editors->setItem(INIT_NAME);
    editors->setItem(DRAW_NAME);
    mInit = editors->editor(INIT_NAME);
    mDraw = editors->editor(DRAW_NAME);

    connect(mTarget, SIGNAL(init()), mInit, SLOT(run()));
    connect(mTarget, SIGNAL(draw()), mDraw, SLOT(run()));

    auto models = dynamic_cast<ModelStore*>(mTarget->blob(globals->symbols(), "modelstore"));
    auto images = dynamic_cast<ImageStore*>(mTarget->texBlob(globals->symbols(), "imagestore"));

    models->clean();
    images->clean();

    auto shaders = new TextFileStore("shaders", editors, this);
    auto textures = new TextureStore("textures", editors, mTarget, this);

    mFolders[ScriptItems] = editors;
    mFolders[ModelItems] = models;
    mFolders[ImageItems] = images;
    mFolders[ShaderItems] = shaders;
    mFolders[TextureItems] = textures;

    recompileProject();
}


Demo::Project::Project(const QString& path, GLWidget* target, const Scope* globals, bool autoCompileOn)
    : QAbstractItemModel(target)
    , mTarget(target)
    , mAutoCompileOn(autoCompileOn)
{

    QFileInfo info(path);
    if (!info.exists()) throw BadProject(QString("Project file %1 does not exist").arg(path));
    if (!info.isFile() || !info.isReadable()) throw BadProject(QString("Project file %1 is not readable").arg(path));

    mWatcher = new QFileSystemWatcher(this);

    connect(this, SIGNAL(initChanged()), mTarget, SLOT(initChanged()));
    connect(this, SIGNAL(drawChanged()), mTarget, SLOT(drawChanged()));
    connect(mWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(fileChanged(const QString&)));


    mProjectDir = info.absoluteDir();
    mProjectIni = info.fileName();

    QSettings project(path, QSettings::IniFormat);

    project.beginGroup("Models");
    NameMap modelmap;
    for (auto& key: project.childKeys()) {
        modelmap[uniqueName(key, modelmap.keys())] = fullpath(project.value(key).toString());
    }
    project.endGroup();

    project.beginGroup("Images");
    NameMap imagemap;
    for (auto& key: project.childKeys()) {
        imagemap[uniqueName(key, imagemap.keys())] = fullpath(project.value(key).toString());
    }
    project.endGroup();

    auto editors = globals->clone(this);
    auto shaders = new TextFileStore("shaders", editors, this);
    project.beginGroup("Shaders");
    for (auto& key: project.childKeys()) {
        auto fpath = fullpath(project.value(key).toString());
        shaders->setItem(uniqueName(key, shaders->items()), fpath);
        if (!fpath.isEmpty()) {
            // qDebug() << "adding (project) " << fname;
            if (!mWatcher->addPath(fpath)) {
                qWarning() << "Ctor: Cannot watch" << fpath;
            }
        }
    }
    project.endGroup();

    auto textures = new TextureStore("textures", editors, mTarget, this);
    project.beginGroup("Textures");
    for (auto& key: project.childKeys()) {
        auto fpath = fullpath(project.value(key).toString());
        textures->setItem(uniqueName(key, shaders->items()), fpath);
    }
    project.endGroup();

    project.beginGroup("Scripts");
    for (auto& key: project.childKeys()) {
        editors->setItem(uniqueName(key, editors->items()), fullpath(project.value(key).toString()));
    }
    project.endGroup();

    if (project.status() != QSettings::NoError) throw BadProject(QString(R"(%1 is not a valid project file)").arg(path));

    mInit = editors->editor(INIT_NAME);
    if (!mInit) {
        throw BadProject(QString(R"("%1" missing in the Scripts section of "%2")").arg(INIT_NAME, path));
    }
    connect(mTarget, SIGNAL(init()), mInit, SLOT(run()));

    mDraw = editors->editor(DRAW_NAME);
    if (!mDraw) {
        throw BadProject(QString(R"("%1" missing in the Scripts section of "%2")").arg(DRAW_NAME, path));
    }
    connect(mTarget, SIGNAL(draw()), mDraw, SLOT(run()));

    // safe to update globals
    auto models = dynamic_cast<ModelStore*>(mTarget->blob(globals->symbols(), "modelstore"));
    models->clean();
    NameIterator itm(modelmap);
    while (itm.hasNext()) {
        itm.next();
        models->setItem(itm.key(), itm.value());
    }

    auto images = dynamic_cast<ImageStore*>(mTarget->texBlob(globals->symbols(), "imagestore"));
    images->clean();
    NameIterator iti(imagemap);
    while (iti.hasNext()) {
        iti.next();
        images->setItem(iti.key(), iti.value());
    }

    mFolders[ScriptItems] = editors;
    mFolders[ModelItems] = models;
    mFolders[ImageItems] = images;
    mFolders[ShaderItems] = shaders;
    mFolders[TextureItems] = textures;

    recompileProject();
}

QString Demo::Project::fullpath(const QString &v) {
    QFileInfo info(v);
    if (!info.isAbsolute()) {
        info = QFileInfo(mProjectDir.absoluteFilePath(v));
    }
    QString fname = info.canonicalFilePath();
    if (!v.isEmpty() && (!info.exists() || !info.isFile() || !info.isReadable())) {
        throw BadProject(QString(R"(Project file "%1" is not readable)").arg(v));
    }
    return v.isEmpty() ? v : fname;
}

void Demo::Project::fileChanged(const QString& path) {
    // qDebug() << path;
    for (int i = 0; i < rowCount(itemParent(ShaderItems)); i++) {
        QModelIndex addr = index(i, ShaderItems);
        QString fname = data(addr, FileNameRole).toString();
        if (fname == path) {
            // qDebug() << "set data";
            setData(addr, path, FileRole);
            // Does not work without resetting!?
            if (!mWatcher->addPath(path)) {
                qWarning() << "fileChanged: Cannot watch" << path;
            }
            return;
        }
    }
}


void Demo::Project::recompileProject() {
    auto scope = dynamic_cast<Scope*>(mFolders[ScriptItems]);
    scope->recompileAll();
    if (mInit->compiler()->ready()) {
        emit initChanged();
    }
}

void Demo::Project::saveProject() {
    QSettings project(mProjectDir.absoluteFilePath(mProjectIni), QSettings::IniFormat);
    project.clear();

    QMap<ItemType, QString> groups;
    groups[ScriptItems] = "Script";
    groups[ImageItems] = "Images";
    groups[ModelItems] = "Models";
    groups[ShaderItems] = "Shaders";
    groups[TextureItems] = "Textures";

    QMapIterator<ItemType, QString> itg(groups);
    while (itg.hasNext()) {
        itg.next();
        project.beginGroup(itg.value());
        auto folder = mFolders[itg.key()];
        for (int i = 0; i < folder->size(); ++i) {
            project.setValue(folder->itemName(i), mProjectDir.relativeFilePath(folder->fileName(i)));
        }
        project.endGroup();
    }
}

void Demo::Project::setProjectFile(const QString& fname) {
    QFileInfo info(fname);
    mProjectIni = info.fileName();
    mProjectDir = info.absoluteDir();
    // qDebug() << "setProjectFile" << mProjectDir.absolutePath() << mProjectIni;
}

void Demo::Project::scriptCompiled() {
    auto scope = dynamic_cast<Scope*>(mFolders[ScriptItems]);
    if (scope->subscriptRelation(mInit->objectName(), sender()->objectName())) {
        emit initChanged();
    } else if (scope->subscriptRelation(mDraw->objectName(), sender()->objectName())) {
        emit drawChanged();
    }
}

void Demo::Project::scriptModification_changed(bool edited) {
    scriptStatus_changed();
    emit scriptModificationChanged(edited);
}

void Demo::Project::scriptStatus_changed() {
    auto scope = mFolders[ScriptItems];
    QModelIndex lower = index(0, ScriptItems);
    QModelIndex upper = index(scope->size() - 1, itemParent(ScriptItems));
    emit dataChanged(lower, upper);
}

void Demo::Project::toggleAutoCompile(bool on) {
    mAutoCompileOn = on;
    auto scope = dynamic_cast<Scope*>(mFolders[ScriptItems]);
    for (auto ed: scope->editors()) {
        ed->toggleAutoCompile(on);
    }
}

QModelIndex Demo::Project::itemParent(ItemType kind) const{
    return index(kind, QModelIndex());
}

static int row2Internal(int parent, int row) {
    return 10000 * (parent + 1) + row + 1;
}

static int internal2ItemType(int id) {
    return id / 10000 - 1;
}

/*static int internal2Row(int id) {
    return id % 10000 - 1;
}*/

QModelIndex Demo::Project::index(int row, int column, const QModelIndex &parent) const {

    if (!hasIndex(row, column, parent)) return QModelIndex();
    if (!parent.isValid()) return createIndex(row, column, row);
    return createIndex(row, column, row2Internal(parent.internalId(), row));
}


QModelIndex Demo::Project::index(int row, const QModelIndex &parent) const {
    return index(row, 0, parent);
}


QModelIndex Demo::Project::index(int row, ItemType kind) const {
    return index(row, 0, itemParent(kind));
}

QModelIndex Demo::Project::parent(const QModelIndex &index) const {

    if (!index.isValid()) return QModelIndex();

    int itemType = internal2ItemType(index.internalId());
    if (itemType < 0) return QModelIndex();
    return createIndex(itemType, 0, itemType);
}

QVariant Demo::Project::headerData(int, Qt::Orientation, int) const {
    return QVariant();
}

int Demo::Project::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) return NumItemTypes;
    int id = parent.internalId();
    if (id < 0 || id >= NumItemTypes) return 0;
    return mFolders[static_cast<ItemType>(id)]->size();
}

int Demo::Project::columnCount(const QModelIndex&) const {
    return 1;
}

QVariant Demo::Project::data(const QModelIndex& index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }

    if (index.parent() == QModelIndex()) {
        if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("folder");
        }

        if (role == Qt::DisplayRole) {
            if (index == itemParent(ScriptItems)) return QString("Scripts");
            if (index == itemParent(ModelItems)) return QString("Models");
            if (index == itemParent(ImageItems)) return QString("Images");
            if (index == itemParent(ShaderItems)) return QString("Shaders");
            if (index == itemParent(TextureItems)) return QString("Textures");
        }

        return QVariant();
    }

    auto t = static_cast<ItemType>(index.parent().row());
    auto folder = mFolders[t];
    if (role == Qt::DisplayRole) {
        return QVariant::fromValue(folder->itemName(index.row()));
    }

    if (role == Qt::ToolTipRole || role == FileNameRole) {
        return QVariant::fromValue(folder->fileName(index.row()));
    }


    if (index.parent() == itemParent(ScriptItems)) {

        auto ed = dynamic_cast<Scope*>(folder)->editor(index.row());

        if (role == Qt::DecorationRole) {
            if (ed->hasCompileError()) return QIcon::fromTheme("error");
            if (ed->hasRunError()) return QIcon::fromTheme("error");
            if (ed->document()->isModified()) return QIcon::fromTheme("document-save");
            if (!ed->fileName().isEmpty()) return QIcon::fromTheme("text-x-generic");
            return QIcon::fromTheme("unknown");
        }

        if (role == ScriptRole) return QVariant::fromValue(ed->toPlainText());
        if (role == EditorRole) return QVariant::fromValue(qobject_cast<QWidget*>(ed));

        return QVariant();
    }

    if (index.parent() == itemParent(ModelItems) || index.parent() == itemParent(ShaderItems)) {
        if (role == Qt::DecorationRole) {
            if (!folder->fileName(index.row()).isEmpty()) {
                return QIcon::fromTheme("text-x-generic");
            }
            return QIcon::fromTheme("unknown");
        }
        return QVariant();
    }

    if (index.parent() == itemParent(ImageItems) || index.parent() == itemParent(TextureItems)) {
        if (role == Qt::DecorationRole) {
            if (!folder->fileName(index.row()).isEmpty()) {
                return QIcon::fromTheme("image-x-generic");
            }
            return QIcon::fromTheme("unknown");
        }
        return QVariant();
    }

    return QVariant();
}

bool Demo::Project::setData(const QModelIndex &index, const QVariant &value, int role) {

    if (!index.isValid()) return false;
    if (!index.parent().isValid()) return false;

    auto t = static_cast<ItemType>(index.parent().row());
    auto folder = mFolders[t];

    if (role == Qt::EditRole) {
        QString newname = value.toString();
        QString currname = folder->itemName(index.row());
        if (currname == newname) return false;
        folder->rename(currname, uniqueName(newname, folder->items(), currname));
    } else if (role == FileRole) {
        QString path = value.toString();
        QString name = folder->itemName(index.row());

        QFileInfo info(path);
        if (info.isRelative()) {
            info = QFileInfo(mProjectDir.absoluteFilePath(path));
        }
        path = info.canonicalFilePath();

        if (!info.exists() || !info.isFile() || !info.isReadable()) return false;

        if (dynamic_cast<TextFileStore*>(folder)) {
            QString oldpath = QFileInfo(folder->fileName(index.row())).canonicalFilePath();
            if (oldpath != path) {
                // qDebug() << "removing" << oldpath;
                if (!mWatcher->removePath(oldpath)) {
                    qWarning() << "setData: Cannot unwatch" << oldpath;
                }
                // qDebug() << "adding" << path;
                if (!mWatcher->addPath(path)) {
                    qWarning() << "setData: Cannot watch" << path;
                }
            }
        }

        folder->setItem(name, path);


    } else if (index.parent() == itemParent(ScriptItems)) {

        auto ed = dynamic_cast<Scope*>(folder)->editor(index.row());

        if (role == FileNameRole) {
            QString fname = value.toString();
            if (ed->fileName() == fname) return false;
            ed->setFileName(fname);
        } else if (role == ScriptRole) {
            ed->insertPlainText(value.toString());
        } else {
            return false;
        }

    } else {
        return false;
    }

    recompileProject();
    emit dataChanged(index, index);
    return true;
}


Qt::ItemFlags Demo::Project::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}


bool Demo::Project::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!parent.isValid()) return false;
    if (count != 1 || row < 0) return false;

    auto t = static_cast<ItemType>(parent.row());
    auto folder = mFolders[t];

    if (row >= folder->size()) return false;

    beginRemoveRows(parent, row, row);

    if (dynamic_cast<TextFileStore*>(folder)) {
        QString oldpath = data(index(row, ShaderItems), FileNameRole).toString();
        // qDebug() << "removing" << oldpath;
        if (!mWatcher->removePath(oldpath)) {
            qWarning() << "removeRows: Cannot unwatch" << oldpath;
        }
    }

    folder->remove(row);
    endRemoveRows();


    recompileProject();
    emit dataChanged(index(row, 0, parent), index(row, 0, parent));
    return true;
}


bool Demo::Project::appendRow(const QString& name, const QString& file, const QModelIndex &parent) {

    if (!parent.isValid()) return false;

    int row = rowCount(parent);
    QModelIndex new_idx = index(row, parent);

    auto t = static_cast<ItemType>(parent.row());
    auto folder = mFolders[t];

    beginInsertRows(parent, row, row);
    QString fname = file;
    QFileInfo info(fname);
    if (info.isRelative()) {
        info = QFileInfo(mProjectDir.absoluteFilePath(fname));
    }
    fname = info.canonicalFilePath();

    QString uniq = uniqueName(name, folder->items());
    if (info.exists() && info.isFile() && info.isReadable()) {

        folder->setItem(uniq, fname);

        if (dynamic_cast<TextFileStore*>(folder)) {
            if (!mWatcher->addPath(fname)) {
                qWarning() << "appendRow: Cannot watch" << fname;
            }
        }

    } else {
        folder->setItem(uniq);
    }
    endInsertRows();

    recompileProject();
    emit dataChanged(new_idx, new_idx);
    return true;
}


