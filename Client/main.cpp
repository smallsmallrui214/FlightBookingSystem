#include <QApplication>
#include "testclient.h"
#include "SplashScreen.h"
#include <QTimer>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 创建登录窗口（但不显示）
    TestClient *loginWindow = new TestClient();
    loginWindow->setWindowTitle("航班管理系统 - 登录");

    // 创建启动画面
    SplashScreen *splash = new SplashScreen();

    // 连接启动画面完成信号
    QObject::connect(splash, &SplashScreen::animationFinished, [=]() {
        qDebug() << "启动动画完成，显示登录窗口";

        // 显示登录窗口
        loginWindow->show();

        // 删除启动画面
        splash->deleteLater();
    });

    // 显示启动画面
    splash->showWithAnimation();

    return a.exec();
}
