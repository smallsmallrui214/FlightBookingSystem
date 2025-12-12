#ifndef CHANGEUSERNAMEDIALOG_H
#define CHANGEUSERNAMEDIALOG_H

#include <QDialog>
#include "../Common/protocol.h"

// 前向声明
class QLineEdit;
class QPushButton;
class QLabel;
class ClientNetworkManager;

class ChangeUsernameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeUsernameDialog(ClientNetworkManager* networkManager,
                                  const QString& currentUsername,
                                  QWidget *parent = nullptr);
    ~ChangeUsernameDialog();

signals:
    void usernameChanged(const QString& newUsername);

private slots:
    void onConfirmClicked();
    void onCancelClicked();
    void onMessageReceived(const NetworkMessage &message);

private:
    void setupUI();
    void applyBeautifyStyles();
    bool validateUsername(const QString& username);

protected:
    void closeEvent(QCloseEvent *event) override;

    ClientNetworkManager* networkManager;
    QString currentUsername;

    // UI控件
    QLabel *titleLabel;
    QLabel *currentUsernameLabel;
    QLabel *newUsernameLabel;
    QLabel *errorLabel;
    QLineEdit *newUsernameEdit;
    QPushButton *confirmButton;
    QPushButton *cancelButton;

    // 待检查的用户名
    QString pendingNewUsername;
    bool isCheckingUsername = false;
};

#endif // CHANGEUSERNAMEDIALOG_H
