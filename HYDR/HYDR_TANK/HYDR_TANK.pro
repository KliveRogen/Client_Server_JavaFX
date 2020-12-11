#-------------------------------------------------
#
# Project created by CalcElementProjectGenerator
#
#-------------------------------------------------

QT       -= gui

win32:CONFIG(release, debug|release): TARGET = HYDR_TANK
else:win32:CONFIG(debug, debug|release): TARGET = HYDR_TANKd
TEMPLATE = lib

SOURCES += HYDR_TANK.cpp

HEADERS += HYDR_TANK.h \
    ../../include/IPort.h \
    ../../CalcElement/CalcElement.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../target/release/calcLibrary -lPort
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../target/debug/calcLibrary -lPortd

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../target/release/calcLibrary -lCalcElement
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../target/debug/calcLibrary -lCalcElementd

win32:CONFIG(release, debug|release): DESTDIR = $$PWD/../../../target/release/calcLibrary
else:win32:CONFIG(debug, debug|release): DESTDIR = $$PWD/../../../target/debug/calcLibrary
