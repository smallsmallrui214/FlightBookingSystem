QT += core network widgets gui
CONFIG += c++17

INCLUDEPATH += ../Common

SOURCES += \
    calendardialog.cpp \
    main.cpp \
    clientnetworkmanager.cpp \
    testclient.cpp \
    registerdialog.cpp \
    mainwindow.cpp \
    SplashScreen.cpp \
    ../Common/flight.cpp

HEADERS += \
    calendardialog.h \
    clientnetworkmanager.h \
    testclient.h \
    registerdialog.h \
    mainwindow.h \
    SplashScreen.h \
    ../Common/protocol.h \
    ../Common/flight.h

FORMS += \
    testclient.ui \
    mainwindow.ui
