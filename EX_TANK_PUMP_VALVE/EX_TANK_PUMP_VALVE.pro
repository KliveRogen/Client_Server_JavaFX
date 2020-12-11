#-------------------------------------------------
#
# Project created by CalcElementProjectGenerator
#
#-------------------------------------------------

QT       -= gui

win32:CONFIG(release, debug|release): {
    TARGET = EX_TANK_PUMP_VALVE
    DESTDIR = $$PWD/../libs/release

    LIBS += -L$$PWD/../libs/release -lPort
    LIBS += -L$$PWD/../libs/release -lCalcElement
    LIBS += -L$$PWD/../libs/release -lDynMath
}
else:win32:CONFIG(debug, debug|release): {
    TARGET = EX_TANK_PUMP_VALVEd
    DESTDIR = $$PWD/../libs/debug

    LIBS += -L$$PWD/../libs/debug -lPortd
    LIBS += -L$$PWD/../libs/debug -lCalcElementd
    LIBS += -L$$PWD/../libs/debug -lDynMathd
}

TEMPLATE = lib

SOURCES += EX_TANK_PUMP_VALVE.cpp

HEADERS += EX_TANK_PUMP_VALVE.h \
    ../include/IPort.h \
    ../CalcElement/CalcElement.h \
    ../DynMath/InertOne.h \
    ../DynMath/Integrator.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
