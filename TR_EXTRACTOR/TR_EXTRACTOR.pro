#-------------------------------------------------
#
# Project created by QtCreator 2020-01-21T12:39:37
#
#-------------------------------------------------

QT       -= gui

win32:CONFIG(release, debug|release): {
    TARGET = TR_EXTRACTOR
    DESTDIR = $$PWD/../libs/release

    LIBS += -L$$PWD/../../target/release/calcLibrary -lPort
    LIBS += -L$$PWD/../../target/release/calcLibrary -lCalcElement
    LIBS += -L$$PWD/../../target/release/calcLibrary -lDynMath
}
else:win32:CONFIG(debug, debug|release): {
    TARGET = TR_EXTRACTORd
    DESTDIR = $$PWD/../../target/debug/calcLibrary

    LIBS += -L$$PWD/../../target/debug/calcLibrary -lPortd
    LIBS += -L$$PWD/../../target/debug/calcLibrary -lCalcElementd
    LIBS += -L$$PWD/../../target/debug/calcLibrary -lDynMathd
}

TEMPLATE = lib

DEFINES += TR_EXTRACTOR_LIBRARY

SOURCES += TR_EXTRACTOR.cpp \
    Valve.cpp \
    Pump.cpp \
    #Receiver.cpp \
    Tank.cpp

HEADERS += TR_EXTRACTOR.h\
    ../include/IPort.h \
    ../CalcElement/CalcElement.h \
    Valve.h \
    Pump.h \
    ../DynMath/Delay.h \
    ../DynMath/InertOne.h \
    ../DynMath/Integrator.h \
    #Receiver.h \
    Tank.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
