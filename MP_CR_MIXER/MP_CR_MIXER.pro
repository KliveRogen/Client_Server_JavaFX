#-------------------------------------------------
#
# Project created by CalcElementProjectGenerator
#
#-------------------------------------------------

QT       -= gui

win32:CONFIG(release, debug|release): TARGET = MP_CR_MIXER
else:win32:CONFIG(debug, debug|release): TARGET = MP_CR_MIXERd
TEMPLATE = lib

SOURCES += MP_CR_MIXER.cpp

HEADERS += MP_CR_MIXER.h \
    ../include/IPort.h \
    ../CalcElement/CalcElement.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/release -lPort
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/debug -lPortd

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../libs/release -lCalcElement
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../libs/debug -lCalcElementd

win32:CONFIG(release, debug|release): DESTDIR = $$PWD/../libs/release
else:win32:CONFIG(debug, debug|release): DESTDIR = $$PWD/../libs/debug
