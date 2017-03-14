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

#ifndef DEMO_MAINWINDOW_H
#define DEMO_MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QModelIndex>
#include <QDir>

#include "gl_lang_compiler.h"

namespace Ui {class MainWindow;}

namespace Demo {

class GLWidget;
class Project;
class ScriptSelector;
class Scope;


class MainWindow: public QMainWindow {

    Q_OBJECT

public:

    MainWindow(const QString& project);
    virtual ~MainWindow();

protected:

    virtual void closeEvent(QCloseEvent *event);

private slots:

    //! Show a file dialog to select new project dir.
    void on_actionNewProject_triggered();

    //! Show a file dialog to open a project file.
    void on_actionOpenProject_triggered();

    //! save the project.
    void on_actionSaveAll_triggered();

    //! New item.
    void on_actionNew_triggered();

    //! Show a file dialog to insert contents of a file.
    void on_actionInsert_triggered();

    //! Show a file dialog to open a script.
    void on_actionOpen_triggered();

    //! Save the script.
    void on_actionSave_triggered();

    //! Show a file dialog to save the script to a new file.
    void on_actionSaveAs_triggered();

    //! As it says.
    void on_actionDelete_triggered();

    //! As it says.
    void on_actionRename_triggered();

    //! As it says.
    void on_actionEdit_triggered();

    //! As it says.
    void on_actionReload_triggered();

    //! As it says.
    void on_actionQuit_triggered();

    //! About $APP
    void on_actionAbout_triggered();

    //! About Qt
    void on_actionAboutQt_triggered();

    void on_editorsTabs_tabCloseRequested(int index);
    void on_editorsTabs_currentChanged(int index);

    void on_actionPlay_triggered();
    void on_actionPause_triggered();
    void fps_changed(int);

    void initScript_changed(const QString&);
    void drawScript_changed(const QString&);

    void selectionChanged();
    void setProjectModified();

    void on_actionAutocompile_toggled(bool on);
    void on_actionCompile_triggered();
    void on_actionCompileProject_triggered();

    void scriptModification_changed(bool edited);

private:


    //! Test if there are unsaved edits before clearing the project.
    bool maybeSaveProject();
    bool maybeSave();
    void saveScript(const QString& fname);

    //! Read saved state
    void readSettings();

    //! Save current state
    void writeSettings();

    void openProject(const QString& data);

    void setupScriptActions();
    void setupResourceActions();

private:

    QDir mLastDir;
    Ui::MainWindow *mUI;
    GLWidget* mGLWidget;
    Project* mProject;
    QModelIndex mSelectedIndex;
    ScriptSelector* mScripts;
    bool mProjectModified;
    int mNumEdits;
    Scope* mGlobals;
};

}

#endif
