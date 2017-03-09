// ------------------------------------------------------------------------------
//   Copyright (C) 2007 by Jukka Sirkka
//   jukka.sirkka@iki.fi
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the
//   Free Software Foundation, Inc.,
//   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ------------------------------------------------------------------------------

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "newdialog.h"
#include "gl_widget.h"
#include "project.h"
#include "codeeditor.h"
#include "fpscontrol.h"
#include "scriptselector.h"
#include "constant.h"
#include "scope.h"

#include <QtDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QCloseEvent>
#include <QSettings>
#include <QUndoStack>
#include <QApplication>
#include <QImageReader>

using namespace Demo;

MainWindow::MainWindow(const QString& project):
    QMainWindow(),
    mLastDir(QDir::home()),
    mUI(new Ui::MainWindow),
    mProject(0),
    mProjectModified(false),
    mNumEdits(0)
{


    mUI->setupUi(this);

    FPSControl* fps = new FPSControl();
    connect(fps, SIGNAL(valueChanged(int)), this, SLOT(fps_changed(int)));
    mUI->demoBar->addWidget(fps);

    mScripts = new ScriptSelector();
    connect(mScripts, SIGNAL(initScriptChanged(const QString&)), this, SLOT(initScript_changed(const QString&)));
    connect(mScripts, SIGNAL(drawScriptChanged(const QString&)), this, SLOT(drawScript_changed(const QString&)));
    mUI->demoBar->addWidget(mScripts);

    mGLWidget = new GLWidget(mUI->graphicsDockContents);

    // Initialize globals
    mGlobals = new Scope(mGLWidget, this);

    mUI->graphicsDockLayout->addWidget(mGLWidget);

    mUI->centralwidget->hide();

    mUI->projectItems->addAction(mUI->actionOpen);
    mUI->projectItems->addAction(mUI->actionSave);
    mUI->projectItems->addAction(mUI->actionSaveAs);
    mUI->projectItems->addAction(mUI->actionRename);
    mUI->projectItems->addAction(mUI->actionEdit);
    mUI->projectItems->addAction(mUI->actionCompile);
    mUI->projectItems->addAction(mUI->actionDelete);
    mUI->projectItems->addAction(mUI->actionReload);


    readSettings();

    openProject(project);
}


void Demo::MainWindow::on_actionNewProject_triggered() {
    QString dirName = QFileDialog::getExistingDirectory(
        this,
        "Select project directory",
        mLastDir.absolutePath()
    );
    if (dirName.isEmpty()) return;
    mLastDir = QDir(dirName);

    // suggest to save the current project before deleting
    if(!maybeSaveProject()) return;

    openProject(dirName);
}

void Demo::MainWindow::on_actionOpenProject_triggered() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open project",
        mLastDir.absolutePath(),
        "INI files (*.ini)"
    );
    if (fileName.isEmpty()) return;
    QFileInfo info(fileName);
    mLastDir = info.absoluteDir();

    // suggest to save the current project before deleting
    if(!maybeSaveProject()) return;

    openProject(fileName);
}

void Demo::MainWindow::on_actionSaveAll_triggered() {

    QString fname = mProject->directory().absoluteFilePath(mProject->projectFile());
    QFileInfo info(fname);


    if (!info.isFile() || !info.isWritable()) {
        fname = QFileDialog::getSaveFileName(
            this,
            "Select project file to save to",
            mProject->directory().absolutePath(),
            "INI files (*.ini)"
        );
        if (fname.isEmpty()) return;
        // qDebug() << "on_actionSaveAll_triggered" << fname;
        mProject->setProjectFile(fname);
        setWindowTitle(QString("%1: %2 [*]").arg(QApplication::applicationName(), fname));
    }

    QModelIndex tmp = mSelectedIndex;
    for (int i = 0; i < mProject->rowCount(mProject->itemParent(Project::ScriptItems)); ++i) {
        mSelectedIndex = mProject->index(i, Project::ScriptItems);
        QWidget* widget = mProject->data(mSelectedIndex, Project::EditorRole).value<QWidget*>();
        CodeEditor* editor = qobject_cast<CodeEditor*>(widget);
        if (editor->document()->isModified()) {
            on_actionSave_triggered();
        } else if (mProject->data(mSelectedIndex, Project::FileNameRole) == "") {
            on_actionSaveAs_triggered();
        }
    }
    mSelectedIndex = tmp;

    mProject->saveProject();

    mProjectModified = false;
    mNumEdits = 0;
    setProjectModified();

}


void Demo::MainWindow::on_actionNew_triggered() {
    NewDialog dlg(mProject);
    if (dlg.exec() == QDialog::Accepted) {
        mProject->appendRow(dlg.name(), "", dlg.listParent());
        mScripts->setup(mProject);
    }
}

void Demo::MainWindow::on_actionOpen_triggered() {
    QString title;
    QString filter;
    if (mSelectedIndex.parent() == mProject->itemParent(Project::ScriptItems)) {
        title = "Open a file and bind to a script";
        filter = "OpenGL script files ( *.ogl)";
    } else if (mSelectedIndex.parent() == mProject->itemParent(Project::ModelItems)) {
        title = "Open a model file to create vertex data";
        filter = "Model files ( *.obj)";
    } else if (mSelectedIndex.parent() == mProject->itemParent(Project::TextureItems)) {
        title = "Open an image to create texture data";
        filter = "Image files (";
        foreach(QByteArray b, QImageReader::supportedImageFormats()) {
            filter += QString(" *.%1").arg(QString(b));
        }
        filter += ")";
    } else if (mSelectedIndex.parent() == mProject->itemParent(Project::ShaderItems)) {
        title = "Open a shader source file to create a shader";
        filter = "GLSL files ( *.glsl *.vert *.frag)";
    }

    QString fileName = QFileDialog::getOpenFileName(
                this,
                title,
                mProject->directory().absolutePath(),
                filter
                );
    if (fileName.isEmpty()) return;

    QFileInfo info(fileName);
    mLastDir = info.absoluteDir();

    // suggest to save the current script before deleting
    if(!maybeSave()) return;

    mProject->setData(mSelectedIndex, QVariant::fromValue(fileName), Project::FileRole);
    mProjectModified = true;
    setProjectModified();
}


void Demo::MainWindow::on_actionSave_triggered() {

    QString fname = mProject->data(mSelectedIndex, Project::FileNameRole).toString();

    QFileInfo info(fname);
    if (info.isRelative()) {
        fname = mProject->directory().absoluteFilePath(fname);
        info = QFileInfo(fname);
    }

    if (info.isFile() && info.isWritable()) {
        saveScript(fname);
    } else {
        on_actionSaveAs_triggered();
    }
}

void Demo::MainWindow::on_actionSaveAs_triggered() {
    QString name = mProject->data(mSelectedIndex).toString();

    QString fname = QFileDialog::getSaveFileName(
        this,
        QString("Select file to save the script \"%1\" to").arg(name),
        mProject->directory().absolutePath(),
        "OpenGL command files (*.ogl)"
    );
    if (fname.isEmpty()) return;
    saveScript(fname);
    mProject->setData(mSelectedIndex, QVariant::fromValue(fname), Project::FileNameRole);
    mProjectModified = true;
    setProjectModified();
}

void Demo::MainWindow::on_actionDelete_triggered() {
    // suggest to save the current script before deleting
    if(!maybeSave()) return;

    mProject->removeRows(mSelectedIndex.row(), 1, mSelectedIndex.parent());
    mScripts->setup(mProject);
    mProjectModified = true;
    setProjectModified();
}

void Demo::MainWindow::on_actionRename_triggered() {
    bool ok;
    QString text = QInputDialog::getText(
                        this,
                        "Rename",
                        "New item name:",
                        QLineEdit::Normal,
                        mProject->data(mSelectedIndex).toString(),
                        &ok);
   if (ok && !text.isEmpty()) {
       mProject->setData(mSelectedIndex, QVariant::fromValue(text));
       QWidget* widget = mProject->data(mSelectedIndex, Project::EditorRole).value<QWidget*>();
       int idx = mUI->editorsTabs->indexOf(widget);
       if (idx != -1) {
           mUI->editorsTabs->setTabText(idx, text);
       }
       mScripts->setup(mProject);

       mProjectModified = true;
       setProjectModified();
   }
}

void Demo::MainWindow::on_actionEdit_triggered() {
    QWidget* widget = mProject->data(mSelectedIndex, Project::EditorRole).value<QWidget*>();
    if (mUI->editorsTabs->indexOf(widget) == -1) {
        QString label = mProject->data(mSelectedIndex).toString();
        mUI->editorsTabs->addTab(widget, label);
    }
    mUI->editorsTabs->setCurrentWidget(widget);
}

void Demo::MainWindow::on_actionReload_triggered() {
    QString fileName = mProject->data(mSelectedIndex, Project::FileNameRole).value<QString>();
    if (fileName.isEmpty()) return;
    mProject->setData(mSelectedIndex, QVariant::fromValue(fileName), Project::FileRole);
}

void Demo::MainWindow::on_actionQuit_triggered() {
    close();
}



void Demo::MainWindow::on_actionAbout_triggered() {
    QMessageBox::about(this, "OpenGL Demos", "OpenGL commands demonstration program.\nBy jukka.sirkka@iki.fi");
}

void Demo::MainWindow::on_actionAboutQt_triggered() {
    QMessageBox::aboutQt(this, "OpenGL Demos");
}


void Demo::MainWindow::on_editorsTabs_tabCloseRequested(int index) {
    mUI->editorsTabs->removeTab(index);
}

void Demo::MainWindow::on_actionPlay_triggered() {
    mUI->actionPause->setEnabled(true);
    mUI->actionPlay->setEnabled(false);
    mGLWidget->animStart();
}

void Demo::MainWindow::on_actionPause_triggered() {
    mUI->actionPlay->setEnabled(true);
    mUI->actionPause->setEnabled(false);
    mGLWidget->animStop();
}

void Demo::MainWindow::fps_changed(int value) {
    mGLWidget->animReset(value);
}

void Demo::MainWindow::initScript_changed(const QString& name) {
    mProject->setInitScript(name);
    mProjectModified = true;
    setProjectModified();
}

void Demo::MainWindow::drawScript_changed(const QString& name) {
    mProject->setDrawScript(name);
    mProjectModified = true;
    setProjectModified();
}

void Demo::MainWindow::selectionChanged() {
    const QItemSelectionModel* s = mUI->projectItems->selectionModel();
    if (!s->hasSelection()) {
        mSelectedIndex = QModelIndex();
        mUI->actionOpen->setEnabled(false);
        mUI->actionSave->setEnabled(false);
        mUI->actionSaveAs->setEnabled(false);
        mUI->actionRename->setEnabled(false);
        mUI->actionEdit->setEnabled(false);
        mUI->actionCompile->setEnabled(false);
        mUI->actionDelete->setEnabled(false);
        mUI->actionReload->setEnabled(false);
        return;
    }

    mSelectedIndex = s->selectedIndexes()[0];

    // item headers
    if (mSelectedIndex.parent() == QModelIndex()) {
        QList<QAction*> as = mUI->projectItems->actions();
        foreach (QAction* a, as) {
            mUI->projectItems->removeAction(a);
        }
        mUI->actionOpen->setEnabled(false);
        mUI->actionSave->setEnabled(false);
        mUI->actionSaveAs->setEnabled(false);
        mUI->actionRename->setEnabled(false);
        mUI->actionEdit->setEnabled(false);
        mUI->actionCompile->setEnabled(false);
        mUI->actionDelete->setEnabled(false);
        mUI->actionReload->setEnabled(false);
        return;
    }

    if (mSelectedIndex.parent() == mProject->itemParent(Project::ScriptItems)) {
        setupScriptActions();
        return;
    }

    // models/textures/shaders
    setupResourceActions();

}

#define ADDACTION(a) \
    if (!mUI->projectItems->actions().contains(mUI->action##a)) mUI->projectItems->addAction(mUI->action##a)

#define REMACTION(a) \
    if (mUI->projectItems->actions().contains(mUI->action##a)) mUI->projectItems->removeAction(mUI->action##a)


void Demo::MainWindow::setupScriptActions() {


    ADDACTION(Open);
    ADDACTION(Save);
    ADDACTION(SaveAs);
    ADDACTION(Rename);
    ADDACTION(Edit);
    ADDACTION(Compile);
    ADDACTION(Delete);
    REMACTION(Reload);

    mUI->actionOpen->setEnabled(true);

    QWidget* widget = mProject->data(mSelectedIndex, Project::EditorRole).value<QWidget*>();
    if (mUI->editorsTabs->indexOf(widget) != -1) {
        mUI->editorsTabs->setCurrentWidget(widget);
    }
    CodeEditor* editor = qobject_cast<CodeEditor*>(widget);
    mUI->actionSave->setEnabled(editor->document()->isModified());

    mUI->actionSaveAs->setEnabled(true);
    mUI->actionRename->setEnabled(true);
    mUI->actionEdit->setEnabled(true);
    mUI->actionDelete->setEnabled(true);

    mUI->actionCompile->setDisabled(mUI->actionAutocompile->isChecked());

    mUI->actionReload->setEnabled(false);

}

void Demo::MainWindow::setupResourceActions() {

    ADDACTION(Open);
    REMACTION(Save);
    REMACTION(SaveAs);
    ADDACTION(Rename);
    REMACTION(Edit);
    REMACTION(Compile);
    ADDACTION(Delete);
    ADDACTION(Reload);

    mUI->actionOpen->setEnabled(true);
    mUI->actionSave->setEnabled(false);
    mUI->actionSaveAs->setEnabled(false);
    mUI->actionRename->setEnabled(true);
    mUI->actionEdit->setEnabled(false);
    mUI->actionCompile->setEnabled(false);
    mUI->actionDelete->setEnabled(true);

    bool unbound = mProject->data(mSelectedIndex, Project::FileNameRole).value<QString>().isEmpty();
    mUI->actionReload->setDisabled(unbound);

}

#undef REMACTION
#undef ADDACTION

void Demo::MainWindow::setProjectModified() {
    bool mod = mProjectModified || mNumEdits > 0;
    setWindowModified(mod);
    mUI->actionSaveAll->setEnabled(mod);
    // Check which actions should be active
    selectionChanged();
}

void Demo::MainWindow::scriptModification_changed(bool edited) {
    mNumEdits += (edited ? 1 : -1);
    setProjectModified();
}

void Demo::MainWindow::on_actionAutocompile_toggled(bool on) {
    mUI->actionCompile->setDisabled(true);
    const QItemSelectionModel* s = mUI->projectItems->selectionModel();
    if (s && s->hasSelection()) {
        QModelIndex index = s->selectedIndexes()[0];
        if (index.parent() == mProject->itemParent(Project::ScriptItems)) {
            mUI->actionCompile->setDisabled(on);
        }
    }
    if (mProject) mProject->toggleAutoCompile(on);
}

void Demo::MainWindow::on_actionCompile_triggered() {
    const QItemSelectionModel* s = mUI->projectItems->selectionModel();
    if (s->hasSelection()) {
        QModelIndex index = s->selectedIndexes()[0];
        if (index.parent() == mProject->itemParent(Project::ScriptItems)) {
            QWidget* widget = mProject->data(index, Project::EditorRole).value<QWidget*>();
            CodeEditor* ed = qobject_cast<CodeEditor*>(widget);
            ed->compile();
        }
    }
}



void Demo::MainWindow::readSettings() {
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowstate").toByteArray());
    mLastDir = QDir(settings.value("lastdir", mLastDir.absolutePath()).toString());

    mUI->actionDemoBar->setChecked(settings.value("demobar", true).toBool());
    mUI->actionStatusbar->setChecked(settings.value("statusbar", true).toBool());
    mUI->actionToolbar->setChecked(settings.value("toolbar", true).toBool());
    mUI->actionProjectDock->setChecked(settings.value("projectdock", true).toBool());
    mUI->actionEditorsDock->setChecked(settings.value("editorsdock", true).toBool());
    mUI->actionGraphicsDock->setChecked(settings.value("graphicsdock", true).toBool());
}

void Demo::MainWindow::writeSettings() {
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowstate", saveState());
    settings.setValue("lastdir", QVariant::fromValue(mLastDir.absolutePath()));

    settings.setValue("demobar", QVariant::fromValue(mUI->demoBar->isVisible()));
    settings.setValue("statusbar", QVariant::fromValue(mUI->statusbar->isVisible()));
    settings.setValue("toolbar", QVariant::fromValue(mUI->toolBar->isVisible()));
    settings.setValue("projectdock", QVariant::fromValue(mUI->projectDock->isVisible()));
    settings.setValue("editorsdock", QVariant::fromValue(mUI->editorsDock->isVisible()));
    settings.setValue("graphicsdock", QVariant::fromValue(mUI->graphicsDock->isVisible()));
}

bool Demo::MainWindow::maybeSaveProject() {
    bool cancel = false;
    if (isWindowModified()) {
        QMessageBox msgBox;
        msgBox.setText("The project has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Save) on_actionSaveAll_triggered();
        else if (ret == QMessageBox::Cancel) cancel = true;
    }
    return !cancel;
}

bool Demo::MainWindow::maybeSave() {
    if (mSelectedIndex.parent() != mProject->itemParent(Project::ScriptItems))
        return true;
    bool cancel = false;
    QWidget* widget = mProject->data(mSelectedIndex, Project::EditorRole).value<QWidget*>();
    CodeEditor* editor = qobject_cast<CodeEditor*>(widget);
    if (editor->document()->isModified()) {
        QMessageBox msgBox;
        msgBox.setText("The scripts has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Save) on_actionSave_triggered();
        else if (ret == QMessageBox::Cancel) cancel = true;
    }
    return !cancel;
}


void Demo::MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSaveProject()) {
        delete mProject;
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void Demo::MainWindow::saveScript(const QString &fname) {
    if (mSelectedIndex.parent() != mProject->itemParent(Project::ScriptItems)) return;
    QString script = mProject->data(mSelectedIndex, Project::ScriptRole).toString();
    QFile f(fname);
    f.open(QFile::WriteOnly);
    f.write(script.toUtf8());
    f.close();
    QWidget* widget = mProject->data(mSelectedIndex, Project::EditorRole).value<QWidget*>();
    CodeEditor* editor = qobject_cast<CodeEditor*>(widget);
    editor->document()->setModified(false);
}


void Demo::MainWindow::openProject(const QString &path) {
    QString title = windowTitle();
    Project* newp(0);
    try {
        if (path.isEmpty()) {
            newp = new Project(mLastDir, mGLWidget, mGlobals, mUI->actionAutocompile->isChecked());
            title = QString("%1: new project [*]").arg(QApplication::applicationName());
        } else {
            QFileInfo info(path);
            if (info.isDir()) {
                newp = new Project(QDir(path), mGLWidget, mGlobals, mUI->actionAutocompile->isChecked());
                title = QString("%1: new project [*]").arg(QApplication::applicationName());
            } else {
                newp = new Project(path, mGLWidget, mGlobals, mUI->actionAutocompile->isChecked());
                title = QString("%1: %2 [*]").arg(QApplication::applicationName(), path);
            }
        }
        mUI->projectItems->setModel(newp);

        mUI->actionNew->setEnabled(true);

        delete mProject;
        mProject = newp;

        connect(mUI->projectItems->selectionModel(),
                SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                this,
                SLOT(selectionChanged()));

        connect(mProject,
                SIGNAL(scriptModificationChanged(bool)),
                this,
                SLOT(scriptModification_changed(bool)));

        mScripts->setup(mProject);
        mProjectModified = false;
        mNumEdits = 0;
        setProjectModified();
    } catch (BadProject& e) {
        if (!mProject) {
            title = QString("%1 [*]").arg(QApplication::applicationName());
            mUI->actionNew->setEnabled(false);
        }
        qDebug() << e.msg();
    }
    setWindowTitle(title);
}



Demo::MainWindow::~MainWindow() {
    delete mUI;
}

// #include "mainwindow.moc"
