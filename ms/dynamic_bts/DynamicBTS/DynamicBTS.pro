#-------------------------------------------------
#
# Project created by QtCreator 2016-06-02T00:51:00
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DynamicBTS
TEMPLATE = app


SOURCES += main.cpp\
        gui.cpp \
    serialportselector.cpp \
    serialconnector.cpp \
    networkvisualization.cpp \
    networknode.cpp \
    networkedge.cpp \
    qgraphicsfixedview.cpp \
    networkassignmentswidget.cpp \
    networkstatstablewidget.cpp \
    latencylogwidget.cpp

HEADERS  += gui.h \
    serialportselector.h \
    serialconnector.h \
    networkvisualization.h \
    common.h \
    networknode.h \
    networkedge.h \
    qgraphicsfixedview.h \
    networkassignmentswidget.h \
    networkstatstablewidget.h \
    latencylogwidget.h

FORMS    += gui.ui \
    serialportselector.ui

RESOURCES += \
    qdarkstyle/style.qrc
