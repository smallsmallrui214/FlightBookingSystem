#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QTimer>
#include <QPropertyAnimation>
#include <QMovie>

class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplashScreen(QWidget *parent = nullptr);
    ~SplashScreen();

    void showWithAnimation();

signals:
    void animationFinished();

private slots:
    void updateProgress();
    void fadeOut();

private:
    void setupUI();
    void applyStyles();

    // 基本类型先声明
    int progressValue;

    // 指针类型按照初始化顺序声明
    QMovie *gifMovie;
    QLabel *logoLabel;
    QLabel *titleLabel;
    QLabel *versionLabel;
    QProgressBar *progressBar;
    QVBoxLayout *mainLayout;
    QTimer *progressTimer;
    QPropertyAnimation *fadeAnimation;
};

#endif // SPLASHSCREEN_H
