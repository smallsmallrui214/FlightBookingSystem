#ifndef CHANGEPASSWORDDIALOG_H
#define CHANGEPASSWORDDIALOG_H

#include <QDialog>

// 前向声明
class QLineEdit;
class QPushButton;
class QLabel;
class ClientNetworkManager;

struct NetworkMessage;  // 前向声明

class ChangePasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePasswordDialog(ClientNetworkManager* networkManager,
                                  const QString& username,
                                  QWidget *parent = nullptr);
    ~ChangePasswordDialog();

private slots:
    void onConfirmClicked();
    void onCancelClicked();
    void onMessageReceived(const NetworkMessage &message);

private:
    void setupUI();
    void applyBeautifyStyles();
    bool validatePasswords();

protected:
    void closeEvent(QCloseEvent *event) override;

    ClientNetworkManager* networkManager;
    QString username;

    // UI控件
    QLabel *titleLabel;
    QLabel *oldPasswordLabel;
    QLabel *newPasswordLabel;
    QLabel *confirmPasswordLabel;
    QLabel *errorLabel;
    QLineEdit *oldPasswordEdit;
    QLineEdit *newPasswordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *confirmButton;
    QPushButton *cancelButton;
};

#endif // CHANGEPASSWORDDIALOG_H
