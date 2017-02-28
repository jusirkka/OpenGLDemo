#-------------------------------------------------
#
# Project created by QtCreator 2013-10-09T18:26:49
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenGLDemo
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    parser.cpp \
    gl_widget.cpp \
    runner.cpp \
    teapot.cpp \
    camera.cpp \
    project.cpp \
    codeeditor.cpp \
    highlight.cpp \
    newdialog.cpp \
    imagestore.cpp \
    patch.cpp \
    modelstore.cpp \
    fpscontrol.cpp \
    scriptselector.cpp

HEADERS  += mainwindow.h \
    math3d.h \
    gl_functions.h \
    constant.h \
    function.h \
    parser.h \
    symbol.h \
    variable.h \
    gl_widget.h \
    runner.h \
    blob.h \
    teapot.h \
    camera.h \
    project.h \
    codeeditor.h \
    highlight.h \
    newdialog.h \
    texblob.h \
    imagestore.h \
    patch.h \
    modelstore.h \
    fpscontrol.h \
    wavefront_types.h \
    gl_lang_types.h \
    scriptselector.h

FORMS    += mainwindow.ui \
    newdialog.ui \
    fpscontrol.ui \
    scriptselector.ui

OTHER_FILES += \
    TODO.txt \
    wavefront.y \
    wavefront.l \
    gl_lang.l \
    gl_lang.y


DEFINES += YYERROR_VERBOSE QT_STATICPLUGIN

MY_YACC_SOURCES = wavefront.y gl_lang.y

my_yacc_source.commands = yacc -t ${QMAKE_FILE_IN} -p ${QMAKE_FILE_BASE}_ && mv y.tab.c ${QMAKE_FILE_BASE}.cpp
my_yacc_source.input = MY_YACC_SOURCES
my_yacc_source.output = ${QMAKE_FILE_BASE}.cpp
my_yacc_source.variable_out = SOURCES
my_yacc_source.CONFIG += target_predeps

my_yacc_header.commands = yacc -t -d ${QMAKE_FILE_IN}  -p ${QMAKE_FILE_BASE}_ && rm y.tab.c && mv y.tab.h ${QMAKE_FILE_BASE}.h
my_yacc_header.input = MY_YACC_SOURCES
my_yacc_header.output = ${QMAKE_FILE_BASE}.h
my_yacc_header.variable_out = HEADERS
my_yacc_header.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += my_yacc_source
QMAKE_EXTRA_COMPILERS += my_yacc_header

MY_LEX_SOURCES = wavefront.l gl_lang.l

my_lex_source.commands = flex -P${QMAKE_FILE_BASE}_ -t ${QMAKE_FILE_IN} > ${QMAKE_FILE_BASE}_scanner.c
my_lex_source.input = MY_LEX_SOURCES
my_lex_source.output = ${QMAKE_FILE_BASE}_scanner.c
my_lex_source.variable_out = SOURCES
my_lex_source.CONFIG += target_predeps

my_lex_header.commands = flex -P${QMAKE_FILE_BASE}_ --header-file=${QMAKE_FILE_BASE}_scanner.h -t ${QMAKE_FILE_IN} > /dev/null
my_lex_header.input = MY_LEX_SOURCES
my_lex_header.output = ${QMAKE_FILE_BASE}_scanner.h
my_lex_header.variable_out = HEADERS
my_lex_header.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += my_lex_source
QMAKE_EXTRA_COMPILERS += my_lex_header

