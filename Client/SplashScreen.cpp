#include "SplashScreen.h"
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QDebug>

SplashScreen::SplashScreen(QWidget *parent)
    : QWidget(parent),
    progressValue(0),      // 基本类型先初始化
    gifMovie(nullptr),     // 指针类型按照声明顺序初始化
    logoLabel(nullptr),
    titleLabel(nullptr),
    versionLabel(nullptr),
    progressBar(nullptr),
    mainLayout(nullptr),
    progressTimer(nullptr),
    fadeAnimation(nullptr)
{
    setupUI();
    applyStyles();

    // 设置窗口属性
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

SplashScreen::~SplashScreen()
{
    if (gifMovie) {
        gifMovie->stop();
        delete gifMovie;
    }
    if (progressTimer) {
        progressTimer->stop();
        delete progressTimer;
    }
    if (fadeAnimation) {
        fadeAnimation->stop();
        delete fadeAnimation;
    }
}

void SplashScreen::setupUI()
{
    // 设置固定大小
    setFixedSize(500, 400);

    // 创建主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 创建中心容器用于放置所有内容
    QWidget *centerWidget = new QWidget(this);
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setSpacing(8);
    centerLayout->setContentsMargins(0, 0, 0, 0);

    // 创建GIF加载动画标签
    logoLabel = new QLabel(this);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setFixedSize(180, 180);
    logoLabel->setStyleSheet("background: transparent;");

    // 加载GIF
    gifMovie = new QMovie("D:/develop/project/FlightBookingSystem/Client/loading.gif");

    if (gifMovie->isValid()) {
        gifMovie->setScaledSize(QSize(160, 160));
        logoLabel->setMovie(gifMovie);
        gifMovie->start();
        qDebug() << "GIF加载成功，尺寸:" << gifMovie->frameRect().size();
    } else {
        qDebug() << "GIF加载失败，路径:" << "C:/FlightBookingSystem/Client/loading.gif";
        logoLabel->setText("✈️");
        logoLabel->setStyleSheet("font-size: 90px; color: white; background: transparent;");
    }

    // 创建标题标签 - 移除不支持的text-shadow
    titleLabel = new QLabel("航班票务管理系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    color: white;"
        "    font-size: 28px;"
        "    font-weight: bold;"
        "    background: rgba(52, 152, 219, 0.4);"
        "    margin: 5px 40px;"
        "    padding: 12px 25px;"
        "    border-radius: 20px;"
        "    border: 2px solid rgba(255, 255, 255, 0.3);"
        "    letter-spacing: 2px;"
        "    font-family: 'Microsoft YaHei', 'SimHei', sans-serif;"
        "}"
        );

    // 创建版本标签
    versionLabel = new QLabel("Version 1.0.0", this);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(
        "QLabel {"
        "    color: #ecf0f1;"
        "    font-size: 13px;"
        "    background: rgba(149, 165, 166, 0.3);"
        "    margin: 2px 60px;"
        "    padding: 6px 15px;"
        "    border-radius: 12px;"
        "    border: 1px solid rgba(255, 255, 255, 0.2);"
        "    font-style: italic;"
        "}"
        );

    // 创建进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(false);
    progressBar->setFixedWidth(280);
    progressBar->setStyleSheet(
        "QProgressBar {"
        "    border: 2px solid rgba(255, 255, 255, 0.4);"
        "    border-radius: 10px;"
        "    text-align: center;"
        "    background: rgba(0, 0, 0, 0.3);"
        "    height: 14px;"
        "    margin: 8px 0px;"
        "}"
        "QProgressBar::chunk {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #3498db, stop:0.5 #9b59b6, stop:1 #e74c3c);"
        "    border-radius: 8px;"
        "}"
        );

    // 将内容添加到中心布局
    centerLayout->addStretch(1);
    centerLayout->addWidget(logoLabel, 0, Qt::AlignCenter);
    centerLayout->addSpacing(5);
    centerLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    centerLayout->addSpacing(3);
    centerLayout->addWidget(versionLabel, 0, Qt::AlignCenter);
    centerLayout->addSpacing(8);
    centerLayout->addWidget(progressBar, 0, Qt::AlignCenter);
    centerLayout->addStretch(1);

    centerWidget->setLayout(centerLayout);
    mainLayout->addWidget(centerWidget, 1);

    // 创建定时器和动画
    progressTimer = new QTimer(this);
    fadeAnimation = new QPropertyAnimation(this, "windowOpacity", this);

    connect(progressTimer, &QTimer::timeout, this, &SplashScreen::updateProgress);
    connect(fadeAnimation, &QPropertyAnimation::finished, this, &SplashScreen::animationFinished);
}

void SplashScreen::applyStyles()
{
    setStyleSheet(
        "SplashScreen {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "        stop:0 #2c3e50, stop:1 #34495e);"
        "    border-radius: 20px;"
        "    border: 3px solid rgba(255, 255, 255, 0.3);"
        "}"
        );
}

void SplashScreen::showWithAnimation()
{
    // 居中显示
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move(screenGeometry.center() - rect().center());

    // 初始透明度为0
    setWindowOpacity(0);
    show();

    // 淡入动画
    QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "windowOpacity", this);
    fadeIn->setDuration(1000);
    fadeIn->setStartValue(0);
    fadeIn->setEndValue(1);
    fadeIn->setEasingCurve(QEasingCurve::InOutQuad);
    fadeIn->start(QPropertyAnimation::DeleteWhenStopped);

    // 开始进度条动画
    progressTimer->start(40); // 每40ms更新一次进度，总时长约4秒
}

void SplashScreen::updateProgress()
{
    progressValue += 1; // 每次增加1%，让动画更平滑
    progressBar->setValue(progressValue);

    if (progressValue >= 100) {
        progressTimer->stop();
        fadeOut();
    }
}

void SplashScreen::fadeOut()
{
    qDebug() << "启动画面淡出开始";

    // 停止GIF动画
    if (gifMovie && gifMovie->isValid()) {
        gifMovie->stop();
    }

    fadeAnimation->setDuration(600);
    fadeAnimation->setStartValue(1);
    fadeAnimation->setEndValue(0);
    fadeAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    fadeAnimation->start();
}
