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
    runner.cpp

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
    grammar.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    scanner.l \
    grammar.y


DEFINES += YYERROR_VERBOSE

jBISONSOURCES = grammar.y


jbison.commands = yacc -t -v ${QMAKE_FILE_IN} && mv y.tab.c ${QMAKE_FILE_BASE}.cpp
jbison.input = jBISONSOURCES
jbison.output = ${QMAKE_FILE_BASE}.cpp
jbison.variable_out = SOURCES

QMAKE_EXTRA_COMPILERS += jbison

jLEXSOURCES = scanner.l

jlexsource.input = jLEXSOURCES
jlexsource.output = ${QMAKE_FILE_BASE}.c
jlexsource.commands = lex -t ${QMAKE_FILE_IN} > ${QMAKE_FILE_BASE}.c
jlexsource.variable_out = SOURCES
jlexsource.CONFIG += target_predeps

jlexheader.input = jLEXSOURCES
jlexheader.output = ${QMAKE_FILE_BASE}.h
jlexheader.commands = lex -t ${QMAKE_FILE_IN} > /dev/null
jlexheader.variable_out = HEADERS
jlexheader.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += jlexheader
QMAKE_EXTRA_COMPILERS += jlexsource

