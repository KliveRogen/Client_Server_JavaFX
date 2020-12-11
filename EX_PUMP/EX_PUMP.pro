#-------------------------------------------------
#
# Project created by CalcElementProjectGenerator
#
#-------------------------------------------------

QT       -= gui

win32:CONFIG(release, debug|release): {
    TARGET = EX_PUMP
    DESTDIR = $$PWD/../libs/release

    LIBS += -L$$PWD/../libs/release -lPort
    LIBS += -L$$PWD/../libs/release -lCalcElement
    LIBS += -L$$PWD/../libs/release -lDynMath
}
else:win32:CONFIG(debug, debug|release): {
    TARGET = EX_PUMPd
    DESTDIR = $$PWD/../libs/debug

    LIBS += -L$$PWD/../libs/debug -lPortd
    LIBS += -L$$PWD/../libs/debug -lCalcElementd
    LIBS += -L$$PWD/../libs/debug -lDynMathd
}

TEMPLATE = lib

SOURCES += EX_PUMP.cpp

HEADERS += EX_PUMP.h \
    ../include/IPort.h \
    ../CalcElement/CalcElement.h \
    ../DynMath/InertOne.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
