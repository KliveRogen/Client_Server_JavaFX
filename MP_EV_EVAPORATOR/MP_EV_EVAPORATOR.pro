#-------------------------------------------------
#
# Project created by CalcElementProjectGenerator
#
#-------------------------------------------------

QT       -= gui

win32:CONFIG(release, debug|release): TARGET = MP_EV_EVAPORATOR
else:win32:CONFIG(debug, debug|release): TARGET = MP_EV_EVAPORATORd
TEMPLATE = lib

SOURCES += MP_EV_EVAPORATOR.cpp

HEADERS += MP_EV_EVAPORATOR.h \
    ../include/IPort.h \
    ../CalcElement/CalcElement.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../target/release/calcLibrary -lPort
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../target/debug/calcLibrary -lPortd

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../target/release/calcLibrary -lCalcElement
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../target/debug/calcLibrary -lCalcElementd

win32:CONFIG(release, debug|release): DESTDIR = $$PWD/../../target/release/calcLibrary
else:win32:CONFIG(debug, debug|release): DESTDIR = $$PWD/../../target/debug/calcLibrary
