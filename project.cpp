#include "project.h"
#include "gl_widget.h"
#include "gl_lang_runner.h"
#include "imagestore.h"
#include "modelstore.h"
#include "textfilestore.h"
#include "scope.h"

#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QIcon>
#include <QRegExp>


static QString uniqueName(const QString& key, const QStringList& names) {
    QRegExp ex("(.*)_(\\d+)$");
    QString pkey = key;
    int pindex = 0;
    if (ex.indexIn(key) != -1) {
        pkey = ex.cap(1);
        pindex = ex.cap(2).toInt();
    }
    QSet<int> hits;
    foreach(QString name, names) {
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

Demo::Project::~Project() {
    delete mGlobals;
}


Demo::Project::Project(const QDir& pdir, GLWidget* target, const Scope* globals, bool autoCompileOn):
    QAbstractItemModel(target),
    mProjectDir(pdir),
    mProjectIni(""),
    mGlobals(globals->clone(this)),
    mShaders(new TextFileStore("shaders", mGlobals, this)),
    mInit(0),
    mDraw(0),
    mTarget(target),
    mAutoCompileOn(autoCompileOn) {
    if (!mProjectDir.exists() || !mProjectDir.isReadable())
        throw BadProject(QString("Project dir %1 is not readable").arg(mProjectDir.absolutePath()));

    connect(this, SIGNAL(initChanged()), mTarget, SLOT(initChanged()));
    connect(this, SIGNAL(drawChanged()), mTarget, SLOT(drawChanged()));

    CodeEditor* ed = new CodeEditor("Init", mGlobals, this);
    mGlobals->appendEditor(ed, "clearcolor vec(.1,.2,.2,1)\n", "");
    ed = new CodeEditor("Draw", mGlobals, this);
    mGlobals->appendEditor(ed, "clear color_buffer_bit\n", "");

    mModels = dynamic_cast<ModelStore*>(mTarget->blob(globals->symbols(), "modelstore"));
    mImages = dynamic_cast<ImageStore*>(mTarget->texBlob(globals->symbols(), "imagestore"));

    mModels->clean();
    mImages->clean();

    setInitScript("Init");
    setDrawScript("Draw");
}

Demo::Project::Project(const QString& path, GLWidget* target, const Scope* globals, bool autoCompileOn):
    QAbstractItemModel(target),
    mGlobals(globals->clone(this)),
    mShaders(new TextFileStore("shaders", mGlobals, this)),
    mInit(0),
    mDraw(0),
    mTarget(target),
    mAutoCompileOn(autoCompileOn) {

    connect(this, SIGNAL(initChanged()), mTarget, SLOT(initChanged()));
    connect(this, SIGNAL(drawChanged()), mTarget, SLOT(drawChanged()));

    QFileInfo info(path);
    if (!info.exists()) throw BadProject(QString("Project file %1 does not exist").arg(path));
    if (!info.isFile() || !info.isReadable()) throw BadProject(QString("Project file %1 is not readable").arg(path));

    mProjectDir = info.absoluteDir();
    mProjectIni = info.fileName();

    QSettings project(path, QSettings::IniFormat);

    project.beginGroup("Models");
    NameMap models;
    foreach (const QString &key, project.childKeys()) {
        QString value = project.value(key).toString();
        QString fname = value;
        QFileInfo info(fname);
        if (info.isRelative()) {
            fname = mProjectDir.absoluteFilePath(fname);
            info = QFileInfo(fname);
        }
        if (!info.exists() || !info.isFile() || !info.isReadable()) {
            value = "";
        }

        models[uniqueName(key, models.keys())] = value;
    }
    project.endGroup();

    project.beginGroup("Textures");
    NameMap textures;
    foreach (const QString &key, project.childKeys()) {
        QString value = project.value(key).toString();
        QString fname = value;
        QFileInfo info(fname);
        if (info.isRelative()) {
            fname = mProjectDir.absoluteFilePath(fname);
            info = QFileInfo(fname);
        }
        if (!info.exists() || !info.isFile() || !info.isReadable()) {
            value = "";
        }

        textures[uniqueName(key, textures.keys())] = value;
    }
    project.endGroup();

    project.beginGroup("Shaders");
    foreach (const QString &key, project.childKeys()) {
        QString value = project.value(key).toString();
        QString fname = value;
        QFileInfo info(fname);
        if (info.isRelative()) {
            fname = mProjectDir.absoluteFilePath(fname);
            info = QFileInfo(fname);
        }
        if (!info.exists() || !info.isFile() || !info.isReadable()) {
            value = "";
        }

        mShaders->setText(uniqueShaderName(key), value);

    }
    project.endGroup();

    project.beginGroup("Scripts");
    foreach (const QString &key, project.childKeys()) {
        QString value = project.value(key).toString();
        QString fname = value;
        QFileInfo info(fname);
        if (info.isRelative()) {
            fname = mProjectDir.absoluteFilePath(fname);
            info = QFileInfo(fname);
        }
        if (!info.exists()) throw BadProject(QString("Script file \"%1\" does not exist").arg(value));
        if (!info.isFile() || !info.isReadable()) throw BadProject(QString("Script file \"%1\" is not readable").arg(value));

        QFile file(fname);
        file.open(QFile::ReadOnly);
        CodeEditor* ed = new CodeEditor(uniqueScriptName(key), mGlobals, this);
        mGlobals->appendEditor(ed, QString(file.readAll()), value);

        file.close();

    }
    project.endGroup();

    if (project.status() != QSettings::NoError) throw BadProject(QString("%1 is not a valid project file").arg(path));

    // safe to update globals
    mModels = dynamic_cast<ModelStore*>(mTarget->blob(globals->symbols(), "modelstore"));
    mModels->clean();
    NameIterator itm(models);
    while (itm.hasNext()) {
        itm.next();
        mModels->setModel(itm.key(), itm.value());
    }

    mImages = dynamic_cast<ImageStore*>(mTarget->texBlob(globals->symbols(), "imagestore"));
    mImages->clean();
    NameIterator iti(textures);
    while (iti.hasNext()) {
        iti.next();
        mImages->setImage(iti.key(), iti.value());
    }


    project.beginGroup("Roles");
    QString initKey = project.value("Init", "unknown_init").toString();
    setInitScript(initKey);
    QString drawKey = project.value("Draw", "unknown_draw").toString();
    setDrawScript(drawKey);
    project.endGroup();
}

void Demo::Project::saveProject() {
    QSettings project(mProjectDir.absoluteFilePath(mProjectIni), QSettings::IniFormat);
    project.clear();

    project.beginGroup("Scripts");
    foreach (CodeEditor* ed, mGlobals->editors()) {
        project.setValue(ed->objectName(), ed->fileName());
    }
    project.endGroup();

    project.beginGroup("Roles");
    project.setValue("Init", "None");
    if (mInit) {
        project.setValue("Init", mInit->objectName());
    }
    project.setValue("Draw", "None");
    if (mDraw) {
        project.setValue("Draw", mDraw->objectName());
    }
    project.endGroup();

    project.beginGroup("Models");
    for (int i = 0; i < mModels->size(); ++i) {
        project.setValue(mModels->modelName(i), mModels->fileName(i));
    }
    project.endGroup();

    project.beginGroup("Textures");
    for (int i = 0; i < mImages->size(); ++i) {
        project.setValue(mImages->imageName(i), mImages->fileName(i));
    }
    project.endGroup();

    project.beginGroup("Shaders");
    for (int i = 0; i < mShaders->size(); ++i) {
        project.setValue(mShaders->itemName(i), mShaders->fileName(i));
    }
    project.endGroup();

}

void Demo::Project::setProjectFile(const QString& fname) {
    QFileInfo info(fname);
    mProjectIni = info.fileName();
    mProjectDir = info.absoluteDir();
    // qDebug() << "setProjectFile" << mProjectDir.absolutePath() << mProjectIni;
}

void Demo::Project::scriptCompiled() {
    if (mInit && mGlobals->subscriptRelation(mInit->objectName(), sender()->objectName())) {
        emit initChanged();
    } else if (mDraw && mGlobals->subscriptRelation(mDraw->objectName(), sender()->objectName())) {
        emit drawChanged();
    }
}

void Demo::Project::scriptModification_changed(bool edited) {
    scriptStatus_changed();
    emit scriptModificationChanged(edited);
}

void Demo::Project::scriptStatus_changed() {
    QModelIndex lower = index(0, ScriptItems);
    QModelIndex upper = index(mGlobals->editors().size() - 1, itemParent(ScriptItems));
    emit dataChanged(lower, upper);
}

QString Demo::Project::initScript() const {
    if (mInit) return mInit->objectName();
    return "";
}

QString Demo::Project::drawScript() const {
    if (mDraw) return mDraw->objectName();
    return "";
}

void Demo::Project::setInitScript(const QString& name) {
    if (mInit) {
        disconnect(mTarget, SIGNAL(init()), mInit, SLOT(run()));
    }

    mInit = mGlobals->editor(name);

    if (mInit) {
        connect(mTarget, SIGNAL(init()), mInit, SLOT(run()));
    }

    emit initChanged();
}

void Demo::Project::setDrawScript(const QString& name) {
    if (mDraw) {
        disconnect(mTarget, SIGNAL(draw()), mDraw, SLOT(run()));
    }

    mDraw = mGlobals->editor(name);

    if (mDraw) {
        connect(mTarget, SIGNAL(draw()), mDraw, SLOT(run()));
    }

    emit drawChanged();
}

void Demo::Project::toggleAutoCompile(bool on) {
    mAutoCompileOn = on;
    foreach (CodeEditor* ed, mGlobals->editors()) {
        ed->toggleAutoCompile(on);
    }
}

QString Demo::Project::uniqueScriptName(const QString& orig) const {
    QStringList names;

    foreach (CodeEditor* ed, mGlobals->editors()) {
        names.append(ed->objectName());
    }

    return uniqueName(orig, names);
}

QString Demo::Project::uniqueModelName(const QString& orig) const {
    QStringList names;
    for (int i = 0; i < mModels->size(); ++i) {
        names.append(mModels->modelName(i));
    }

    return uniqueName(orig, names);
}


QString Demo::Project::uniqueImageName(const QString& orig) const {
    QStringList names;
    for (int i = 0; i < mImages->size(); ++i) {
        names.append(mImages->imageName(i));
    }

    return uniqueName(orig, names);
}

QString Demo::Project::uniqueShaderName(const QString& orig) const {
    QStringList names;
    for (int i = 0; i < mShaders->size(); ++i) {
        names.append(mShaders->itemName(i));
    }

    return uniqueName(orig, names);
}

QModelIndex Demo::Project::itemParent(ItemType kind) const{
    return index(kind, QModelIndex());
}


QModelIndex Demo::Project::index(int row, int column, const QModelIndex &parent) const {

    if (!hasIndex(row, column, parent)) return QModelIndex();
    if (!parent.isValid()) return createIndex(row, column, row);
    int internal_id;
    if (parent.internalId() == ScriptItems) {
        internal_id = NumItemTypes + row;
    } else if (parent.internalId() == ModelItems) {
        internal_id = NumItemTypes + mGlobals->editors().size() + row;
    } else if (parent.internalId() == TextureItems) {
        internal_id = NumItemTypes + mGlobals->editors().size() + mModels->size() + row;
    } else if (parent.internalId() == ShaderItems) {
        internal_id = NumItemTypes + mGlobals->editors().size() + mModels->size() + mImages->size() + row;
    } else {
        return QModelIndex();
    }

    return createIndex(row, column, internal_id);
}


QModelIndex Demo::Project::index(int row, const QModelIndex &parent) const {
    return index(row, 0, parent);
}


QModelIndex Demo::Project::index(int row, ItemType kind) const {
    return index(row, 0, itemParent(kind));
}

QModelIndex Demo::Project::parent(const QModelIndex &index) const {

    if (!index.isValid()) return QModelIndex();

    int id = index.internalId();

    int offset = NumItemTypes;
    if (id < offset) return QModelIndex();
    offset += mGlobals->editors().size();
    if (id < offset) return createIndex(ScriptItems, 0, ScriptItems);
    offset += mModels->size();
    if (id < offset) return createIndex(ModelItems, 0, ModelItems);
    offset += mImages->size();
    if (id < offset) return createIndex(TextureItems, 0, TextureItems);
    offset += mShaders->size();
    if (id < offset) return createIndex(ShaderItems, 0, ShaderItems);

    return QModelIndex();
}

QVariant Demo::Project::headerData(int, Qt::Orientation, int) const {
    return QVariant();
}

int Demo::Project::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) return NumItemTypes;
    int id = parent.internalId();
    if (id == ScriptItems) return mGlobals->editors().size();
    if (id == ModelItems) return mModels->size();
    if (id == TextureItems) return mImages->size();
    if (id == ShaderItems) return mShaders->size();

    return 0;
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
            if (index == itemParent(TextureItems)) return QString("Textures");
            if (index == itemParent(ShaderItems)) return QString("Shaders");
        }

        return QVariant();
    }


    if (index.parent() == itemParent(ScriptItems)) {

        CodeEditor* ed = mGlobals->editor(index.row());
        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(ed->objectName());
        }

        if (role == Qt::DecorationRole) {
            if (ed->hasCompileError()) {
                return QIcon::fromTheme("error");
            }
            if (ed->hasRunError()) {
                return QIcon::fromTheme("error");
            }
            if (ed->document()->isModified()) {
                return QIcon::fromTheme("document-save");
            }
            if (!ed->fileName().isEmpty()) {
                return QIcon::fromTheme("text-x-generic");
            }
            return QVariant();
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(ed->fileName());
        }

        if (role == FileNameRole) {
            return QVariant::fromValue(ed->fileName());
        }

        if (role == ScriptRole) {
            return QVariant::fromValue(ed->toPlainText());
        }

        if (role == EditorRole) {
            QWidget* widget = ed;
            return QVariant::fromValue(widget);
        }

        return QVariant();
    }

    if (index.parent() == itemParent(ModelItems)) {

        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(mModels->modelName(index.row()));
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(mModels->fileName(index.row()));
        }

        if (role == FileNameRole) {
            return QVariant::fromValue(mModels->fileName(index.row()));
        }

        if (role == Qt::DecorationRole) {
            if (!mModels->fileName(index.row()).isEmpty()) {
                return QIcon::fromTheme("text-x-generic");
            }
        }

        return QVariant();
    }

    if (index.parent() == itemParent(TextureItems)) {
        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(mImages->imageName(index.row()));
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(mImages->fileName(index.row()));
        }

        if (role == FileNameRole) {
            return QVariant::fromValue(mImages->fileName(index.row()));
        }

        if (role == Qt::DecorationRole) {
            if (!mImages->fileName(index.row()).isEmpty()) {
                return QIcon::fromTheme("image-x-generic");
            }
        }


        return QVariant();
    }

    if (index.parent() == itemParent(ShaderItems)) {
        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(mShaders->itemName(index.row()));
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(mShaders->fileName(index.row()));
        }

        if (role == FileNameRole) {
            return QVariant::fromValue(mShaders->fileName(index.row()));
        }

        if (role == Qt::DecorationRole) {
            if (!mShaders->fileName(index.row()).isEmpty()) {
                return QIcon::fromTheme("text-x-generic");
            }
        }


        return QVariant();
    }


    return QVariant();
}

bool Demo::Project::setData(const QModelIndex &index, const QVariant &value, int role) {

    if (!index.isValid()) return false;


    if (index.parent() == itemParent(ScriptItems)) {

        CodeEditor* ed = mGlobals->editor(index.row());
        if (role == Qt::EditRole) {
            QString newname = value.toString();
            if (ed->objectName() != newname) {
                mGlobals->rename(ed, uniqueScriptName(newname));
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileRole) {
            QString fname = value.toString();
            QString cfname = ed->fileName();

            QFile file(fname);
            if (file.open(QFile::ReadOnly)) {
                ed->setPlainText(file.readAll());
                file.close();
            }

            if (cfname != fname) {
                ed->setFileName(fname);
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileNameRole) {
            QString fname = value.toString();
            if (ed->fileName() != fname) {
                ed->setFileName(fname);
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == ScriptRole) {
            ed->setPlainText(value.toString());
            return true;
        }
        return false;
    }

    if (index.parent() == itemParent(ModelItems)) {
        if (role == Qt::EditRole) {
            QString newname = value.toString();
            QString curr = mModels->modelName(index.row());
            if (curr != newname) {
                mModels->rename(curr, uniqueModelName(newname));
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileRole) {
            QString fname = value.toString();
            QString name = mModels->modelName(index.row());

            QFileInfo info(fname);
            if (info.isRelative()) {
                fname = mProjectDir.absoluteFilePath(fname);
                info = QFileInfo(fname);
            }

            if (info.exists() && info.isFile() && info.isReadable()) {
                // TODO: throw error otherwise
                mModels->setModel(name, fname);
                emit dataChanged(index, index);
            }
            return true;
        }

        return false;
    }

    if (index.parent() == itemParent(TextureItems)) {
        if (role == Qt::EditRole) {
            QString newname = value.toString();
            QString curr = mImages->imageName(index.row());
            if (curr != newname) {
                mImages->rename(curr, uniqueImageName(newname));
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileRole) {
            QString fname = value.toString();
            QString name = mImages->imageName(index.row());

            QFileInfo info(fname);
            if (info.isRelative()) {
                fname = mProjectDir.absoluteFilePath(fname);
                info = QFileInfo(fname);
            }

            if (info.exists() && info.isFile() && info.isReadable()) {
                // TODO: throw error otherwise
                mImages->setImage(name, fname);
                emit dataChanged(index, index);
            }
        }

        return false;
    }

    if (index.parent() == itemParent(ShaderItems)) {
        if (role == Qt::EditRole) {
            QString newname = value.toString();
            QString curr = mShaders->itemName(index.row());
            if (curr != newname) {
                mShaders->rename(curr, uniqueShaderName(newname));
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileRole) {
            QString fname = value.toString();
            QString name = mShaders->itemName(index.row());

            QFileInfo info(fname);
            if (info.isRelative()) {
                fname = mProjectDir.absoluteFilePath(fname);
                info = QFileInfo(fname);
            }

            if (info.exists() && info.isFile() && info.isReadable()) {
                // TODO: throw error otherwise
                mShaders->setText(name, fname);
                emit dataChanged(index, index);
            }
        }

        return false;
    }

    return false;
}


Qt::ItemFlags Demo::Project::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}


bool Demo::Project::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!parent.isValid())
        return false;

    if (count != 1 || row < 0)
        return false;


    if (parent == itemParent(ScriptItems)) {

        if (row >= mGlobals->editors().size())
            return false;

        beginRemoveRows(parent, row, row);
        mGlobals->removeEditor(row);
        endRemoveRows();

    } else if (parent == itemParent(ModelItems)) {

        if (row >= mModels->size())
            return false;

        beginRemoveRows(parent, row, row);
        mModels->remove(row);
        endRemoveRows();

    } else if (parent == itemParent(TextureItems)) {

        if (row >= mImages->size())
            return false;

        beginRemoveRows(parent, row, row);
        mImages->remove(row);
        endRemoveRows();
    } else if (parent == itemParent(ShaderItems)) {

        if (row >= mShaders->size())
            return false;

        beginRemoveRows(parent, row, row);
        mShaders->remove(row);
        endRemoveRows();
    }

    emit dataChanged(index(row, 0, parent), index(row, 0, parent));
    return true;
}


bool Demo::Project::appendRow(const QString& name, const QString& file, const QModelIndex &parent) {

    if (!parent.isValid())
        return false;

    int row = rowCount(parent);
    QModelIndex new_idx = index(row, parent);

    if (parent == itemParent(ScriptItems)) {

        beginInsertRows(parent, row, row);
        CodeEditor* ed = new CodeEditor(uniqueScriptName(name), mGlobals, this);
        mGlobals->appendEditor(ed, "// new commands here\n", file);
        endInsertRows();

    } else if (parent == itemParent(ModelItems)) {
        beginInsertRows(parent, row, row);
        QString fname = file;
        QFileInfo info(fname);
        if (info.isRelative()) {
            fname = mProjectDir.absoluteFilePath(fname);
            info = QFileInfo(fname);
        }

        if (info.exists() && info.isFile() && info.isReadable()) {
            mModels->setModel(uniqueModelName(name), fname);
        } else {
            mModels->setModel(uniqueModelName(name));
        }
        endInsertRows();

    } else if (parent == itemParent(TextureItems)) {
        beginInsertRows(parent, row, row);
        QString fname = file;
        QFileInfo info(fname);
        if (info.isRelative()) {
            fname = mProjectDir.absoluteFilePath(fname);
            info = QFileInfo(fname);
        }

        if (info.exists() && info.isFile() && info.isReadable()) {
            mImages->setImage(uniqueImageName(name), fname);
        } else {
            mImages->setImage(uniqueImageName(name));
        }
        endInsertRows();

    } else if (parent == itemParent(ShaderItems)) {
        beginInsertRows(parent, row, row);
        QString fname = file;
        QFileInfo info(fname);
        if (info.isRelative()) {
            fname = mProjectDir.absoluteFilePath(fname);
            info = QFileInfo(fname);
        }

        if (info.exists() && info.isFile() && info.isReadable()) {
            mShaders->setText(uniqueShaderName(name), fname);
        } else {
            mShaders->setText(uniqueShaderName(name));
        }
        endInsertRows();

    } else {
        return false;
    }

    emit dataChanged(new_idx, new_idx);

    return true;
}


