QT += core network widgets
CONFIG += c++17

SOURCES += \
    main.cpp \
    clientnetworkmanager.cpp \
    testclient.cpp \
    registerdialog.cpp  # 新增注册对话框

HEADERS += \
    clientnetworkmanager.h \
    testclient.h \
    registerdialog.h \  # 新增注册对话框
    ../Common/protocol.h

FORMS += \
    testclient.ui
    # 注意：registerdialog没有.ui文件，因为我们用代码创建UI
