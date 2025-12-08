QT += core network widgets gui
CONFIG += c++17
CONFIG += c++11

INCLUDEPATH += ../Common

SOURCES += \
    calendardialog.cpp \
    main.cpp \
    clientnetworkmanager.cpp \
    testclient.cpp \
    registerdialog.cpp \
    mainwindow.cpp \
    SplashScreen.cpp \
    flightdetaildialog.cpp \
    bookingdialog.cpp \
    walletdialog.cpp \  # 新增
    ../Common/flight.cpp \
    ../Common/cabin.cpp \
    ../Common/booking.cpp \
    ../Common/wallet.cpp  # 新增

HEADERS += \
    calendardialog.h \
    clientnetworkmanager.h \
    testclient.h \
    registerdialog.h \
    mainwindow.h \
    SplashScreen.h \
    flightdetaildialog.h \
    bookingdialog.h \
    walletdialog.h \  # 新增
    ../Common/protocol.h \
    ../Common/flight.h \
    ../Common/cabin.h \
    ../Common/booking.h \
    ../Common/wallet.h  # 新增

FORMS += \
    testclient.ui \
    mainwindow.ui \
    flightdetaildialog.ui \
    bookingdialog.ui \
    walletdialog.ui  # 新增
