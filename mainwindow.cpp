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
#include "constant.h"
#include "scope.h"
#include "depthzoom.h"

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
#include <QTimer>

#define  GLSL_PATT "*.vert *.tesc *.tese *.geom *.frag *.comp *.glsl"

using namespace Demo;

MainWindow::MainWindow(const QString& project)
    : QMainWindow()
    , mLastDir(QDir::home())
    , mUI(new Ui::MainWindow)
    , mProject(nullptr)
    , mProjectModified(false)
    , mNumEdits(0)
    , mPlaying(false)
{


    mUI->setupUi(this);

    auto fps = new FPSControl();
    connect(fps, SIGNAL(valueChanged(int)), this, SLOT(fps_changed(int)));
    mUI->demoBar->addWidget(fps);

    auto zoom = new DepthZoom();
    connect(zoom, SIGNAL(valuesChanged(float, float)), this, SLOT(depthChanged(float, float)));
    mUI->demoBar->addWidget(zoom);

    mGLWidget = new GLWidget();
    mGLWidget->setWindowFlags(Qt::WindowStaysOnTopHint);
    connect(mGLWidget, SIGNAL(hidden()), this, SLOT(hideScene()));
    connect(mGLWidget, SIGNAL(toggleAnimate()), this, SLOT(on_actionPlay_triggered()));

    // Initialize globals
    mGlobals = new Scope(mGLWidget, this);

    // cannot record until opengl is initialized
    mUI->actionRecord->setEnabled(false);
    connect(mGLWidget, SIGNAL(openGLReady(bool)), mUI->actionRecord, SLOT(setEnabled(bool)));

    // init the projection near/far values
    depthChanged(zoom->near(), zoom->far());


    mUI->projectItems->addAction(mUI->actionInsert);
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

    auto hackTimer = new QTimer(this);
    connect(hackTimer, SIGNAL(timeout()), this, SLOT(restoreDocking()));
    hackTimer->setSingleShot(true);
    hackTimer->start(300);
}

void Demo::MainWindow::depthChanged(float near, float far) {
    mGLWidget->setProjection(near, far);
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
        // qCDebug(OGL) << "on_actionSaveAll_triggered" << fname;
        mProject->setProjectFile(fname);
        setWindowTitle(QString("%1: %2 [*]").arg(QApplication::applicationName(), fname));
    }

    for (int i = 0; i < mProject->rowCount(mProject->itemParent(Project::ScriptItems)); ++i) {
        auto script = mProject->index(i, Project::ScriptItems);
        auto editor = qobject_cast<CodeEditor*>(mProject->data(script, Project::EditorRole).value<QWidget*>());
        if (editor->document()->isModified()) {
            saveScript(script);
        } else if (mProject->data(script, Project::FileNameRole) == "") {
            saveScriptAs(script);
        }
    }

    mProject->saveProject();

    mProjectModified = false;
    mNumEdits = 0;
    setProjectModified();

}


void Demo::MainWindow::on_actionNew_triggered() {
    NewDialog dlg(mProject);
    if (dlg.exec() == QDialog::Accepted) {
        mProject->appendRow(dlg.name(), "", dlg.listParent());
        mProjectModified = true;
        setProjectModified();
    }
}

void Demo::MainWindow::on_actionInsert_triggered() {
    auto script = getSelection();
    if (!script.isValid()) return;
    // Just be sure, insert shouldn't be active in other cases
    if (script.parent() != mProject->itemParent(Project::ScriptItems)) return;
    QString title("Insert contents of a script");
    QString filter("OpenGL script files ( *.ogl)");

    QString fileName = QFileDialog::getOpenFileName(
                this,
                title,
                mProject->directory().absolutePath(),
                filter
                );
    if (fileName.isEmpty()) return;

    QFileInfo info(fileName);
    // TODO: inform user that op failed
    if (!info.exists() || !info.isFile() || !info.isReadable()) return;
    mLastDir = info.absoluteDir();

    QFile file(fileName);
    file.open(QFile::ReadOnly);
    mProject->setData(script, QVariant::fromValue(file.readAll()), Project::ScriptRole);
    file.close();
    setProjectModified();
}

void Demo::MainWindow::on_actionOpen_triggered() {
    QString title;
    QString filter;
    auto item = getSelection();
    if (!item.isValid()) return;
    if (item.parent() == mProject->itemParent(Project::ScriptItems)) {
        title = "Open a file and bind to a script";
        filter = "OpenGL script files ( *.ogl)";
    } else if (item.parent() == mProject->itemParent(Project::ModelItems)) {
        title = "Open a model file to create vertex data";
        filter = "Model files ( *.obj)";
    } else if (item.parent() == mProject->itemParent(Project::ImageItems)) {
        title = "Open an image to create texture data";
        filter = "Image files (";
        const auto formats = QImageReader::supportedImageFormats();
        for (auto& b: formats) {
            filter += QString(" *.%1").arg(QString(b));
        }
        filter += ")";
    } else if (item.parent() == mProject->itemParent(Project::ShaderItems)) {
        title = "Open a shader source file to create a shader";
        filter = "GLSL files (" GLSL_PATT ")";
    } else if (item.parent() == mProject->itemParent(Project::TextureItems)) {
        title = "Open a KTX file to create a texture";
        filter = "KTX files ( *.ktx)";
    }

    QString fileName = QFileDialog::getOpenFileName(
                this,
                title,
                mLastDir.absolutePath(),
                filter
                );
    if (fileName.isEmpty()) return;

    QFileInfo info(fileName);
    mLastDir = info.absoluteDir();

    // suggest to save the current script before deleting
    if(!maybeSave(item)) return;

    mProject->setData(item, QVariant::fromValue(fileName), Project::FileRole);
    mProjectModified = true;
    setProjectModified();
}

void Demo::MainWindow::on_actionSave_triggered() {
    saveScript(getSelection());
}

void Demo::MainWindow::saveScript(const QModelIndex& item) {

    if (item.parent() != mProject->itemParent(Project::ScriptItems)) return;

    QString fname = mProject->data(item, Project::FileNameRole).toString();

    QFileInfo info(fname);
    if (info.isRelative()) {
        fname = mProject->directory().absoluteFilePath(fname);
        info = QFileInfo(fname);
    }

    if (info.isFile() && info.isWritable()) {
        saveScript(item, fname);
    } else {
        saveScriptAs(item);
    }
}

void Demo::MainWindow::on_actionSaveAs_triggered() {
    saveScriptAs(getSelection());
}

void Demo::MainWindow::saveScriptAs(const QModelIndex& item) {

    if (item.parent() != mProject->itemParent(Project::ScriptItems)) return;

    auto name = mProject->data(item).toString();

    QString fname = QFileDialog::getSaveFileName(
        this,
        QString(R"(Select file to save the script "%1" to)").arg(name),
        mProject->directory().absolutePath(),
        "OpenGL command files (*.ogl)"
    );
    if (fname.isEmpty()) return;
    saveScript(item, fname);
    mProject->setData(item, QVariant::fromValue(fname), Project::FileNameRole);
    mProjectModified = true;
    setProjectModified();
}

void Demo::MainWindow::on_actionDelete_triggered() {

    auto item = getSelection();
    // do not remove header items
    if (!item.parent().isValid()) return;
    // suggest to save the current script before deleting
    if(!maybeSave(item)) return;
    auto widget = mProject->data(item, Project::EditorRole).value<QWidget*>();
    int idx = mUI->editorsTabs->indexOf(widget);
    mProject->removeRows(item.row(), 1, item.parent());
    if (idx != -1) { // tab removal generates selection change
        mUI->editorsTabs->removeTab(idx);
    }
    mProjectModified = true;
    setProjectModified();
}

void Demo::MainWindow::on_actionRename_triggered() {
    auto item = getSelection();
    // do not rename headers
    if (!item.parent().isValid()) return;
    bool ok;
    QString text = QInputDialog::getText(
                        this,
                        "Rename",
                        "New item name:",
                        QLineEdit::Normal,
                        mProject->data(item).toString(),
                        &ok);
   if (ok && !text.isEmpty()) {
       mProject->setData(item, QVariant::fromValue(text));
       QWidget* widget = mProject->data(item, Project::EditorRole).value<QWidget*>();
       int idx = mUI->editorsTabs->indexOf(widget);
       if (idx != -1) {
           mUI->editorsTabs->setTabText(idx, text);
       }

       mProjectModified = true;
       setProjectModified();
   }
}

void Demo::MainWindow::on_actionEdit_triggered() {
    auto item = getSelection();
    if (item.parent() != mProject->itemParent(Project::ScriptItems)) return;
    auto widget = mProject->data(item, Project::EditorRole).value<QWidget*>();
    if (mUI->editorsTabs->indexOf(widget) == -1) {
        auto label = mProject->data(item).toString();
        mUI->editorsTabs->addTab(widget, label);
    }
    mUI->editorsTabs->setCurrentWidget(widget);
    selectionChanged(); // check which actions should be active
}

void Demo::MainWindow::on_actionReload_triggered() {
    auto item = getSelection();
    if (item.parent() != mProject->itemParent(Project::ModelItems) &&
            item.parent() != mProject->itemParent(Project::ImageItems) &&
            item.parent() != mProject->itemParent(Project::TextureItems) &&
            item.parent() != mProject->itemParent(Project::ShaderItems)) {
        return;
    }

    QString fileName = mProject->data(item, Project::FileNameRole).toString();
    if (fileName.isEmpty()) return;
    mProject->setData(item, QVariant::fromValue(fileName), Project::FileRole);
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

void Demo::MainWindow::on_editorsTabs_currentChanged(int index) {
    if (index < 0) return;
    QItemSelectionModel* s = mUI->projectItems->selectionModel();
    // check if selection already points to the correct tab
    if (!s->selectedIndexes().isEmpty()) {
        QModelIndex sel = s->selectedIndexes()[0];
        QWidget* widget = mProject->data(sel, Project::EditorRole).value<QWidget*>();
        if (mUI->editorsTabs->indexOf(widget) == index) return;
    }

    // find the editor's modelindex
    for (int i = 0; i < mProject->rowCount(mProject->itemParent(Project::ScriptItems)); ++i) {
        QModelIndex edIndex = mProject->index(i, Project::ScriptItems);
        QWidget* widget = mProject->data(edIndex, Project::EditorRole).value<QWidget*>();
        if (mUI->editorsTabs->indexOf(widget) == index) {
            // set selection
            s->select(edIndex, QItemSelectionModel::ClearAndSelect);
            return;
        }
    }
}

void Demo::MainWindow::on_actionPlay_triggered() {
    if (mPlaying) {
        mGLWidget->animStop();
        mUI->actionPlay->setIcon(QIcon::fromTheme("media-playback-start"));
        mUI->actionPlay->setToolTip("Start Animation");
    } else {
        mGLWidget->animStart();
        mUI->actionPlay->setIcon(QIcon::fromTheme("media-playback-pause"));
        mUI->actionPlay->setToolTip("Pause Animation");
    }
    mPlaying = !mPlaying;
}

void Demo::MainWindow::hideScene() {
    if (mSceneVisible) {
        mUI->actionViewScene->setIcon(QIcon::fromTheme("camera-photo"));
        mUI->actionViewScene->setToolTip("View the GL scene");
        mUI->actionViewScene->setChecked(false);
        mSceneVisible = false;

        mGLWidget->animStop();
        mUI->actionPlay->setIcon(QIcon::fromTheme("media-playback-start"));
        mUI->actionPlay->setToolTip("Start Animation");
        mPlaying = false;
    }
}

void Demo::MainWindow::on_actionViewScene_triggered() {
    if (mGLWidget->isVisible()) {
        mGLWidget->hide();
        mUI->actionViewScene->setToolTip("View the GL scene");
        mUI->actionViewScene->setChecked(false);
        mSceneVisible = false;

        mGLWidget->animStop();
        mUI->actionPlay->setIcon(QIcon::fromTheme("media-playback-start"));
        mUI->actionPlay->setToolTip("Start Animation");
        mPlaying = false;

    } else {
        mGLWidget->show();
        mUI->actionViewScene->setToolTip("Hide the GL scene");
        mSceneVisible = true;
        mUI->actionViewScene->setChecked(true);
    }
}

void Demo::MainWindow::fps_changed(int value) {
    mGLWidget->animReset(value);
}

QModelIndex Demo::MainWindow::getSelection() {
    const QItemSelectionModel* s = mUI->projectItems->selectionModel();
    if (!s->hasSelection()) {
        return QModelIndex();
    }
    if (s->selectedIndexes().isEmpty()) {
        return QModelIndex();
    }
    return s->selectedIndexes()[0];
}

void Demo::MainWindow::selectionChanged() {

    QModelIndex selection = getSelection();

    if (!selection.isValid()) {
        mUI->actionInsert->setEnabled(false);
        mUI->actionOpen->setEnabled(false);
        mUI->actionSave->setEnabled(false);
        mUI->actionSaveAs->setEnabled(false);
        mUI->actionRename->setEnabled(false);
        mUI->actionEdit->setEnabled(false);
        mUI->actionCompile->setEnabled(false);
        mUI->actionDelete->setEnabled(false);
        mUI->actionReload->setEnabled(false);
        mUI->actionComplete->setEnabled(false);

        QWidget* curr = mUI->editorsTabs->currentWidget();
        if (curr) curr->setDisabled(true);
        return;
    }


    // item headers
    if (!selection.parent().isValid()) {
        const auto as = mUI->projectItems->actions();
        for (auto a: as) {
            mUI->projectItems->removeAction(a);
        }
        mUI->actionInsert->setEnabled(false);
        mUI->actionOpen->setEnabled(false);
        mUI->actionSave->setEnabled(false);
        mUI->actionSaveAs->setEnabled(false);
        mUI->actionRename->setEnabled(false);
        mUI->actionEdit->setEnabled(false);
        mUI->actionCompile->setEnabled(false);
        mUI->actionComplete->setEnabled(false);
        mUI->actionDelete->setEnabled(false);
        mUI->actionReload->setEnabled(false);
        mUI->actionComplete->setEnabled(false);

        QWidget* curr = mUI->editorsTabs->currentWidget();
        if (curr) curr->setDisabled(true);
        return;
    }

    setupScriptActions(selection);

    // models/images/shaders
    setupResourceActions(selection);

}

#define ADDACTION(a) \
    if (!mUI->projectItems->actions().contains(mUI->action##a)) mUI->projectItems->addAction(mUI->action##a)

#define REMACTION(a) \
    if (mUI->projectItems->actions().contains(mUI->action##a)) mUI->projectItems->removeAction(mUI->action##a)


void Demo::MainWindow::setupScriptActions(const QModelIndex& selection) {

    if (selection.parent() != mProject->itemParent(Project::ScriptItems)) return;

    auto scriptName = mProject->data(selection).toString();
    QStringList unmods;
    unmods << mProject->initScriptName() << mProject->drawScriptName();

    if (unmods.contains(scriptName)) {
        ADDACTION(Insert);
        ADDACTION(Open);
        ADDACTION(Save);
        ADDACTION(SaveAs);
        REMACTION(Rename);
        ADDACTION(Edit);
        ADDACTION(Compile);
        REMACTION(Delete);
        REMACTION(Reload);
        mUI->actionRename->setEnabled(false);
        mUI->actionDelete->setEnabled(false);
    } else {
        ADDACTION(Insert);
        ADDACTION(Open);
        ADDACTION(Save);
        ADDACTION(SaveAs);
        ADDACTION(Rename);
        ADDACTION(Edit);
        ADDACTION(Compile);
        ADDACTION(Delete);
        REMACTION(Reload);
        mUI->actionRename->setEnabled(true);
        mUI->actionDelete->setEnabled(true);
    }

    mUI->actionInsert->setEnabled(true);
    mUI->actionOpen->setEnabled(true);

    QWidget* widget = mProject->data(selection, Project::EditorRole).value<QWidget*>();
    if (mUI->editorsTabs->indexOf(widget) != -1) {
        mUI->editorsTabs->setCurrentWidget(widget);
    }
    CodeEditor* editor = qobject_cast<CodeEditor*>(widget);
    mUI->actionSave->setEnabled(editor->document()->isModified());

    QWidget* curr = mUI->editorsTabs->currentWidget();
    if (curr) {
        curr->setEnabled(curr == widget);
    }

    mUI->actionSaveAs->setEnabled(true);
    mUI->actionEdit->setEnabled(true);

    mUI->actionCompile->setDisabled(mUI->actionAutocompile->isChecked());

    mUI->actionReload->setEnabled(false);

    mUI->actionComplete->setEnabled(curr && curr == widget);

}

void Demo::MainWindow::setupResourceActions(const QModelIndex& selection) {

    if (selection.parent() != mProject->itemParent(Project::ModelItems) &&
            selection.parent() != mProject->itemParent(Project::ImageItems) &&
            selection.parent() != mProject->itemParent(Project::TextureItems) &&
            selection.parent() != mProject->itemParent(Project::ShaderItems)) {
        return;
    }


    REMACTION(Insert);
    ADDACTION(Open);
    REMACTION(Save);
    REMACTION(SaveAs);
    ADDACTION(Rename);
    REMACTION(Edit);
    REMACTION(Compile);
    ADDACTION(Delete);
    ADDACTION(Reload);

    mUI->actionInsert->setEnabled(false);
    mUI->actionOpen->setEnabled(true);
    mUI->actionSave->setEnabled(false);
    mUI->actionSaveAs->setEnabled(false);
    mUI->actionRename->setEnabled(true);
    mUI->actionEdit->setEnabled(false);
    mUI->actionCompile->setEnabled(false);
    mUI->actionDelete->setEnabled(true);

    bool unbound = mProject->data(selection, Project::FileNameRole).toString().isEmpty();
    mUI->actionReload->setDisabled(unbound);

    mUI->actionComplete->setEnabled(false);

    QWidget* curr = mUI->editorsTabs->currentWidget();
    if (curr) curr->setDisabled(true);

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
    if (!mProject) return;
    const QItemSelectionModel* s = mUI->projectItems->selectionModel();
    if (s && s->hasSelection()) {
        QModelIndex index = s->selectedIndexes()[0];
        if (index.parent() == mProject->itemParent(Project::ScriptItems)) {
            mUI->actionCompile->setDisabled(on);
        }
    }
    mProject->toggleAutoCompile(on);
}

void Demo::MainWindow::on_actionRecord_toggled(bool on) {
    QString tmpl;
    QFileInfo info(mProject->directory().absoluteFilePath(mProject->projectFile()));
    if (info.exists() && info.isFile()) {
        tmpl = mProject->directory().absoluteFilePath(info.completeBaseName());
    } else {
        tmpl = mProject->directory().absoluteFilePath(QApplication::applicationName());
    }
    // qCDebug(OGL) << "record file template" << tmpl;

    mGLWidget->saveToDisk(on, tmpl);
}

void Demo::MainWindow::on_actionCompile_triggered() {
    if (!mProject) return;
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

void Demo::MainWindow::on_actionCompileProject_triggered() {
    if (mProject) mProject->recompileProject();
}


void Demo::MainWindow::on_actionComplete_triggered() {
    if (!mProject) return;
    const QItemSelectionModel* s = mUI->projectItems->selectionModel();
    if (s->hasSelection()) {
        QModelIndex index = s->selectedIndexes()[0];
        if (index.parent() == mProject->itemParent(Project::ScriptItems)) {
            QWidget* widget = mProject->data(index, Project::EditorRole).value<QWidget*>();
            CodeEditor* ed = qobject_cast<CodeEditor*>(widget);
            ed->complete();
        }
    }
}

void Demo::MainWindow::readSettings() {
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    mLastDir = QDir(settings.value("lastdir", mLastDir.absolutePath()).toString());

    mUI->actionDemoBar->setChecked(settings.value("demobar", true).toBool());
    mUI->actionStatusbar->setChecked(settings.value("statusbar", true).toBool());
    mUI->actionToolbar->setChecked(settings.value("toolbar", true).toBool());

    mGLWidget->resize(settings.value("scenesize", QSize(400, 225)).toSize());

    if (settings.value("scenevisible", false).toBool()) {
        on_actionViewScene_triggered();
    }
}

void Demo::MainWindow::restoreDocking() {
    QSettings settings;
    restoreState(settings.value("windowstate").toByteArray());
    bool ok;
    int w_pr = settings.value("projectdock-width", 100).toInt(&ok);
    if (!ok) w_pr = 100;

    resizeDocks({mUI->projectDock}, {w_pr}, Qt::Horizontal);
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
    settings.setValue("projectdock-width", QVariant::fromValue(mUI->projectDock->width()));

    settings.setValue("scenevisible", QVariant::fromValue(mGLWidget->isVisible()));
    settings.setValue("scenesize", QVariant::fromValue(mGLWidget->size()));

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

bool Demo::MainWindow::maybeSave(const QModelIndex& item) {
    if (item.parent() != mProject->itemParent(Project::ScriptItems)) return true;
    bool cancel = false;
    QWidget* widget = mProject->data(item, Project::EditorRole).value<QWidget*>();
    CodeEditor* editor = qobject_cast<CodeEditor*>(widget);
    if (editor->document()->isModified()) {
        QMessageBox msgBox;
        msgBox.setText("The script has been modified.");
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
        writeSettings();
        mGLWidget->hide();
        event->accept();
    } else {
        event->ignore();
    }
}

void Demo::MainWindow::saveScript(const QModelIndex& script, const QString &fname) {
    if (script.parent() != mProject->itemParent(Project::ScriptItems)) return;
    auto data = mProject->data(script, Project::ScriptRole).toString();
    QFile f(fname);
    f.open(QFile::WriteOnly);
    f.write(data.toUtf8());
    f.close();
    auto editor = qobject_cast<CodeEditor*>(mProject->data(script, Project::EditorRole).value<QWidget*>());
    editor->document()->setModified(false);
}


void Demo::MainWindow::openProject(const QString &path) {
    QString title = windowTitle();
    Project* newp(nullptr);
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
        mUI->actionCompileProject->setEnabled(true);

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

        mProjectModified = false;
        mNumEdits = 0;
        setProjectModified();
    } catch (BadProject& e) {
        if (!mProject) {
            title = QString("%1 [*]").arg(QApplication::applicationName());
            mUI->actionNew->setEnabled(false);
            mUI->actionCompileProject->setEnabled(false);
        }
        qWarning() << e.msg();
    }
    setWindowTitle(title);
}



Demo::MainWindow::~MainWindow() {
    disconnect();
    mUI->editorsTabs->disconnect();
    delete mUI;
    delete mGLWidget;
    mProject->deleteLater();
}

