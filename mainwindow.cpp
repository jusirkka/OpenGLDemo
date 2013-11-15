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
#include "gl_widget.h"
#include "project.h"
#include "codeeditor.h"

#include <QtDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QCloseEvent>
#include <QSettings>
#include <QUndoStack>
#include <QApplication>



Demo::MainWindow::MainWindow(const QString& project):
    QMainWindow(),
    mLastDir(QDir::home()),
    mUI(new Ui::MainWindow),
    mProject(0)
{

    mUI->setupUi(this);

    mGLWidget = new GLWidget(mUI->graphicsDockContents);
    mUI->graphicsDockLayout->addWidget(mGLWidget);

    mUI->centralwidget->hide();

    mUI->commandGroups->addAction(mUI->actionSave);
    mUI->commandGroups->addAction(mUI->actionSaveAs);
    mUI->commandGroups->addAction(mUI->actionRename);
    mUI->commandGroups->addAction(mUI->actionEdit);
    mUI->commandGroups->addAction(mUI->actionDelete);

    openProject(project, false);

    readSettings();
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

    openProject(dirName, true);
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

    openProject(fileName, false);
}

void Demo::MainWindow::on_actionSaveAll_triggered() {
    int gcount = mProject->rowCount(QModelIndex());
    int tmp = mSelectedIndex;
    for (int i = 0; i < gcount; ++i) {
        mSelectedIndex = i;
        on_actionSave_triggered();
    }
    mSelectedIndex = tmp;

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
        qDebug() << "on_actionSaveAll_triggered" << fname;
        mProject->setProjectFile(fname);
    }

    mProject->saveProject();
    setAllModified(false);

}


void Demo::MainWindow::on_actionNew_triggered() {
    mProject->appendRow("unnamed", "", "// new group of commands\n");
}

void Demo::MainWindow::on_actionOpen_triggered() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open existing group file",
        mProject->directory().absolutePath(),
        "OpenGL files (*.ogl)"
    );
    if (fileName.isEmpty()) return;

    QFileInfo info(fileName);
    mLastDir = info.absoluteDir();

    // suggest to save the current group before deleting
    if(!maybeSave()) return;

    mProject->setData(mProject->index(mSelectedIndex), QVariant::fromValue(fileName), Project::FileRole);
    QFile gfile(fileName);
    gfile.open(QFile::ReadOnly);
    mProject->setData(mProject->index(mSelectedIndex), QVariant::fromValue(QString(gfile.readAll())), Project::GroupRole);
    gfile.close();
}


void Demo::MainWindow::on_actionSave_triggered() {

    QString fname = mProject->data(mProject->index(mSelectedIndex), Project::FileRole).toString();

    QFileInfo info(fname);
    if (info.isRelative()) {
        fname = mProject->directory().absoluteFilePath(fname);
        info = QFileInfo(fname);
    }

    if (info.isFile() && info.isWritable()) {
        saveGroup(fname);
    } else {
        on_actionSaveAs_triggered();
    }
}

void Demo::MainWindow::on_actionSaveAs_triggered() {
    QString name = mProject->data(mProject->index(mSelectedIndex)).toString();

    QString fname = QFileDialog::getSaveFileName(
        this,
        QString("Select file to save the group %1 to").arg(name),
        mProject->directory().absolutePath(),
        "OpenGL files (*.ogl)"
    );
    if (fname.isEmpty()) return;
    mProject->setData(mProject->index(mSelectedIndex), QVariant::fromValue(fname), Project::FileRole);
    saveGroup(fname);
}

void Demo::MainWindow::on_actionDelete_triggered() {
    // suggest to save the current group before deleting
    if(!maybeSave()) return;

    mProject->removeRows(mSelectedIndex, 1);
}

void Demo::MainWindow::on_actionRename_triggered() {
    bool ok;
    QString text = QInputDialog::getText(
                        this,
                        "Rename",
                        "New group name:",
                        QLineEdit::Normal,
                        mProject->data(mProject->index(mSelectedIndex)).toString(),
                        &ok);
   if (ok && !text.isEmpty()) {
       mProject->setData(mProject->index(mSelectedIndex), QVariant::fromValue(text));
   }
}

void Demo::MainWindow::on_actionEdit_triggered() {
    QWidget* widget = mProject->data(mProject->index(mSelectedIndex), Project::EditorRole).value<QWidget*>();
    if (mUI->editorsTabs->indexOf(widget) == -1) {
        QString label = mProject->data(mProject->index(mSelectedIndex)).toString();
        mUI->editorsTabs->addTab(widget, label);
    }
    mUI->editorsTabs->setCurrentWidget(widget);
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

void Demo::MainWindow::selectionChanged() {
    const QItemSelectionModel* s = mUI->commandGroups->selectionModel();
    if (s->hasSelection()) {
        QModelIndex index = s->selectedIndexes()[0];
        mSelectedIndex = index.row();
        mUI->actionSaveAs->setEnabled(true);
        mUI->actionRename->setEnabled(true);
        mUI->actionEdit->setEnabled(true);
        mUI->actionDelete->setEnabled(true);

        QWidget* widget = mProject->data(mProject->index(mSelectedIndex), Project::EditorRole).value<QWidget*>();
        if (mUI->editorsTabs->indexOf(widget) != -1) {
            mUI->editorsTabs->setCurrentWidget(widget);
        }
        CodeEditor* editor = qobject_cast<CodeEditor*>(widget);
        mUI->actionSave->setEnabled(editor->document()->isModified());
    } else {
        mSelectedIndex = -1;
        mUI->actionSave->setEnabled(false);
        mUI->actionSaveAs->setEnabled(false);
        mUI->actionRename->setEnabled(false);
        mUI->actionEdit->setEnabled(false);
        mUI->actionDelete->setEnabled(false);
    }
}

void Demo::MainWindow::dataChanged() {
    bool modified = mProject->modified();
    for (int i = 0; i < mProject->rowCount(QModelIndex()) && !modified; ++i) {
        QWidget* widget = mProject->data(mProject->index(i), Project::EditorRole).value<QWidget*>();
        CodeEditor* editor = qobject_cast<CodeEditor*>(widget);
        modified = editor->document()->isModified();
    }
    setAllModified(modified);
    setupCombos();
}

void Demo::MainWindow::setAllModified(bool mod) {
    setWindowModified(mod);
    mUI->actionSaveAll->setEnabled(mod);
}

void Demo::MainWindow::setupCombos() {

    mUI->initCombo->disconnect();
    mUI->drawCombo->disconnect();

    mUI->initCombo->clear();
    mUI->drawCombo->clear();
    int initIndex = -1;
    int drawIndex = -1;
    for (int i = 0; i < mProject->rowCount(QModelIndex()); ++i) {
        QString name = mProject->data(mProject->index(i)).toString();
        if (mProject->initGroup() == name) initIndex = i;
        if (mProject->drawGroup() == name) drawIndex = i;
        mUI->initCombo->addItem(name);
        mUI->drawCombo->addItem(name);
    }
    mUI->initCombo->setCurrentIndex(initIndex);
    mUI->drawCombo->setCurrentIndex(drawIndex);



    connect(mUI->initCombo,
            SIGNAL(currentIndexChanged(QString)),
            mProject,
            SLOT(setInitGroup(QString)));
    connect(mUI->drawCombo,
            SIGNAL(currentIndexChanged(QString)),
            mProject,
            SLOT(setDrawGroup(QString)));

    connect(mUI->initCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setAllModified()));
    connect(mUI->drawCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setAllModified()));
}


void Demo::MainWindow::readSettings() {
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowstate").toByteArray());
    mLastDir = QDir(settings.value("lastdir").toString());
}

void Demo::MainWindow::writeSettings() {
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowstate", saveState());
    settings.setValue("lastdir", QVariant::fromValue(mLastDir.absolutePath()));
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
    bool cancel = false;
    QWidget* widget = mProject->data(mProject->index(mSelectedIndex), Project::EditorRole).value<QWidget*>();
    CodeEditor* editor = qobject_cast<CodeEditor*>(widget);
    if (editor->document()->isModified()) {
        QMessageBox msgBox;
        msgBox.setText("The group has been modified.");
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
        // do this before closing tabs
        delete mProject;
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void Demo::MainWindow::saveGroup(const QString &fname) {
    QString group = mProject->data(mProject->index(mSelectedIndex), Project::GroupRole).toString();
    QFile f(fname);
    f.open(QFile::WriteOnly);
    f.write(group.toAscii());
    f.close();
    QWidget* widget = mProject->data(mProject->index(mSelectedIndex), Project::EditorRole).value<QWidget*>();
    CodeEditor* editor = qobject_cast<CodeEditor*>(widget);
    editor->document()->setModified(false);
}


void Demo::MainWindow::openProject(const QString &data, bool isDir) {
    try {
        Project* newp;
        if (isDir) {
            newp = new Project(QDir(data), mGLWidget);
        } else {
            newp = new Project(data, mGLWidget);
        }
        mUI->commandGroups->setModel(newp);
        delete mProject;
        mProject = newp;
        mUI->actionNew->setEnabled(true);
        mUI->actionOpen->setEnabled(true);
        connect(mUI->commandGroups->selectionModel(),
                SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                this,
                SLOT(selectionChanged()));
        connect(mProject,
                SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this,
                SLOT(dataChanged()));
        setupCombos();
        setAllModified(false);
    } catch (BadProject& e) {
        if (!mProject) {
            mUI->actionNew->setEnabled(false);
            mUI->actionOpen->setEnabled(false);
        }
        qDebug() << e.msg();
    }

}



Demo::MainWindow::~MainWindow() {
    delete mUI;
}

// #include "mainwindow.moc"
