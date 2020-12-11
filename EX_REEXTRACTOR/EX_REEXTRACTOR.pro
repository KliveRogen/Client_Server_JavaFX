#-------------------------------------------------
#
# Project created by CalcElementProjectGenerator
#
#-------------------------------------------------

QT       -= gui

win32:CONFIG(release, debug|release): TARGET = EX_REEXTRACTOR
else:win32:CONFIG(debug, debug|release): TARGET = EX_REEXTRACTORd
TEMPLATE = lib

SOURCES += EX_REEXTRACTOR.cpp

HEADERS += EX_REEXTRACTOR.h \
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

DESTDIR = $$PWD/../libs
