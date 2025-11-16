QT += core network widgets
CONFIG += c++17

SOURCES += \
    main.cpp \
    clientnetworkmanager.cpp \
    testclient.cpp \
    registerdialog.cpp \
    mainwindow.cpp  # 新增

HEADERS += \
    clientnetworkmanager.h \
    testclient.h \
    registerdialog.h \
    mainwindow.h \  # 新增
    ../Common/protocol.h

FORMS += \
    testclient.ui \
    mainwindow.ui
    # 注意：mainwindow 和 registerdialog 都没有.ui文件

RESOURCES += \
    images.qrc
