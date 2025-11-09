QT += core network widgets
CONFIG += c++17

SOURCES += \
    main.cpp \
    clientnetworkmanager.cpp \
    testclient.cpp

HEADERS += \
    clientnetworkmanager.h \
    testclient.h \
    ../Common/protocol.h

FORMS += \
    testclient.ui
