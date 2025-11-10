QT += core network sql
CONFIG += c++17

SOURCES += \
    main.cpp \
    ServerNetworkManager.cpp \
    databasemanager.cpp

HEADERS += \
    ServerNetworkManager.h \
    databasemanager.h \
    ../Common/protocol.h
