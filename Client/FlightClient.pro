QT += core network widgets gui
CONFIG += c++17
CONFIG += c++11

INCLUDEPATH += ../Common

SOURCES += \
    calendardialog.cpp \
    changeUsernameDialog.cpp \
    changepassworddialog.cpp \
    main.cpp \
    clientnetworkmanager.cpp \
    rechargedialog.cpp \  # 新增
    testclient.cpp \
    registerdialog.cpp \
    mainwindow.cpp \
    SplashScreen.cpp \
    flightdetaildialog.cpp \
    bookingdialog.cpp \
    walletdialog.cpp \
    ../Common/flight.cpp \
    ../Common/cabin.cpp \
    ../Common/booking.cpp \
    ../Common/wallet.cpp

HEADERS += \
    calendardialog.h \
    changeUsernameDialog.h \
    changepassworddialog.h \
    clientnetworkmanager.h \
    rechargedialog.h \  # 新增
    testclient.h \
    registerdialog.h \
    mainwindow.h \
    SplashScreen.h \
    flightdetaildialog.h \
    bookingdialog.h \
    walletdialog.h \
    ../Common/protocol.h \
    ../Common/flight.h \
    ../Common/cabin.h \
    ../Common/booking.h \
    ../Common/wallet.h

FORMS += \
    testclient.ui \
    mainwindow.ui \
    flightdetaildialog.ui \
    bookingdialog.ui \
    walletdialog.ui
