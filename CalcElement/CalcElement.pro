#-------------------------------------------------
#
# Project created by QtCreator 2019-08-14T14:28:45
#
#-------------------------------------------------

QT       -= gui
QT += xml

win32:CONFIG(release, debug|release): TARGET = CalcElement
else:win32:CONFIG(debug, debug|release): TARGET = CalcElementd
TEMPLATE = lib

DEFINES += CALCELEMENT_LIBRARY

SOURCES += CalcElement.cpp

HEADERS += CalcElement.h\
        calcelement_global.h \
    ../include/ICalcElement.h \
    ../include/IPort.h \
    ../Port/InputPort.h \
    ../Port/OutputPort.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}


win32:CONFIG(release, debug|release): {
    DESTDIR = $$PWD/../../target/release/calcLibrary

    LIBS += -L$$PWD/../../target/release/calcLibrary -lPort
    LIBS += -L$$PWD/../../target/release/ -lExtData
}
else:win32:CONFIG(debug, debug|release): {
    DESTDIR = $$PWD/../../target/debug/calcLibrary

    LIBS += -L$$PWD/../../target/debug/calcLibrary -lPortd
    LIBS += -L$$PWD/../../target/debug/ -lExtDatad

}
INCLUDEPATH += $$PWD/../../KTNIMFA/ExtData/
