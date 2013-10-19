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
#include <QTextEdit>

namespace Ui {
class MainWindow;
}

namespace Demo {

class GLWidget;

class MainWindow: public QMainWindow {

        Q_OBJECT

    public:

        MainWindow(const QString& demoFile);
        virtual ~MainWindow();

    protected:

        virtual void closeEvent(QCloseEvent *event);

    private slots:

        //! New command group.
        void on_actionNew_triggered();

        //! Show a file dialog to open a demo file.
        void on_actionOpen_triggered();

        //! Save the demo.
        void on_actionSave_triggered();

        //! Show a file dialog to save a demo to a new file.
        void on_actionSaveAs_triggered();

        //! As it says.
        void on_actionQuit_triggered();

        //! About $APP
        void on_actionAbout_triggered();

        //! About Qt
        void on_actionAboutQt_triggered();

        //! As it says.
        void on_actionCut_triggered();
        void on_actionCopy_triggered();
        void on_actionPaste_triggered();

        void on_buttonNew_pressed();
        void on_buttonEdit_pressed();
        void on_buttonDelete_pressed();

        void on_commandGroups_currentRowChanged(int);

        void parse();
        void childClosed(QObject*);

private:

        //! The actual method to write the scene to a file.
        void saveDemoAs(const QString& demoFile);

        //! Test if there are unsaved edits before clearing a scene.
        bool maybeSave();

        //! Open and load a saved demo.
        void openDemo(const QString& demoFile);

        //! Read saved state
        void readSettings();

        //! Save current state
        void writeSettings();


    private:

        QString mDemoFile;
        QString mLastDir;
        Ui::MainWindow *mUI;
        QStringList mGroups;
        QStringList mNames;
        QMap<int, QTextEdit*> mEditors;
        GLWidget* mGLWidget;
        bool mEdited;
};

}

#endif
