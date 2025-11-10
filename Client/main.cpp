#include <QApplication>
#include "testclient.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TestClient client;
    client.setWindowTitle("航班客户端测试");
    client.resize(600, 500);
    client.show();

    return a.exec();
}
//?/？
