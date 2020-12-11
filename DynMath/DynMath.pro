#-------------------------------------------------
#
# Project created by QtCreator 2020-01-16T21:46:34
#
#-------------------------------------------------

QT       -= gui

win32:CONFIG(release, debug|release): {
    TARGET = DynMath
    DESTDIR = $$PWD/../../target/release/calcLibrary
}
else:win32:CONFIG(debug, debug|release): {
    TARGET = DynMathd
    DESTDIR = $$PWD/../../target/debug/calcLibrary
}

TEMPLATE = lib

DEFINES += DYNMATH_LIBRARY

SOURCES += \
    Integrator.cpp \
    InertOne.cpp \
    Delay.cpp \
    InertOneDelay.cpp

HEADERS +=\
        dynmath_global.h \
    Integrator.h \
    InertOne.h \
    Delay.h \
    InertOneDelay.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
