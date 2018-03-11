#-------------------------------------------------
#
# Project created by QtCreator 2013-10-09T18:26:49
#
#-------------------------------------------------

CONFIG += c++14

QT += core gui opengl widgets network

TARGET = OpenGLDemo
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    gl_widget.cpp \
    gl_lang_compiler.cpp \
    gl_lang_runner.cpp \
    camera.cpp \
    project.cpp \
    codeeditor.cpp \
    highlight.cpp \
    newdialog.cpp \
    imagestore.cpp \
    modelstore.cpp \
    fpscontrol.cpp \
    scope.cpp \
    textfilestore.cpp \
    depthzoom.cpp \
    gl_lang_completer.cpp \
    gl_lang_parser_interface.cpp \
    triangleoptimizer.cpp \
    patcher.cpp \
    bezierpatcher.cpp \
    statement.cpp \
    value.cpp \
    type.cpp \
    projectfolder.cpp \
    texturestore.cpp \
    downloader.cpp \
    videoencoder.cpp \
    datasource.cpp

HEADERS  += mainwindow.h \
    math3d.h \
    gl_functions.h \
    constant.h \
    function.h \
    symbol.h \
    variable.h \
    gl_widget.h \
    gl_lang_compiler.h \
    gl_lang_runner.h \
    blob.h \
    camera.h \
    project.h \
    codeeditor.h \
    highlight.h \
    newdialog.h \
    texblob.h \
    imagestore.h \
    modelstore.h \
    fpscontrol.h \
    scope.h \
    textfilestore.h \
    depthzoom.h \
    gl_lang_completer.h \
    gl_lang_parser_interface.h \
    triangleoptimizer.h \
    patcher.h \
    bezierpatcher.h \
    statement.h \
    operation.h \
    type.h \
    typedef.h \
    value.h \
    projectfolder.h \
    texturestore.h \
    downloader.h \
    videoencoder.h \
    logging.h \
    datasource.h

FORMS    += mainwindow.ui \
    newdialog.ui \
    fpscontrol.ui \
    depthzoom.ui

OTHER_FILES += \
    TODO.txt \
    wavefront_parser.y \
    wavefront_scanner.l \
    gl_lang_parser.y \
    gl_lang_scanner.l


DEFINES += YYERROR_VERBOSE QT_STATICPLUGIN


MY_BISON_SOURCES = wavefront_parser.y gl_lang_parser.y

my_bison_src.commands = bison ${QMAKE_FILE_IN} -o ${QMAKE_FILE_BASE}.cpp
my_bison_src.input = MY_BISON_SOURCES
my_bison_src.output = ${QMAKE_FILE_BASE}.cpp
my_bison_src.variable_out = SOURCES
my_bison_src.CONFIG += target_predeps

my_bison_hdr.commands = bison --defines=${QMAKE_FILE_BASE}.h ${QMAKE_FILE_IN} && rm ${QMAKE_FILE_BASE}.tab.c
my_bison_hdr.input = MY_BISON_SOURCES
my_bison_hdr.output = ${QMAKE_FILE_BASE}.h
my_bison_hdr.variable_out = HEADERS
my_bison_hdr.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += my_bison_src
QMAKE_EXTRA_COMPILERS += my_bison_hdr

MY_FLEX_SOURCES = wavefront_scanner.l gl_lang_scanner.l

my_flex_src.commands = flex -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
my_flex_src.input = MY_FLEX_SOURCES
my_flex_src.output = ${QMAKE_FILE_BASE}.cpp
my_flex_src.variable_out = SOURCES
my_flex_src.CONFIG += target_predeps

my_flex_hdr.commands = flex --header-file=${QMAKE_FILE_BASE}.h -t ${QMAKE_FILE_IN} > /dev/null
my_flex_hdr.input = MY_FLEX_SOURCES
my_flex_hdr.output = ${QMAKE_FILE_BASE}.h
my_flex_hdr.variable_out = HEADERS
my_flex_hdr.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += my_flex_src
QMAKE_EXTRA_COMPILERS += my_flex_hdr

LIBS += -lavcodec -lavutil -lswscale
