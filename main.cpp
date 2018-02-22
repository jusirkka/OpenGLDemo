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

#include <QApplication>
#include <QtPlugin>
#include <QSurfaceFormat>

#include "logging.h"
#include "mainwindow.h"

Q_IMPORT_PLUGIN(ImageStore)
Q_IMPORT_PLUGIN(ModelStore)

Q_LOGGING_CATEGORY(OGL, "OpenGLDemo")


// -------------------------------------------------------
// main
// -------------------------------------------------------


int main(int argc, char *argv[]) {

    QApplication app(argc, argv);
    app.setOrganizationName("Kvanttiapina");
    app.setApplicationName("OpenGLDemos");

    QString demo("");
    QStringList args = QApplication::arguments();
    if (args.size() > 1) {
        demo = args.at(1);
    }

    qRegisterMetaType<Math3D::Vector4>();
    qRegisterMetaType<Math3D::Matrix4>();
    QMetaType::registerDebugStreamOperator<Math3D::Matrix4>();
    QMetaType::registerDebugStreamOperator<Math3D::Vector4>();

    QSurfaceFormat format;
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    qSetMessagePattern("[%{category} "
                       "%{if-debug}D%{endif}"
                       "%{if-info}I%{endif}"
                       "%{if-warning}W%{endif}"
                       "%{if-critical}C%{endif}"
                       "%{if-fatal}F%{endif}]"
                       "[%{file}:%{line}] - %{message}");
    QLoggingCategory::setFilterRules(QStringLiteral("OpenGLDemo.debug=true"));

    Demo::MainWindow mw(demo);
    mw.show();


    return app.exec();
}
