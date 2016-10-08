QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport serialport

RESOURCES = resource.qrc
CONFIG += c++11

TARGET = ArduinoScope
TEMPLATE = app

INCLUDEPATH += sensordatahandler

SOURCES += main.cpp \
           mainwindow.cpp \
           qcustomplot/qcustomplot.cpp \
           arduino/arduinoconnectiondlg.cpp \
           arduino/serialportreader.cpp \
           sensordatahandler/sensordatahandler.cpp

HEADERS  += mainwindow.h \
            qcustomplot/qcustomplot.h \
            arduino/arduinoconnectiondlg.h \
            arduino/serialportreader.h \
            sensordatahandler/sensordatahandler.h

FORMS    += mainwindow.ui \
            parameters.ui \
            arduino/arduinoconnectiondlg.ui
