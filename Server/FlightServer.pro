QT += core network sql
CONFIG += c++17

SOURCES += \
    main.cpp \
    ServerNetworkManager.cpp \
    databasemanager.cpp \
    ../Common/flight.cpp \
    ../Common/cabin.cpp \
    ../Common/booking.cpp

HEADERS += \
    ServerNetworkManager.h \
    databasemanager.h \
    ../Common/protocol.h \
    ../Common/flight.h \
    ../Common/cabin.h \
    ../Common/booking.h
