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

#include <QtDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QCloseEvent>
#include <QSettings>
#include <QUndoStack>
#include <QApplication>
#include <QClipboard>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTextEdit>



Demo::MainWindow::MainWindow(const QString& demoFile):
    QMainWindow(),
    mDemoFile(),
    mLastDir(QDir::homePath()),
    mUI(new Ui::MainWindow),
    mEdited(false)
{

    mUI->setupUi(this);

    mGLWidget = new GLWidget(mUI->centralwidget);
    mUI->centralLayout->addWidget(mGLWidget);

    QListWidget* list = mUI->commandGroups;

    mNames.append("Init");
    mNames.append("Draw");
    mGroups.append("");
    mGroups.append("");
    list->addItems(mNames);


    readSettings();

    openDemo(demoFile);

}


bool Demo::MainWindow::maybeSave() {
    bool cancel = false;
    if (mEdited) {
        QMessageBox msgBox;
        msgBox.setText("The demo has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Save) on_actionSave_triggered();
        else if (ret == QMessageBox::Cancel) cancel = true;
    }
    return !cancel;
}

void Demo::MainWindow::on_actionNew_triggered() {
    qDebug() << "MainWindow::on_actionNew_triggered()";
}

void Demo::MainWindow::on_actionOpen_triggered() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open existing demo file",
        mLastDir,
        "OpenGL files (*.ogl)"
    );
    if (fileName.isEmpty()) return;
    QString lastDir = fileName.left(fileName.lastIndexOf(QDir::separator()));
    if (QDir::isAbsolutePath(lastDir)) mLastDir = lastDir;
    openDemo(fileName);
}

void Demo::MainWindow::openDemo(const QString& demoFile) {
    qDebug() << "MainWindow::openDemo";

    if (demoFile.isEmpty()) return;

    // suggest to save the current demo before deleting
    if(!maybeSave()) return;

    mUI->actionSave->setEnabled(false);
    setWindowModified(false);
    mEdited = false;

    mNames.clear();
    mGroups.clear();
    mNames.append("Init");
    mNames.append("Draw");
    mGroups.append("");
    mGroups.append("");

    QMap<QString, QString> map;
    QRegExp meta("^//\\s*meta\\s*:\\s*name\\s*:(.*)");
    QFile file(demoFile);
    file.open(QFile::ReadOnly);

    QString line(file.readLine());
    QString key("unknown");
    while (!line.isEmpty()) {
        if (meta.exactMatch(line)) {
            key = meta.cap(1).trimmed();
            map[key] = "";
        } else {
            map[key].append(line);
        }
        line = QString(file.readLine());
    }


    if (!map.contains("Draw") || !map.contains("Init")) {
        qWarning() << "Draw or Init missing";
    } else {

        mGroups[0] = map["Init"]; map.remove("Init");
        mGroups[1] = map["Draw"]; map.remove("Draw");

        foreach(const QString& key, map.keys()) {
            mNames.append(key);
            mGroups.append(map[key]);
        }

    }

    foreach (int key, mEditors.keys()) {
        mEditors[key]->setText(mGroups[key]);
    }

    for (int key = 0; key < mGroups.length(); key++) {
        mGLWidget->parse(key, mGroups[key]);
    }

    mUI->commandGroups->clear();
    mUI->commandGroups->addItems(mNames);
    mDemoFile = demoFile;


}

void Demo::MainWindow::on_actionSave_triggered() {
    qDebug() << "MainWindow::on_actionSave_triggered()";
    if(!mDemoFile.isEmpty()) {
        saveDemoAs(mDemoFile);
    } else {
        on_actionSaveAs_triggered();
    }
}

void Demo::MainWindow::on_actionSaveAs_triggered() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save demo to a file",
        mLastDir,
        "OpenGL files (*.ogl)"
    );
    if (fileName.isEmpty()) return;
    QString lastDir = fileName.left(fileName.lastIndexOf(QDir::separator()));
    if (QDir::isAbsolutePath(lastDir)) mLastDir = lastDir;
    saveDemoAs(fileName);
}

void Demo::MainWindow::saveDemoAs(const QString& demoFile) {
    foreach(int key, mEditors.keys()) {
        mGroups[key] = mEditors[key]->toPlainText();
//        qDebug() << "save row = " << key;
//        qDebug() << "save text = " << mGroups[key];
    }

    QByteArray contents;
    for (int i = 0; i < mNames.length(); i++) {
        contents.append(QString("// meta:name: %1\n").arg(mNames[i]));
        contents.append(mGroups[i]);
    }

    QFile file(demoFile);
    file.open(QIODevice::WriteOnly);
    file.write(contents);
    file.close();

    mDemoFile = demoFile;

    mUI->actionSave->setEnabled(false);
    setWindowModified(false);
    mEdited = false;
}

void Demo::MainWindow::on_actionQuit_triggered() {
    close();
}


void Demo::MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSave()) {
        foreach(QTextEdit* ed, mEditors.values()) {
            ed->close();
        }
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void Demo::MainWindow::on_actionAbout_triggered() {
    QMessageBox::about(this, "OpenGL Demos", "OpenGL commands demonstration program.\nBy jukka.sirkka@iki.fi");
}

void Demo::MainWindow::on_actionAboutQt_triggered() {
    QMessageBox::aboutQt(this, "OpenGL Demos");
}


void Demo::MainWindow::on_actionCut_triggered() {
}

void Demo::MainWindow::on_actionCopy_triggered() {
}

void Demo::MainWindow::on_actionPaste_triggered() {
}


void Demo::MainWindow::on_buttonNew_pressed() {
}

void Demo::MainWindow::on_buttonEdit_pressed() {
    int row = mUI->commandGroups->currentRow();
    if (row == -1) return;
//    qDebug() << "row = " << row;
//    qDebug() << "text = " << mGroups[row];
    QTextEdit* editor = new QTextEdit();
    editor->setAttribute(Qt::WA_DeleteOnClose);


    mUI->buttonEdit->setEnabled(false);

    mEditors[row] = editor;

    editor->setPlainText(mGroups[row]);

    connect(editor, SIGNAL(textChanged()), this, SLOT(parse()));
    connect(editor, SIGNAL(destroyed(QObject*)), this, SLOT(childClosed(QObject*)));

    editor->show();
}


void Demo::MainWindow::parse() {
    if (!mEdited) {
        mUI->actionSave->setEnabled(true);
        setWindowModified(true);
        mEdited = true;
    }

    QTextEdit* editor = qobject_cast<QTextEdit*>(sender());
    int key = mEditors.key(editor);
    mGLWidget->parse(key, editor->toPlainText());
}


void Demo::MainWindow::childClosed(QObject* sender) {
    qDebug() << "childClosed" << sender;
    QTextEdit* editor = qobject_cast<QTextEdit*>(sender);


    int key = mEditors.key(editor);
    if (!mUI->commandGroups->currentRow() == key) {
        mUI->buttonEdit->setEnabled(true);
    }
    mEditors.remove(key);

    if (editor) mGroups[key] = editor->toPlainText();

//    qDebug() << "key = " << key;
//    qDebug() << "text = " << mGroups[key];

}

void Demo::MainWindow::on_buttonDelete_pressed() {
}

void Demo::MainWindow::on_commandGroups_currentRowChanged(int row) {
    if (row == -1) {
        mUI->buttonEdit->setEnabled(false);
        mUI->buttonDelete->setEnabled(false);
    } else if (row == 0 || row == 1) {
        mUI->buttonEdit->setEnabled(true);
        mUI->buttonDelete->setEnabled(false);
    } else {
        mUI->buttonEdit->setEnabled(true);
        mUI->buttonDelete->setEnabled(true);
    }

    if (mEditors.contains(row)) {
        mUI->buttonEdit->setEnabled(false);
    }
}

void Demo::MainWindow::readSettings() {
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowstate").toByteArray());
}

void Demo::MainWindow::writeSettings() {
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowstate", saveState());
}


Demo::MainWindow::~MainWindow() {
    delete mUI;
}

// #include "mainwindow.moc"
