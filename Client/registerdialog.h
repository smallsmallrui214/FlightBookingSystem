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

private slots:
    void onRegisterClicked();
    void onCancelClicked();
    void onMessageReceived(const NetworkMessage &message);

private:
    void setupUI();

    // UI控件
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
    QLineEdit *emailEdit;
    QPushButton *registerButton;
    QPushButton *cancelButton;

    ClientNetworkManager* networkManager;
};

#endif // REGISTERDIALOG_H
