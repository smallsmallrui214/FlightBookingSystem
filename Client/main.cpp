#include <QApplication>
#include "testclient.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TestClient loginWindow;
    loginWindow.setWindowTitle("航班管理系统 - 登录");
    loginWindow.resize(400, 300);
    loginWindow.show();

    return a.exec();
}
