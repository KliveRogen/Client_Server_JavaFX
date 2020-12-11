#-------------------------------------------------
#
# Project created by QtCreator 2019-08-14T13:50:51
#
#-------------------------------------------------

QT       -= gui

win32:CONFIG(release, debug|release): TARGET = Port
else:win32:CONFIG(debug, debug|release): TARGET = Portd
TEMPLATE = lib

DEFINES += PORT_LIBRARY

SOURCES += \
    InputPort.cpp \
    OutputPort.cpp

HEADERS +=\
        port_global.h \
    ../include/IPort.h \
    InputPort.h \
    OutputPort.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32:CONFIG(release, debug|release): DESTDIR = $$PWD/../../target/release/calcLibrary
else:win32:CONFIG(debug, debug|release): DESTDIR = $$PWD/../../target/debug/calcLibrary
