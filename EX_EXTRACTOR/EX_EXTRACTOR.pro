#-------------------------------------------------
#
# Project created by CalcElementProjectGenerator
#
#-------------------------------------------------

QT       -= gui

win32:CONFIG(release, debug|release): TARGET = EX_EXTRACTOR
else:win32:CONFIG(debug, debug|release): TARGET = EX_EXTRACTORd
TEMPLATE = lib

SOURCES += EX_EXTRACTOR.cpp

HEADERS += EX_EXTRACTOR.h \
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
