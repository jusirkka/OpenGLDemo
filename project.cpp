#include "project.h"

#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>

Demo::Project::Project(const QDir& pdir, QObject* parent)
    :QAbstractListModel(parent),
      mProjectDir(pdir),
      mProjectIni("")
{
    if (!mProjectDir.exists() || !mProjectDir.isReadable())
        throw BadProject(QString("Project dir %1 is not readable").arg(mProjectDir.absolutePath()));

    mNames.append("Init");
    mNames.append("Draw");

    mFiles.append("");
    mFiles.append("");

    CodeEditor* editor = new CodeEditor();
    editor->insertPlainText("clearcolor vec(.1,.2,.2,1);\n");
    mEditors.append(editor);

    editor = new CodeEditor();
    editor->insertPlainText("clear color_buffer_bit;\n");
    mEditors.append(editor);

}

Demo::Project::Project(const QString& fullpath, QObject* parent)
    :QAbstractListModel(parent)
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

        CodeEditor* editor = new CodeEditor();

        QFile gfile(gfname);
        gfile.open(QFile::ReadOnly);

        editor->insertPlainText(QString(gfile.readAll()));

        gfile.close();

        mEditors.append(editor);
    }
    project.endGroup();

    if (project.status() != QSettings::NoError) throw BadProject(QString("%1 is not a valid project file").arg(fullpath));

}

void Demo::Project::saveProject() {
    QSettings project(mProjectDir.absoluteFilePath(mProjectIni), QSettings::IniFormat);

    project.beginGroup("Groups");
    for (int i = 0; i < mNames.length(); ++i) {
        qDebug() << "saveProject" << mFiles[i];
        project.setValue(mNames[i], mFiles[i]);
    }
    project.endGroup();
}

void Demo::Project::setProjectFile(const QString& fname) {
    QFileInfo info(fname);
    mProjectIni = info.fileName();
    mProjectDir = info.absoluteDir();
    qDebug() << "setProjectFile" << mProjectDir.absolutePath() << mProjectIni;
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

    if (role == Qt::ToolTipRole) {
        return QVariant::fromValue(mFiles.value(index.row()));
    }

    if (role == Qt::EditRole) {
        return QVariant::fromValue(mNames.value(index.row()));
    }

    if (role == NameRole) {
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
            emit dataChanged(index, index);
        }
        return true;
    }

    if (role == NameRole) {
        mNames[index.row()] = value.toString();
        return true;
    }

    if (role == FileRole) {
        mFiles[index.row()] = value.toString();
        return true;
    }

    if (role == GroupRole) {
        CodeEditor* editor = mEditors[index.row()];
        editor->setPlainText(value.toString());
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
    // FIXME: save edits
    delete mEditors[row];
    mEditors.removeAt(row);

    endRemoveRows();
    return true;
}


bool Demo::Project::appendRow(const QString& name, const QString& file, const QString& group) {

    beginInsertRows(QModelIndex(), mNames.length(), mNames.length());

    mNames.append(name);
    mFiles.append(file);

    CodeEditor* editor = new CodeEditor();
    editor->insertPlainText(group);
    mEditors.append(editor);

    endInsertRows();

    return true;
}

