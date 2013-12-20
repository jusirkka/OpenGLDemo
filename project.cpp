#include "project.h"
#include "gl_widget.h"
#include "runner.h"
#include "imagestore.h"
#include "modelstore.h"

#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QIcon>
#include <QRegExp>

class TreeFind {

public:
    typedef CodeEditor::EditorList::ConstIterator Iter;
    TreeFind(CodeEditor* top);
    bool find(CodeEditor* leaf);
    bool find2(CodeEditor* top, CodeEditor* leaf);

    CodeEditor* ttop;
};


TreeFind::TreeFind(CodeEditor *top)
    :ttop(top) {}

bool TreeFind::find(CodeEditor *leaf) {
    if (!ttop) return false;
    return find2(ttop, leaf);
}

bool TreeFind::find2(CodeEditor* top, CodeEditor *leaf) {
    if (top->children().contains(leaf)) return true;
    bool found = false;
    Iter ctop = top->children().constBegin();
    while (ctop !=  top->children().constEnd() && !found) {
        found = find2(*ctop, leaf);
        ++ctop;
    }
    return found;
}

using GL::ImageStore;
using GL::ModelStore;

Demo::Project::~Project() {
    foreach(EditorTuple ed, mEditors) delete ed.editor;
}

void Demo::Project::init_and_connect() {
    connect(this, SIGNAL(initChanged()), mTarget, SLOT(initChanged()));
    connect(this, SIGNAL(drawChanged()), mTarget, SLOT(drawChanged()));
    connect(mTarget, SIGNAL(evaluate(QString,QString)), this, SLOT(dispatcher(QString,QString)));
}

Demo::Project::Project(const QDir& pdir, GLWidget* target, bool autoCompileOn)
    :QAbstractItemModel(target),
      mProjectDir(pdir),
      mProjectIni(""),
      mInit(0),
      mDraw(0),
      mTarget(target),
      mAutoCompileOn(autoCompileOn)
{
    if (!mProjectDir.exists() || !mProjectDir.isReadable())
        throw BadProject(QString("Project dir %1 is not readable").arg(mProjectDir.absolutePath()));

    init_and_connect();

    appendEditor("Init", "clearcolor vec(.1,.2,.2,1);\n", "");
    appendEditor("Draw", "clear color_buffer_bit;\n", "");

    ImageStore::Clean();
    ModelStore::Clean();

    setInitGroup("Init");
    setDrawGroup("Draw");

    mModified = false;
}

Demo::Project::Project(const QString& fullpath, GLWidget* target, bool autoCompileOn)
    :QAbstractItemModel(target),
      mInit(0),
      mDraw(0),
      mTarget(target),
      mAutoCompileOn(autoCompileOn)
{
    init_and_connect();

    QFileInfo info(fullpath);
    if (!info.exists()) throw BadProject(QString("Project file %1 does not exist").arg(fullpath));
    if (!info.isFile() || !info.isReadable()) throw BadProject(QString("Project file %1 is not readable").arg(fullpath));

    mProjectDir = info.absoluteDir();
    mProjectIni = info.fileName();

    QSettings project(fullpath, QSettings::IniFormat);

    project.beginGroup("Groups");
    const QStringList gnames = project.childKeys();
    foreach (const QString &gname, gnames) {
        // check gfile
        QString gfname_orig = project.value(gname).toString();
        QString gfname = gfname_orig;
        QFileInfo ginfo(gfname);
        if (ginfo.isRelative()) {
            gfname = mProjectDir.absoluteFilePath(gfname);
            ginfo = QFileInfo(gfname);
        }
        if (!ginfo.exists()) throw BadProject(QString("Group file %1 does not exist").arg(gfname_orig));
        if (!ginfo.isFile() || !ginfo.isReadable()) throw BadProject(QString("Group file %1 is not readable").arg(gfname_orig));

        QFile gfile(gfname);
        gfile.open(QFile::ReadOnly);

        appendEditor(uniqueGroupName(gname), QString(gfile.readAll()), gfname_orig);

        gfile.close();

    }
    project.endGroup();

    project.beginGroup("Models");
    NameList models;
    const QStringList onames = project.childKeys();
    foreach (const QString &oname, onames) {
        QString fname_orig = project.value(oname).toString();
        QString fname = fname_orig;
        QFileInfo info(fname);
        if (info.isRelative()) {
            fname = mProjectDir.absoluteFilePath(fname);
            info = QFileInfo(fname);
        }
        if (!info.exists() || !info.isFile() || !info.isReadable()) {
            fname_orig = "";
        }

        NameTuple t;
        t.name = oname;
        t.filename = fname_orig;
        models.append(t);
    }
    project.endGroup();

    project.beginGroup("Textures");
    NameList textures;
    const QStringList inames = project.childKeys();
    foreach (const QString &iname, inames) {
        QString fname_orig = project.value(iname).toString();
        QString fname = fname_orig;
        QFileInfo info(fname);
        if (info.isRelative()) {
            fname = mProjectDir.absoluteFilePath(fname);
            info = QFileInfo(fname);
        }
        if (!info.exists() || !info.isFile() || !info.isReadable()) {
            fname_orig = "";
        }

        NameTuple t;
        t.name = iname;
        t.filename = fname_orig;
        textures.append(t);
    }
    project.endGroup();

    if (project.status() != QSettings::NoError) throw BadProject(QString("%1 is not a valid project file").arg(fullpath));

    ImageStore::Clean();
    foreach(NameTuple t, textures) {
        ImageStore::SetImage(uniqueImageName(t.name), t.filename);
    }

    ModelStore::Clean();
    foreach(NameTuple t, models) {
        ModelStore::SetModel(uniqueModelName(t.name), t.filename);
    }

    project.beginGroup("Roles");
    QString initKey = project.value("Init", "unknown_init").toString();
    setInitGroup(initKey);
    QString drawKey = project.value("Draw", "unknown_draw").toString();
    setDrawGroup(drawKey);
    project.endGroup();

    mModified = false;
}

void Demo::Project::saveProject() {
    QSettings project(mProjectDir.absoluteFilePath(mProjectIni), QSettings::IniFormat);
    project.clear();

    project.beginGroup("Groups");
    for (int i = 0; i < mEditors.length(); ++i) {
        project.setValue(mEditors[i].editor->objectName(), mEditors[i].filename);
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
    for (int i = 0; i < ImageStore::Size(); ++i) {
        project.setValue(ImageStore::ImageName(i), ImageStore::FileName(i));
    }
    project.endGroup();

    project.beginGroup("Models");
    for (int i = 0; i < ModelStore::Size(); ++i) {
        project.setValue(ModelStore::ModelName(i), ModelStore::FileName(i));
    }
    project.endGroup();

    mModified = false;
}

void Demo::Project::setProjectFile(const QString& fname) {
    QFileInfo info(fname);
    mProjectIni = info.fileName();
    mProjectDir = info.absoluteDir();
    qDebug() << "setProjectFile" << mProjectDir.absolutePath() << mProjectIni;
}

void Demo::Project::runnerReady() {
    if (sender() == mInit) {
        emit initChanged();
    } else if (sender() == mDraw) {
        emit drawChanged();
    } else {
        CodeEditor* ed = qobject_cast<CodeEditor*>(sender());
        if (TreeFind(mInit).find(ed)) {
            emit initChanged();
        }
        if (TreeFind(mDraw).find(ed)){
            emit drawChanged();
        }
    }
}

void Demo::Project::groupModified(bool) {
    QModelIndex lower = index(0, groupParent());
    QModelIndex upper = index(mEditors.length() - 1, groupParent());
    emit dataChanged(lower, upper);
}

QString Demo::Project::initGroup() const {
    if (mInit) return mInit->objectName();
    return "";
}

QString Demo::Project::drawGroup() const {
    if (mDraw) return mDraw->objectName();
    return "";
}

void Demo::Project::setInitGroup(QString grp) {
    if (mInit) {
        disconnect(mTarget, SIGNAL(init()), mInit, SLOT(evaluate()));
    }
    mInit = 0;
    foreach(EditorTuple ed, mEditors) {
        if (ed.editor->objectName() == grp) {
            mInit = ed.editor;
            break;
        }
    }
    if (mInit) {
        connect(mTarget, SIGNAL(init()), mInit, SLOT(evaluate()));
    }
    mModified = true;
    emit initChanged();
}

void Demo::Project::setDrawGroup(QString grp) {
    if (mDraw) {
        disconnect(mTarget, SIGNAL(draw()), mDraw, SLOT(evaluate()));
    }
    mDraw = 0;
    foreach(EditorTuple ed, mEditors) {
        if (ed.editor->objectName() == grp) {
            mDraw = ed.editor;
            break;
        }
    }
    if (mDraw) {
        connect(mTarget, SIGNAL(draw()), mDraw, SLOT(evaluate()));
    }
    mModified = true;
    emit drawChanged();
}


void Demo::Project::dispatcher(const QString& curr, const QString& other) {
    qDebug() << "dispatch" << curr << other;
    CodeEditor* curr_ed = 0;
    CodeEditor* other_ed = 0;
    foreach (EditorTuple ed, mEditors) {
        if (ed.editor->objectName() == curr) {
            curr_ed = ed.editor;
        } else if (ed.editor->objectName() == other) {
            other_ed = ed.editor;
        }
    }
    if (curr_ed && other_ed) {
        curr_ed->appendChild(other_ed);
        other_ed->evaluate();
    }
}


static void setEditorText(CodeEditor* editor, const QString& group) {
    editor->clear();
    editor->insertPlainText(group);
    editor->document()->setModified(false);

    QTextCursor c = editor->textCursor();
    c.setPosition(0);
    editor->setTextCursor(c);
}

CodeEditor* Demo::Project::appendEditor(const QString& name, const QString& group, const QString& file) {
    CodeEditor* editor = new CodeEditor(this);
    editor->setObjectName(name);
    setEditorText(editor, group);
    EditorTuple t;
    t.editor = editor;
    t.filename = file;
    mEditors.append(t);
    editor->parse();
    return editor;
}

void Demo::Project::toggleAutoCompile(bool on) {
    mAutoCompileOn = on;
    foreach (EditorTuple t, mEditors) {
        CodeEditor* ed = t.editor;
        ed->toggleAutoParse(on);
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

QString Demo::Project::uniqueGroupName(const QString& orig) const {
    QStringList names;
    foreach(EditorTuple ed, mEditors) {
        names.append(ed.editor->objectName());
    }
    return uniqueName(orig, names);
}

QString Demo::Project::uniqueModelName(const QString& orig) const {
    QStringList names;
    for (int i = 0; i < ModelStore::Size(); ++i) {
        names.append(ModelStore::ModelName(i));
    }

    return uniqueName(orig, names);
}


QString Demo::Project::uniqueImageName(const QString& orig) const {
    QStringList names;
    for (int i = 0; i < ImageStore::Size(); ++i) {
        names.append(ImageStore::ImageName(i));
    }

    return uniqueName(orig, names);
}

QModelIndex Demo::Project::groupParent() const{
    return index(GroupRow, QModelIndex());
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
    if (parent.internalId() == GroupRow)
        return createIndex(row, column, NumRows + row);
    if (parent.internalId() == ModelRow)
        return createIndex(row, column, NumRows + mEditors.size() + row);
    if (parent.internalId() == TextureRow)
        return createIndex(row, column, NumRows + mEditors.size() + ModelStore::Size() + row);

    return QModelIndex();
}


QModelIndex Demo::Project::index(int row, const QModelIndex &parent) const {
    return index(row, 0, parent);
}

QModelIndex Demo::Project::parent(const QModelIndex &index) const {

    if (!index.isValid()) return QModelIndex();

    int id = index.internalId();

    if (id < NumRows) return QModelIndex();
    if (id < NumRows + mEditors.size()) return createIndex(GroupRow, 0, GroupRow);
    if (id < NumRows + mEditors.size() + ModelStore::Size()) return createIndex(ModelRow, 0, ModelRow);
    if (id < NumRows + mEditors.size() + ModelStore::Size() + ImageStore::Size()) return createIndex(TextureRow, 0, TextureRow);

    return QModelIndex();
}

QVariant Demo::Project::headerData(int, Qt::Orientation, int) const {
    return QVariant();
}

int Demo::Project::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) return NumRows;
    int id = parent.internalId();
    if (id == GroupRow) return mEditors.size();
    if (id == ModelRow) return ModelStore::Size();
    if (id == TextureRow) return ImageStore::Size();

    return 0;
}

int Demo::Project::columnCount(const QModelIndex&) const {
    return 1;
}

QVariant Demo::Project::data(const QModelIndex& index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }

    if (index.parent() == groupParent()) {
        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(mEditors[index.row()].editor->objectName());
        }

        if (role == Qt::DecorationRole) {
            if (mEditors[index.row()].editor->document()->isModified()) {
                return QIcon::fromTheme("filesave");
            }
            return QVariant();
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(mEditors[index.row()].filename);
        }


        if (role == FileNameRole) {
            return QVariant::fromValue(mEditors[index.row()].filename);
        }

        if (role == GroupRole) {
            return QVariant::fromValue(mEditors[index.row()].editor->toPlainText());
        }

        if (role == EditorRole) {
            QWidget* widget = mEditors[index.row()].editor;
            return QVariant::fromValue(widget);
        }

        return QVariant();
    }

    if (index.parent() == modelParent()) {
        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(ModelStore::ModelName(index.row()));
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(ModelStore::FileName(index.row()));
        }

        if (role == FileNameRole) {
            return QVariant::fromValue(ModelStore::FileName(index.row()));
        }

        return QVariant();
    }

    if (index.parent() == textureParent()) {
        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(ImageStore::ImageName(index.row()));
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(ImageStore::FileName(index.row()));
        }

        if (role == FileNameRole) {
            return QVariant::fromValue(ImageStore::FileName(index.row()));
        }

        return QVariant();
    }

    return QVariant();
}

bool Demo::Project::setData(const QModelIndex &index, const QVariant &value, int role) {

    if (!index.isValid()) return false;


    if (index.parent() == groupParent()) {
        if (role == Qt::EditRole) {
            QString newname = value.toString();
            if (mEditors[index.row()].editor->objectName() != newname) {
                mEditors[index.row()].editor->setObjectName(uniqueGroupName(newname));
                mModified = true;
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileRole) {
            QString fname = value.toString();
            QString cfname = mEditors[index.row()].filename;

            QFile file(fname);
            if (file.open(QFile::ReadOnly)) {
                setEditorText(mEditors[index.row()].editor, file.readAll());
                file.close();
            }

            if (cfname != fname) {
                mEditors[index.row()].filename = fname;
                mModified = true;
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileNameRole) {
            QString fname = value.toString();
            if (mEditors[index.row()].filename != fname) {
                mEditors[index.row()].filename = fname;
                mModified = true;
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == GroupRole) {
            setEditorText(mEditors[index.row()].editor, value.toString());
            return true;
        }
        return false;
    }

    if (index.parent() == modelParent()) {
        if (role == Qt::EditRole) {
            QString newname = value.toString();
            QString curr = ModelStore::ModelName(index.row());
            if (curr != newname) {
                ModelStore::Rename(curr, uniqueModelName(newname));
                mModified = true;
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileRole) {
            QString fname = value.toString();
            QString cfname = ModelStore::FileName(index.row());
            QString name = ModelStore::ModelName(index.row());

            QFileInfo info(fname);
            if (info.isRelative()) {
                fname = mProjectDir.absoluteFilePath(fname);
                info = QFileInfo(fname);
            }

            if (info.exists() && info.isFile() && info.isReadable()) {
                // TODO: throw error otherwise
                ModelStore::SetModel(name, fname);
                if (cfname != fname) {
                    mModified = true;
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
            QString curr = ImageStore::ImageName(index.row());
            if (curr != newname) {
                ImageStore::Rename(curr, uniqueImageName(newname));
                mModified = true;
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileRole) {
            QString fname = value.toString();
            QString cfname = ImageStore::FileName(index.row());
            QString name = ImageStore::ImageName(index.row());

            QFileInfo info(fname);
            if (info.isRelative()) {
                fname = mProjectDir.absoluteFilePath(fname);
                info = QFileInfo(fname);
            }

            if (info.exists() && info.isFile() && info.isReadable()) {
                // TODO: throw error otherwise
                ImageStore::SetImage(name, fname);
                if (cfname != fname) {
                    mModified = true;
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


    if (parent == groupParent()) {

        if (row >= mEditors.size())
            return false;

        beginRemoveRows(parent, row, row);
        delete mEditors[row].editor;
        mEditors.removeAt(row);
        endRemoveRows();

    } else if (parent == modelParent()) {

        if (row >= ModelStore::Size())
            return false;

        beginRemoveRows(parent, row, row);
        ModelStore::Remove(row);
        endRemoveRows();

    } else if (parent == textureParent()) {

        if (row >= ImageStore::Size())
            return false;

        beginRemoveRows(parent, row, row);
        ImageStore::Remove(row);
        endRemoveRows();
    }

    mModified = true;
    emit dataChanged(index(row, 0, parent), index(row, 0, parent));
    return true;
}


bool Demo::Project::appendRow(const QString& name, const QString& file, const QModelIndex &parent) {

    if (!parent.isValid())
        return false;

    int row = rowCount(parent);
    QModelIndex new_idx = index(row, parent);

    if (parent == groupParent()) {

        beginInsertRows(parent, row, row);
        appendEditor(uniqueGroupName(name), "// new commands here\n", file);
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
            ModelStore::SetModel(uniqueModelName(name), fname);
        } else {
            ModelStore::SetModel(uniqueModelName(name));
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
            ImageStore::SetImage(uniqueImageName(name), fname);
        } else {
            ImageStore::SetImage(uniqueImageName(name));
        }
        endInsertRows();

    } else {
        return false;
    }

    mModified = true;
    emit dataChanged(new_idx, new_idx);

    return true;
}


