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
    resize(800, 600);

    // 创建简单的界面（不使用UI文件）
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 欢迎标签
    QLabel *welcomeLabel = new QLabel(QString("欢迎, %1!").arg(username), this);
    welcomeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin: 20px;");
    welcomeLabel->setAlignment(Qt::AlignCenter);

    // 提示标签
    QLabel *infoLabel = new QLabel("这是主界面，后续功能将在这里实现", this);
    infoLabel->setStyleSheet("font-size: 16px; color: #7f8c8d; margin: 10px;");
    infoLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(welcomeLabel);
    mainLayout->addWidget(infoLabel);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

MainWindow::~MainWindow()
{
    if (ui) {
        delete ui;
    }
}
