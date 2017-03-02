#include "project.h"
#include "gl_widget.h"
#include "gl_lang_runner.h"
#include "imagestore.h"
#include "modelstore.h"

#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QIcon>
#include <QRegExp>


using Demo::GL::ImageStore;
using Demo::GL::ModelStore;
using Demo::CodeEditor;

Demo::Project::~Project() {
    foreach (CodeEditor* ed, mEditors) {
        delete ed;
    }
}


Demo::Project::Project(const QDir& pdir, GLWidget* target, SymbolMap& globals, bool autoCompileOn):
    QAbstractItemModel(target),
    mProjectDir(pdir),
    mProjectIni(""),
    mGlobalSymbols(globals),
    mInit(0),
    mDraw(0),
    mTarget(target),
    mAutoCompileOn(autoCompileOn) {
    if (!mProjectDir.exists() || !mProjectDir.isReadable())
        throw BadProject(QString("Project dir %1 is not readable").arg(mProjectDir.absolutePath()));

    connect(this, SIGNAL(initChanged()), mTarget, SLOT(initChanged()));
    connect(this, SIGNAL(drawChanged()), mTarget, SLOT(drawChanged()));

    appendEditor("Init", "clearcolor vec(.1,.2,.2,1)\n", "");
    appendEditor("Draw", "clear color_buffer_bit\n", "");

    mModels = dynamic_cast<ModelStore*>(mTarget->blob(mGlobalSymbols, "modelstore"));
    mImages = dynamic_cast<ImageStore*>(mTarget->blob(mGlobalSymbols, "imagestore"));

    mModels->clean();
    mImages->clean();

    setInitScript("Init");
    setDrawScript("Draw");
}

Demo::Project::Project(const QString& path, GLWidget* target, SymbolMap& globals, bool autoCompileOn):
    QAbstractItemModel(target),
    mGlobalSymbols(globals),
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

        appendEditor(uniqueScriptName(key), QString(file.readAll()), value);

        file.close();

    }
    project.endGroup();

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

        models[uniqueModelName(key)] = value;
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

        textures[uniqueImageName(key)] = value;
    }
    project.endGroup();

    if (project.status() != QSettings::NoError) throw BadProject(QString("%1 is not a valid project file").arg(path));

    mModels->clean();
    NameIterator itm(models);
    while (itm.hasNext()) {
        itm.next();
        mModels->setModel(itm.key(), itm.value());
    }

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
    foreach (CodeEditor* ed, mEditors) {
        project.setValue(ed->objectName(), mScripts.value(ed->objectName()));
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

    project.beginGroup("Textures");
    for (int i = 0; i < mImages->size(); ++i) {
        project.setValue(mImages->imageName(i), mImages->fileName(i));
    }
    project.endGroup();

    project.beginGroup("Models");
    for (int i = 0; i < mModels->size(); ++i) {
        project.setValue(mModels->modelName(i), mModels->fileName(i));
    }
    project.endGroup();
}

void Demo::Project::setProjectFile(const QString& fname) {
    QFileInfo info(fname);
    mProjectIni = info.fileName();
    mProjectDir = info.absoluteDir();
    qDebug() << "setProjectFile" << mProjectDir.absolutePath() << mProjectIni;
}

void Demo::Project::scriptCompiled() {
    if (sender() == mInit) {
        emit initChanged();
    } else if (sender() == mDraw) {
        emit drawChanged();
    } else {
        // TODO
        /*
        CodeEditor* ed = qobject_cast<CodeEditor*>(sender());
        if (TreeFind(mInit).find(ed)) {
            emit initChanged();
        }
        if (TreeFind(mDraw).find(ed)){
            emit drawChanged();
        }
        */
    }
}

void Demo::Project::scriptModification_changed(bool edited) {
    scriptStatus_changed();
    emit scriptModificationChanged(edited);
}

void Demo::Project::scriptStatus_changed() {
    QModelIndex lower = index(0, scriptParent());
    QModelIndex upper = index(mEditors.size() - 1, scriptParent());
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
        disconnect(mTarget, SIGNAL(init()), mInit, SLOT(evaluate()));
    }

    mInit = 0;
    foreach (CodeEditor* ed, mEditors) {
        if (ed->objectName() == name) {
            mInit = ed;
            break;
        }
    }

    if (mInit) {
        connect(mTarget, SIGNAL(init()), mInit, SLOT(evaluate()));
    }

    emit initChanged();
}

void Demo::Project::setDrawScript(const QString& name) {
    if (mDraw) {
        disconnect(mTarget, SIGNAL(draw()), mDraw, SLOT(evaluate()));
    }

    mDraw = 0;
    foreach (CodeEditor* ed, mEditors) {
        if (ed->objectName() == name) {
            mDraw = ed;
            break;
        }
    }

    if (mDraw) {
        connect(mTarget, SIGNAL(draw()), mDraw, SLOT(evaluate()));
    }

    emit drawChanged();
}


void Demo::Project::dispatch(const QString& other) const {
    qDebug() << "dispatch" << other;
    CodeEditor* other_ed = 0;
    foreach (CodeEditor* ed, mEditors) {
        if (ed->objectName() == other) {
            other_ed = ed;
            break;
        }
    }

    if (other_ed) {
        other_ed->run();
    }
}


static void setEditorText(CodeEditor* editor, const QString& group) {
    editor->setPlainText(group);
}

CodeEditor* Demo::Project::appendEditor(const QString& name, const QString& group, const QString& file) {
    CodeEditor* editor = new CodeEditor(this);
    editor->setObjectName(name);
    setEditorText(editor, group);
    mEditors.append(editor);
    mScripts[editor->objectName()] = file;
    editor->compile();
    return editor;
}

void Demo::Project::toggleAutoCompile(bool on) {
    mAutoCompileOn = on;
    foreach (CodeEditor* ed, mEditors) {
        ed->toggleAutoCompile(on);
    }
}

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

QString Demo::Project::uniqueScriptName(const QString& orig) const {
    QStringList names;

    foreach (CodeEditor* ed, mEditors) {
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

QModelIndex Demo::Project::scriptParent() const{
    return index(ScriptRow, QModelIndex());
}

QModelIndex Demo::Project::modelParent() const {
    return index(ModelRow, QModelIndex());
}

QModelIndex Demo::Project::textureParent() const {
    return index(TextureRow, QModelIndex());
}

QModelIndex Demo::Project::index(int row, int column, const QModelIndex &parent) const {

    if (!hasIndex(row, column, parent)) return QModelIndex();
    if (!parent.isValid()) return createIndex(row, column, row);
    if (parent.internalId() == ScriptRow)
        return createIndex(row, column, NumRows + row);
    if (parent.internalId() == ModelRow)
        return createIndex(row, column, NumRows + mEditors.size() + row);
    if (parent.internalId() == TextureRow)
        return createIndex(row, column, NumRows + mEditors.size() + mModels->size() + row);

    return QModelIndex();
}


QModelIndex Demo::Project::index(int row, const QModelIndex &parent) const {
    return index(row, 0, parent);
}

QModelIndex Demo::Project::parent(const QModelIndex &index) const {

    if (!index.isValid()) return QModelIndex();

    int id = index.internalId();

    if (id < NumRows) return QModelIndex();
    if (id < NumRows + mEditors.size()) return createIndex(ScriptRow, 0, ScriptRow);
    if (id < NumRows + mEditors.size() + mModels->size()) return createIndex(ModelRow, 0, ModelRow);
    if (id < NumRows + mEditors.size() + mModels->size() + mImages->size()) return createIndex(TextureRow, 0, TextureRow);

    return QModelIndex();
}

QVariant Demo::Project::headerData(int, Qt::Orientation, int) const {
    return QVariant();
}

int Demo::Project::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) return NumRows;
    int id = parent.internalId();
    if (id == ScriptRow) return mEditors.size();
    if (id == ModelRow) return mModels->size();
    if (id == TextureRow) return mImages->size();

    return 0;
}

int Demo::Project::columnCount(const QModelIndex&) const {
    return 1;
}

QVariant Demo::Project::data(const QModelIndex& index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }



    if (index.parent() == scriptParent()) {

        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(mEditors[index.row()]->objectName());
        }

        if (role == Qt::DecorationRole) {
            if (mEditors[index.row()]->hasCompileError()) {
                return QIcon::fromTheme("error");
            }
            if (mEditors[index.row()]->hasRunError()) {
                return QIcon::fromTheme("error");
            }
            if (mEditors[index.row()]->document()->isModified()) {
                return QIcon::fromTheme("document-save");
            }
            return QVariant();
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(mScripts.value(mEditors[index.row()]->objectName()));
        }


        if (role == FileNameRole) {
            return QVariant::fromValue(mScripts.value(mEditors[index.row()]->objectName()));
        }

        if (role == ScriptRole) {
            return QVariant::fromValue(mEditors[index.row()]->toPlainText());
        }

        if (role == EditorRole) {
            QWidget* widget = mEditors[index.row()];
            return QVariant::fromValue(widget);
        }

        return QVariant();
    }

    if (index.parent() == modelParent()) {
        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(mModels->modelName(index.row()));
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(mModels->fileName(index.row()));
        }

        if (role == FileNameRole) {
            return QVariant::fromValue(mModels->fileName(index.row()));
        }

        return QVariant();
    }

    if (index.parent() == textureParent()) {
        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(mImages->imageName(index.row()));
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(mImages->fileName(index.row()));
        }

        if (role == FileNameRole) {
            return QVariant::fromValue(mImages->fileName(index.row()));
        }

        return QVariant();
    }

    return QVariant();
}

bool Demo::Project::setData(const QModelIndex &index, const QVariant &value, int role) {

    if (!index.isValid()) return false;


    if (index.parent() == scriptParent()) {
        if (role == Qt::EditRole) {
            QString newname = value.toString();
            if (mEditors[index.row()]->objectName() != newname) {
                mEditors[index.row()]->setObjectName(uniqueScriptName(newname));
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileRole) {
            QString fname = value.toString();
            QString cfname = mScripts[mEditors[index.row()]->objectName()];

            QFile file(fname);
            if (file.open(QFile::ReadOnly)) {
                setEditorText(mEditors[index.row()], file.readAll());
                file.close();
            }

            if (cfname != fname) {
                mScripts[mEditors[index.row()]->objectName()] = fname;
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileNameRole) {
            QString fname = value.toString();
            if (mScripts[mEditors[index.row()]->objectName()] != fname) {
                mScripts[mEditors[index.row()]->objectName()] = fname;
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == ScriptRole) {
            setEditorText(mEditors[index.row()], value.toString());
            return true;
        }
        return false;
    }

    if (index.parent() == modelParent()) {
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
            QString cfname = mModels->fileName(index.row());
            QString name = mModels->modelName(index.row());

            QFileInfo info(fname);
            if (info.isRelative()) {
                fname = mProjectDir.absoluteFilePath(fname);
                info = QFileInfo(fname);
            }

            if (info.exists() && info.isFile() && info.isReadable()) {
                // TODO: throw error otherwise
                mModels->setModel(name, fname);
                if (cfname != fname) {
                    emit dataChanged(index, index);
                }
            }
            return true;
        }

        return false;
    }

    if (index.parent() == textureParent()) {
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
            QString cfname = mImages->fileName(index.row());
            QString name = mImages->imageName(index.row());

            QFileInfo info(fname);
            if (info.isRelative()) {
                fname = mProjectDir.absoluteFilePath(fname);
                info = QFileInfo(fname);
            }

            if (info.exists() && info.isFile() && info.isReadable()) {
                // TODO: throw error otherwise
                mImages->setImage(name, fname);
                if (cfname != fname) {
                    emit dataChanged(index, index);
                }
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


    if (parent == scriptParent()) {

        if (row >= mEditors.size())
            return false;

        beginRemoveRows(parent, row, row);
        delete mEditors[row];
        mEditors.removeAt(row);
        endRemoveRows();

    } else if (parent == modelParent()) {

        if (row >= mModels->size())
            return false;

        beginRemoveRows(parent, row, row);
        mModels->remove(row);
        endRemoveRows();

    } else if (parent == textureParent()) {

        if (row >= mImages->size())
            return false;

        beginRemoveRows(parent, row, row);
        mImages->remove(row);
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

    if (parent == scriptParent()) {

        beginInsertRows(parent, row, row);
        appendEditor(uniqueScriptName(name), "// new commands here\n", file);
        endInsertRows();

    } else if (parent == modelParent()) {
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

    } else if (parent == textureParent()) {
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

    } else {
        return false;
    }

    emit dataChanged(new_idx, new_idx);

    return true;
}


