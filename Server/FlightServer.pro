QT += core network sql
CONFIG += c++17

DEFINES += QT_DEPRECATED_WARNINGS

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

# 添加编译选项
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS += -Wno-deprecated-declarations

# 启用RTTI
CONFIG += rtti
