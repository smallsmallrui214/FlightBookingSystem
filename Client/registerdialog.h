#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "../Common/protocol.h"

// 前向声明
class QLineEdit;
class QPushButton;
class ClientNetworkManager;

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(ClientNetworkManager* networkManager, QWidget *parent = nullptr);
    ~RegisterDialog();

signals:
    void registrationSuccess();
    void finished(int result);

private slots:
    void onRegisterClicked();
    void onCancelClicked();
    void onMessageReceived(const NetworkMessage &message);

private:
    void setupUI();
    void applyBeautifyStyles();

    // 保存待注册信息的结构体
    struct PendingRegistration {
        QString username;
        QString password;
        QString email;
        bool checked = false;
    };

    PendingRegistration pendingRegistration;  // 新增成员变量

    ClientNetworkManager* networkManager;

    // UI控件
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
    QLineEdit *emailEdit;
    QPushButton *registerButton;
    QPushButton *cancelButton;
};

#endif // REGISTERDIALOG_H
