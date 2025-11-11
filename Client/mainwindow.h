#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include<QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &username, QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString currentUsername;
};

#endif // MAINWINDOW_H
