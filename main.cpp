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
#include <QDebug>
#include <QtPlugin>
#include <QDebug>
#include <QSurfaceFormat>

#include "mainwindow.h"

Q_IMPORT_PLUGIN(ImageStore)
Q_IMPORT_PLUGIN(ModelStore)
Q_IMPORT_PLUGIN(ShadowMap)



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

//    QSurfaceFormat format;
//    format.setVersion(3, 0);
//    format.setProfile(QSurfaceFormat::CoreProfile);
//    QSurfaceFormat::setDefaultFormat(format);

    Demo::MainWindow mw(demo);
    mw.show();


    return app.exec();
}
