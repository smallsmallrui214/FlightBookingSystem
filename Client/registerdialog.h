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
    void checkUsernameAvailability();  // 检查用户名是否可用

private:
    void setupUI();
    void validateInputs();  // 输入验证
    void sendRegisterRequest();  // 发送注册请求

    // UI控件
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
    QLineEdit *emailEdit;
    QPushButton *registerButton;
    QPushButton *cancelButton;

    // 状态变量
    QString pendingUsername;  // 待检查的用户名
    bool isUsernameAvailable; // 用户名是否可用

    ClientNetworkManager* networkManager;
};

#endif // REGISTERDIALOG_H
