#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QVBoxLayout>

// 如果不想用UI文件，可以用代码创建界面
MainWindow::MainWindow(const QString &username, QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentUsername(username)
{
    ui->setupUi(this);

    // 设置窗口属性
    setWindowTitle("航班管理系统 - 主界面");
    resize(1000, 700);

    // 更新UI中的欢迎信息
    ui->welcomeLabel->setText(QString("欢迎, %1!").arg(username));

}

MainWindow::~MainWindow()
{
    if (ui) {
        delete ui;
    }
}
