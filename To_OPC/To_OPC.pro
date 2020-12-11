#-------------------------------------------------
#
# Project created by CalcElementProjectGenerator
#
#-------------------------------------------------

QT       -= gui

win32:CONFIG(release, debug|release): TARGET = To_OPC
else:win32:CONFIG(debug, debug|release): TARGET = To_OPCd
TEMPLATE = lib

SOURCES += To_OPC.cpp

HEADERS += To_OPC.h \
    ../include/IPort.h \
    ../CalcElement/CalcElement.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32:CONFIG(release, debug|release): {
    DESTDIR = $$PWD/../../target/release/calcLibrary

    LIBS += -L$$PWD/../../target/release/calcLibrary -lPort
    LIBS += -L$$PWD/../../target/release/calcLibrary -lCalcElement
}
else:win32:CONFIG(debug, debug|release):{
    DESTDIR = $$PWD/../../target/debug/calcLibrary

    LIBS += -L$$PWD/../../target/debug/calcLibrary -lPortd
    LIBS += -L$$PWD/../../target/debug/calcLibrary -lCalcElementd
}
