#include "project.h"
#include "gl_widget.h"
#include "runner.h"

#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QIcon>

Demo::Project::~Project() {
    foreach(CodeEditor* ed, mEditors) delete ed;
}

Demo::Project::Project(const QDir& pdir, GLWidget* target)
    :QAbstractListModel(target),
      mProjectDir(pdir),
      mProjectIni(""),
      mTarget(target),
      mModified(false)
{
    if (!mProjectDir.exists() || !mProjectDir.isReadable())
        throw BadProject(QString("Project dir %1 is not readable").arg(mProjectDir.absolutePath()));

    mNames.append("Init");
    mNames.append("Draw");

    mFiles.append("");
    mFiles.append("");

    CodeEditor* editor = new CodeEditor(this);
    mInit = editor;
    qDebug() << mInit;
    editor->insertPlainText("clearcolor vec(.1,.2,.2,1);\n");
    editor->document()->setModified(false);
    mEditors.append(editor);

    editor = new CodeEditor(this);
    mDraw = editor;
    qDebug() << mDraw;
    editor->insertPlainText("clear color_buffer_bit;\n");
    editor->document()->setModified(false);
    mEditors.append(editor);

    connect(this, SIGNAL(initChanged()), mTarget, SLOT(initChanged()));
    connect(this, SIGNAL(drawChanged()), mTarget, SLOT(drawChanged()));
}


Demo::Project::Project(const QString& fullpath, GLWidget* target)
    :QAbstractListModel(target),
      mInit(0),
      mDraw(0),
      mTarget(target),
      mModified(false)
{

    QFileInfo info(fullpath);
    if (!info.exists()) throw BadProject(QString("Project file %1 does not exist").arg(fullpath));
    if (!info.isFile() || !info.isReadable()) throw BadProject(QString("Project file %1 is not readable").arg(fullpath));

    mProjectDir = info.absoluteDir();
    mProjectIni = info.fileName();

    QSettings project(fullpath, QSettings::IniFormat);

    project.beginGroup("Groups");
    const QStringList gnames = project.childKeys();
    foreach (const QString &gname, gnames) {
        mNames.append(gname);
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
        // .. and append
        mFiles.append(gfname_orig);

        CodeEditor* editor = new CodeEditor(this);

        QFile gfile(gfname);
        gfile.open(QFile::ReadOnly);

        editor->insertPlainText(QString(gfile.readAll()));
        editor->document()->setModified(false);

        gfile.close();

        mEditors.append(editor);
    }
    project.endGroup();

    project.beginGroup("Roles");
    QString initKey = project.value("Init", "unk_init").toString();
    if (mNames.contains(initKey)) {
        mInit = mEditors[mNames.indexOf(initKey)];
    }
    QString drawKey = project.value("Draw", "unk_draw").toString();
    if (mNames.contains(drawKey)) {
        mDraw = mEditors[mNames.indexOf(drawKey)];
    }
    project.endGroup();

    if (project.status() != QSettings::NoError) throw BadProject(QString("%1 is not a valid project file").arg(fullpath));

    connect(this, SIGNAL(initChanged()), mTarget, SLOT(initChanged()));
    connect(this, SIGNAL(drawChanged()), mTarget, SLOT(drawChanged()));
}

void Demo::Project::saveProject() {
    QSettings project(mProjectDir.absoluteFilePath(mProjectIni), QSettings::IniFormat);

    project.beginGroup("Groups");
    for (int i = 0; i < mNames.length(); ++i) {
        project.setValue(mNames[i], mFiles[i]);
    }
    project.endGroup();

    project.beginGroup("Roles");
    project.setValue("Init", "None");
    if (mEditors.contains(mInit)) {
        project.setValue("Init", mNames[mEditors.indexOf(mInit)]);
    }
    project.setValue("Draw", "None");
    if (mEditors.contains(mDraw)) {
        project.setValue("Draw", mNames[mEditors.indexOf(mDraw)]);
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
    }
}

void Demo::Project::groupModified(bool) {
    QModelIndex lower = index(0);
    QModelIndex upper = index(mNames.length() - 1);
    emit dataChanged(lower, upper);
}

QString Demo::Project::initGroup() const {
    int idx = mEditors.indexOf(mInit);
    if (idx == -1) return "";
    return mNames[idx];
}

QString Demo::Project::drawGroup() const {
    int idx = mEditors.indexOf(mDraw);
    if (idx == -1) return "";
    return mNames[idx];
}

void Demo::Project::setInitGroup(QString grp) {
    if (mInit && mInit->runner()) {
        disconnect(mTarget, SIGNAL(init()), mInit->runner(), SLOT(evaluate()));
    }
    int idx = mNames.indexOf(grp);
    if (idx == -1) {
        mInit = 0;
    } else {
        mInit = mEditors[idx];
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
    int idx = mNames.indexOf(grp);
    if (idx == -1) {
        mDraw = 0;
    } else {
        mDraw = mEditors[idx];
    }
    if (mDraw && mDraw->runner()) {
        connect(mTarget, SIGNAL(draw()), mDraw->runner(), SLOT(evaluate()));
    }
    mModified = true;
    emit drawChanged();
}

QVariant Demo::Project::headerData(int, Qt::Orientation, int) const {
    return QVariant();
}

int Demo::Project::rowCount(const QModelIndex&) const
{
    return mNames.length();
}

QVariant Demo::Project::data(const QModelIndex &index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return QVariant::fromValue(mNames.value(index.row()));
    }

    if (role == Qt::DecorationRole) {
        if (mEditors[index.row()]->document()->isModified()) {
            return QIcon::fromTheme("filesave");
        }
        return QVariant();
    }

    if (role == Qt::ToolTipRole) {
        return QVariant::fromValue(mFiles.value(index.row()));
    }

    if (role == Qt::EditRole) {
        return QVariant::fromValue(mNames.value(index.row()));
    }

    if (role == FileRole) {
        return QVariant::fromValue(mFiles.value(index.row()));
    }

    if (role == GroupRole) {
        return QVariant::fromValue(mEditors.value(index.row())->toPlainText());
    }

    if (role == EditorRole) {
        QWidget* widget = mEditors.value(index.row());
        return QVariant::fromValue(widget);
    }

    return QVariant();
}

bool Demo::Project::setData(const QModelIndex &index, const QVariant &value, int role) {

    if (!index.isValid()) return false;

    if (role == Qt::EditRole) {
        if (mNames[index.row()] != value.toString()) {
            mNames[index.row()] = value.toString();
            mModified = true;
            emit dataChanged(index, index);
        }
        return true;
    }

    if (role == FileRole) {
        if (mFiles[index.row()] != value.toString()) {
            mFiles[index.row()] = value.toString();
            mModified = true;
            emit dataChanged(index, index);
        }
        return true;
    }

    if (role == GroupRole) {
        CodeEditor* editor = mEditors[index.row()];
        editor->setPlainText(value.toString());
        editor->document()->setModified(false);
        return true;
    }

    if (role == EditorRole) {
        CodeEditor* editor = qobject_cast<CodeEditor*>(value.value<QWidget*>());
        mEditors[index.row()] = editor;
        return true;
    }

    return false;
}


Qt::ItemFlags Demo::Project::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}


bool Demo::Project::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;

    if (count != 1)
        return false;

    if (row >= mNames.size() || row < 0)
        return false;


    beginRemoveRows(parent, row, row);

    mNames.removeAt(row);
    mFiles.removeAt(row);
    delete mEditors[row];
    mEditors.removeAt(row);

    endRemoveRows();

    mModified = true;
    emit dataChanged(index(row), index(row));
    return true;
}


bool Demo::Project::appendRow(const QString& name, const QString& file, const QString& group) {

    int row = mNames.length();

    beginInsertRows(QModelIndex(), row, row);

    mNames.append(name);
    mFiles.append(file);

    CodeEditor* editor = new CodeEditor(this);
    editor->insertPlainText(group);
    editor->document()->setModified(false);
    mEditors.append(editor);

    endInsertRows();

    mModified = true;
    emit dataChanged(index(row), index(row));
    return true;
}

