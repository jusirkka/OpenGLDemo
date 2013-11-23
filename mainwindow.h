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

#ifndef demo_mainwindow_h
#define demo_mainwindow_h

#include <QMainWindow>
#include <QStringList>
#include <QModelIndex>
#include <QDir>

namespace Ui {
class MainWindow;
}

namespace Demo {

class GLWidget;
class Project;

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


    //! New command group.
    void on_actionNew_triggered();

    //! Show a file dialog to open a group file.
    void on_actionOpen_triggered();

    //! Save the group.
    void on_actionSave_triggered();

    //! Show a file dialog to save a group to a new file.
    void on_actionSaveAs_triggered();

    //! As it says.
    void on_actionDelete_triggered();

    //! As it says.
    void on_actionRename_triggered();

    //! As it says.
    void on_actionEdit_triggered();

    //! As it says.
    void on_actionQuit_triggered();

    //! About $APP
    void on_actionAbout_triggered();

    //! About Qt
    void on_actionAboutQt_triggered();

    void on_editorsTabs_tabCloseRequested(int index);

    void selectionChanged();
    void dataChanged();
    void setAllModified(bool = true);

private:


    //! Test if there are unsaved edits before clearing the project.
    bool maybeSaveProject();
    bool maybeSave();
    void saveGroup(const QString& fname);

    //! Read saved state
    void readSettings();

    //! Save current state
    void writeSettings();

    void setupCombos();

    void openProject(const QString& data, bool isDir);

private:

    QDir mLastDir;
    Ui::MainWindow *mUI;
    GLWidget* mGLWidget;
    Project* mProject;
    QModelIndex mSelectedIndex;
};

}

#endif
