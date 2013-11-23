#include "project.h"
#include "gl_widget.h"
#include "runner.h"
#include "imagestore.h"

#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QIcon>

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

Demo::Project::~Project() {
    foreach(EditorTuple ed, mEditors) delete ed.editor;
}

void Demo::Project::init_and_connect() {
    connect(this, SIGNAL(initChanged()), mTarget, SLOT(initChanged()));
    connect(this, SIGNAL(drawChanged()), mTarget, SLOT(drawChanged()));
    connect(mTarget, SIGNAL(evaluate(QString,QString)), this, SLOT(dispatcher(QString,QString)));
}

Demo::Project::Project(const QDir& pdir, GLWidget* target)
    :QAbstractItemModel(target),
      mProjectDir(pdir),
      mProjectIni(""),
      mTarget(target),
      mModified(false)
{
    if (!mProjectDir.exists() || !mProjectDir.isReadable())
        throw BadProject(QString("Project dir %1 is not readable").arg(mProjectDir.absolutePath()));

    init_and_connect();

    mInit = appendEditor("Init", "clearcolor vec(.1,.2,.2,1);\n", "");
    mDraw = appendEditor("Draw", "clear color_buffer_bit;\n", "");

    mTarget->imageStore()->clean();
}

Demo::Project::Project(const QString& fullpath, GLWidget* target)
    :QAbstractItemModel(target),
      mInit(0),
      mDraw(0),
      mTarget(target),
      mModified(false)
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

        appendEditor(gname, QString(gfile.readAll()), gfname_orig);

        gfile.close();

    }
    project.endGroup();

    project.beginGroup("Roles");
    QString initKey = project.value("Init", "unknown_init").toString();
    setInitGroup(initKey);
    QString drawKey = project.value("Draw", "unknown_draw").toString();
    setDrawGroup(drawKey);
    project.endGroup();

    project.beginGroup("Models");
    const QStringList onames = project.childKeys();
    foreach (const QString &oname, onames) {
        QString fname_orig = project.value(oname).toString();
        QString fname = fname_orig;
        QFileInfo info(fname);
        if (info.isRelative()) {
            fname = mProjectDir.absoluteFilePath(fname);
            info = QFileInfo(fname);
        }
        if (!info.exists()) throw BadProject(QString("Object file %1 does not exist").arg(fname_orig));
        if (!info.isFile() || !info.isReadable()) throw BadProject(QString("Object file %1 is not readable").arg(fname_orig));

        NameTuple t;
        t.name = oname;
        t.filename = fname_orig;
        mModels.append(t);
        // TODO: create blob
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

    mTarget->imageStore()->clean();
    foreach(NameTuple t, textures) {
        mTarget->imageStore()->setImage(t.name, t.filename);
    }
}

void Demo::Project::saveProject() {
    QSettings project(mProjectDir.absoluteFilePath(mProjectIni), QSettings::IniFormat);

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
        connect(mTarget, SIGNAL(init()), mInit->runner(), SLOT(evaluate()));
        emit initChanged();
    } else if (sender() == mDraw) {
        connect(mTarget, SIGNAL(draw()), mDraw->runner(), SLOT(evaluate()));
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
    if (mInit && mInit->runner()) {
        disconnect(mTarget, SIGNAL(init()), mInit->runner(), SLOT(evaluate()));
    }
    mInit = 0;
    foreach(EditorTuple ed, mEditors) {
        if (ed.editor->objectName() == grp) {
            mInit = ed.editor;
            break;
        }
    }
    if (mInit && mInit->runner()) {
        connect(mTarget, SIGNAL(init()), mInit->runner(), SLOT(evaluate()));
    }
    mModified = true;
    emit initChanged();
}

void Demo::Project::setDrawGroup(QString grp) {
    if (mDraw && mDraw->runner()) {
        disconnect(mTarget, SIGNAL(draw()), mDraw->runner(), SLOT(evaluate()));
    }
    mDraw = 0;
    foreach(EditorTuple ed, mEditors) {
        if (ed.editor->objectName() == grp) {
            mDraw = ed.editor;
            break;
        }
    }
    if (mDraw && mDraw->runner()) {
        connect(mTarget, SIGNAL(draw()), mDraw->runner(), SLOT(evaluate()));
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
        if (other_ed->runner()) other_ed->runner()->evaluate();
    }
}


static void setEditorText(CodeEditor* editor, const QString& group) {
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
    return editor;
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
        return createIndex(row, column, NumRows + mEditors.size() + mModels.size() + row);

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
    if (id < NumRows + mEditors.size() + mModels.size()) return createIndex(ModelRow, 0, ModelRow);
    if (id < NumRows + mEditors.size() + mModels.size() + mTarget->imageStore()->size()) return createIndex(TextureRow, 0, TextureRow);

    return QModelIndex();
}

QVariant Demo::Project::headerData(int, Qt::Orientation, int) const {
    return QVariant();
}

int Demo::Project::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) return NumRows;
    int id = parent.internalId();
    if (id == GroupRow) return mEditors.size();
    if (id == ModelRow) return mModels.size();
    if (id == TextureRow) return mTarget->imageStore()->size();

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
            return QVariant::fromValue(mModels[index.row()].name);
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(mModels[index.row()].filename);
        }

        if (role == FileNameRole) {
            return QVariant::fromValue(mModels[index.row()].filename);
        }

        return QVariant();
    }

    if (index.parent() == textureParent()) {
        if (role == Qt::DisplayRole) {
            return QVariant::fromValue(mTarget->imageStore()->imageName(index.row()));
        }

        if (role == Qt::ToolTipRole) {
            return QVariant::fromValue(mTarget->imageStore()->fileName(index.row()));
        }

        if (role == FileNameRole) {
            return QVariant::fromValue(mTarget->imageStore()->fileName(index.row()));
        }

        return QVariant();
    }

    return QVariant();
}

bool Demo::Project::setData(const QModelIndex &index, const QVariant &value, int role) {

    if (!index.isValid()) return false;


    if (index.parent() == groupParent()) {
        if (role == Qt::EditRole) {
            if (mEditors[index.row()].editor->objectName() != value.toString()) {
                mEditors[index.row()].editor->setObjectName(value.toString());
                mModified = true;
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileRole) {
            QString fname = value.toString();
            if (mEditors[index.row()].filename != fname) {
                mEditors[index.row()].filename = fname;
                mModified = true;

                QFile file(fname);
                if (file.open(QFile::ReadOnly)) {
                    setEditorText(mEditors[index.row()].editor, file.readAll());
                    file.close();
                }

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
            if (mModels[index.row()].name != value.toString()) {
                mModels[index.row()].name = value.toString();
                mModified = true;
                // TODO: update symbols & blob map
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileRole) {
            QString fname = value.toString();
            if (mModels[index.row()].filename != fname) {
                mModels[index.row()].filename = fname;
                mModified = true;

                QFile file(fname);
                if (file.open(QFile::ReadOnly)) {
                    // TODO: update blob
                    file.close();
                }

                emit dataChanged(index, index);
            }
            return true;
        }


        return false;
    }

    if (index.parent() == textureParent()) {
        if (role == Qt::EditRole) {
            QString curr = mTarget->imageStore()->imageName(index.row());
            if (curr != value.toString()) {
                mTarget->imageStore()->rename(curr, value.toString());
                mModified = true;
                emit dataChanged(index, index);
            }
            return true;
        }

        if (role == FileRole) {
            QString fname = value.toString();
            QString cfname = mTarget->imageStore()->fileName(index.row());
            QString name = mTarget->imageStore()->imageName(index.row());

            QFileInfo info(fname);
            if (info.isRelative()) {
                fname = mProjectDir.absoluteFilePath(fname);
                info = QFileInfo(fname);
            }

            if (info.exists() && info.isFile() && info.isReadable()) {
                // TODO: throw error otherwise
                mTarget->imageStore()->setImage(name, fname);
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

        if (row >= mModels.size())
            return false;

        beginRemoveRows(parent, row, row);
        mModels.removeAt(row);
        endRemoveRows();
        // TODO: remove blob

    } else if (parent == textureParent()) {

        if (row >= mTarget->imageStore()->size())
            return false;

        beginRemoveRows(parent, row, row);
        mTarget->imageStore()->remove(row);
        endRemoveRows();
        // TODO: remove texture blob
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
        appendEditor(name, "// new commands here\n", file);
        endInsertRows();

    } else if (parent == modelParent()) {
        beginInsertRows(parent, row, row);
        NameTuple t;
        t.name = name;
        t.filename = file;
        mModels.append(t);
        endInsertRows();

        // TODO add blob

    } else if (parent == textureParent()) {
        beginInsertRows(parent, row, row);
        QString fname = file;
        QFileInfo info(fname);
        if (info.isRelative()) {
            fname = mProjectDir.absoluteFilePath(fname);
            info = QFileInfo(fname);
        }

        if (info.exists() && info.isFile() && info.isReadable()) {
            mTarget->imageStore()->setImage(name, fname);
        } else {
            mTarget->imageStore()->setImage(name);
        }
        endInsertRows();

        // TODO add texture blob
    } else {
        return false;
    }

    mModified = true;
    emit dataChanged(new_idx, new_idx);

    return true;
}


