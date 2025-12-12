#include "changepassworddialog.h"
#include "clientnetworkmanager.h"
#include "../Common/protocol.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug>
#include <QCloseEvent>
#include <QFontMetrics>

ChangePasswordDialog::ChangePasswordDialog(ClientNetworkManager* networkManager,
                                           const QString& username,
                                           QWidget *parent)
    : QDialog(parent),
    networkManager(networkManager),
    username(username),
    titleLabel(nullptr),
    oldPasswordLabel(nullptr),
    newPasswordLabel(nullptr),
    confirmPasswordLabel(nullptr),
    errorLabel(nullptr),
    oldPasswordEdit(nullptr),
    newPasswordEdit(nullptr),
    confirmPasswordEdit(nullptr),
    confirmButton(nullptr),
    cancelButton(nullptr)
{
    // 设置窗口属性
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);
    setWindowTitle("修改密码");
    setFixedSize(450, 400);

    setupUI();
    applyBeautifyStyles();

    // 连接网络消息信号
    if (networkManager) {
        connect(networkManager, &ClientNetworkManager::messageReceived,
                this, &ChangePasswordDialog::onMessageReceived);
    }

    // 连接按钮信号
    connect(confirmButton, &QPushButton::clicked, this, &ChangePasswordDialog::onConfirmClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ChangePasswordDialog::onCancelClicked);
}

ChangePasswordDialog::~ChangePasswordDialog()
{
    // Qt对象会自动管理内存
}

void ChangePasswordDialog::setupUI()
{
    // 设置主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 创建组框
    QGroupBox *groupBox = new QGroupBox("修改密码", this);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);
    groupLayout->setSpacing(15);

    // 标题
    titleLabel = new QLabel("🔒 修改密码", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    groupLayout->addWidget(titleLabel);

    // 原密码输入
    QHBoxLayout *oldPasswordLayout = new QHBoxLayout();
    oldPasswordLayout->setSpacing(10);

    oldPasswordLabel = new QLabel("原密码:", this);
    oldPasswordLabel->setMinimumWidth(80);

    oldPasswordEdit = new QLineEdit(this);
    oldPasswordEdit->setEchoMode(QLineEdit::Password);
    oldPasswordEdit->setPlaceholderText("请输入原密码");

    oldPasswordLayout->addWidget(oldPasswordLabel);
    oldPasswordLayout->addWidget(oldPasswordEdit);
    groupLayout->addLayout(oldPasswordLayout);

    // 新密码输入
    QHBoxLayout *newPasswordLayout = new QHBoxLayout();
    newPasswordLayout->setSpacing(10);

    newPasswordLabel = new QLabel("新密码:", this);
    newPasswordLabel->setMinimumWidth(80);

    newPasswordEdit = new QLineEdit(this);
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setPlaceholderText("请输入新密码（6-20位字符）");

    newPasswordLayout->addWidget(newPasswordLabel);
    newPasswordLayout->addWidget(newPasswordEdit);
    groupLayout->addLayout(newPasswordLayout);

    // 确认新密码输入
    QHBoxLayout *confirmPasswordLayout = new QHBoxLayout();
    confirmPasswordLayout->setSpacing(10);

    confirmPasswordLabel = new QLabel("确 认:", this);
    confirmPasswordLabel->setMinimumWidth(80);

    confirmPasswordEdit = new QLineEdit(this);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("请再次输入新密码");

    confirmPasswordLayout->addWidget(confirmPasswordLabel);
    confirmPasswordLayout->addWidget(confirmPasswordEdit);
    groupLayout->addLayout(confirmPasswordLayout);

    // 错误信息标签
    errorLabel = new QLabel("", this);
    errorLabel->setMinimumHeight(30);
    groupLayout->addWidget(errorLabel);

    // 添加弹性空间
    groupLayout->addStretch();

    // 按钮行
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20);

    cancelButton = new QPushButton("取消", this);
    confirmButton = new QPushButton("确认修改", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(confirmButton);
    buttonLayout->addStretch();

    groupLayout->addLayout(buttonLayout);

    mainLayout->addWidget(groupBox);
}

void ChangePasswordDialog::applyBeautifyStyles()
{
    // 设置窗口背景为黄蓝色渐变，与注册界面一致
    this->setStyleSheet(
        "QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #FFD700, stop:0.5 #87CEEB, stop:1 #1E90FF);"
        "  border-radius: 15px;"
        "}"
        );

    // GroupBox样式
    QList<QGroupBox*> groupBoxes = findChildren<QGroupBox*>();
    for (QGroupBox* groupBox : groupBoxes) {
        groupBox->setStyleSheet(
            "QGroupBox {"
            "  background-color: rgba(255, 255, 255, 0.95);"
            "  border-radius: 12px;"
            "  border: 2px solid rgba(255, 255, 255, 0.8);"
            "  padding: 20px;"
            "  font-size: 14px;"
            "}"
            "QGroupBox::title {"
            "  subcontrol-origin: margin;"
            "  subcontrol-position: top center;"
            "  padding: 6px 18px;"
            "  background-color: #FFA500;"
            "  color: white;"
            "  border-radius: 8px;"
            "  font-size: 16px;"
            "  font-weight: bold;"
            "}"
            );
    }

    // Label样式
    if (titleLabel) {
        titleLabel->setStyleSheet(
            "QLabel {"
            "  color: #1e3a5f;"
            "  font-size: 18px;"
            "  font-weight: bold;"
            "  padding: 10px 0px;"
            "}"
            );
    }

    // 其他标签样式
    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label != titleLabel && label != errorLabel) {
            label->setStyleSheet(
                "QLabel {"
                "  color: #1e3a5f;"
                "  font-size: 14px;"
                "  font-weight: bold;"
                "}"
                );
        }
    }

    // 错误标签样式
    if (errorLabel) {
        errorLabel->setStyleSheet(
            "QLabel {"
            "  color: #e74c3c;"
            "  font-size: 12px;"
            "  font-weight: normal;"
            "}"
            );
    }

    // LineEdit样式
    QList<QLineEdit*> lineEdits = findChildren<QLineEdit*>();
    for (QLineEdit* edit : lineEdits) {
        edit->setStyleSheet(
            "QLineEdit {"
            "  background-color: white;"
            "  border: 2px solid #bdc3c7;"
            "  border-radius: 6px;"
            "  padding: 8px 12px;"
            "  font-size: 14px;"
            "  color: #2c3e50;"
            "  min-height: 35px;"
            "}"
            "QLineEdit:focus {"
            "  border-color: #FFD700;"
            "  background-color: #fffaf0;"
            "}"
            );
    }

    // 按钮样式
    if (confirmButton) {
        confirmButton->setStyleSheet(
            "QPushButton {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FFD700, stop:1 #FFA500);"
            "  color: white;"
            "  border: none;"
            "  border-radius: 6px;"
            "  padding: 10px 15px;"
            "  font-size: 14px;"
            "  font-weight: bold;"
            "  min-width: 100px;"
            "  min-height: 35px;"
            "}"
            "QPushButton:hover {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FFA500, stop:1 #FF8C00);"
            "}"
            "QPushButton:pressed {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FF8C00, stop:1 #FF7F50);"
            "}"
            "QPushButton:disabled {"
            "  background: #bdc3c7;"
            "  color: #7f8c8d;"
            "}"
            );
    }

    if (cancelButton) {
        cancelButton->setStyleSheet(
            "QPushButton {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1E90FF, stop:1 #4169E1);"
            "  color: white;"
            "  border: none;"
            "  border-radius: 6px;"
            "  padding: 10px 15px;"
            "  font-size: 14px;"
            "  font-weight: bold;"
            "  min-width: 100px;"
            "  min-height: 35px;"
            "}"
            "QPushButton:hover {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4169E1, stop:1 #0000CD);"
            "}"
            "QPushButton:pressed {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0000CD, stop:1 #191970);"
            "}"
            );
    }
}

bool ChangePasswordDialog::validatePasswords()
{
    QString oldPassword = oldPasswordEdit->text().trimmed();
    QString newPassword = newPasswordEdit->text().trimmed();
    QString confirmPassword = confirmPasswordEdit->text().trimmed();

    if (oldPassword.isEmpty()) {
        errorLabel->setText("请输入原密码");
        oldPasswordEdit->setFocus();
        return false;
    }

    if (newPassword.isEmpty()) {
        errorLabel->setText("请输入新密码");
        newPasswordEdit->setFocus();
        return false;
    }

    if (newPassword.length() < 6 || newPassword.length() > 20) {
        errorLabel->setText("密码长度应在6-20个字符之间");
        newPasswordEdit->setFocus();
        newPasswordEdit->selectAll();
        return false;
    }

    if (confirmPassword.isEmpty()) {
        errorLabel->setText("请再次输入新密码");
        confirmPasswordEdit->setFocus();
        return false;
    }

    if (newPassword != confirmPassword) {
        errorLabel->setText("两次输入的新密码不一致");
        confirmPasswordEdit->setFocus();
        confirmPasswordEdit->selectAll();
        return false;
    }

    if (oldPassword == newPassword) {
        errorLabel->setText("新密码不能与原密码相同");
        newPasswordEdit->setFocus();
        newPasswordEdit->selectAll();
        return false;
    }

    errorLabel->clear();
    return true;
}

void ChangePasswordDialog::onConfirmClicked()
{
    if (!validatePasswords()) {
        return;
    }

    if (!networkManager) {
        errorLabel->setText("网络管理器未初始化");
        return;
    }

    if (!networkManager->isConnected()) {
        errorLabel->setText("未连接到服务器");
        return;
    }

    QString oldPassword = oldPasswordEdit->text().trimmed();
    QString newPassword = newPasswordEdit->text().trimmed();

    NetworkMessage changeMsg;
    changeMsg.type = CHANGE_PASSWORD_REQUEST;
    changeMsg.data["username"] = username;
    changeMsg.data["old_password"] = oldPassword;
    changeMsg.data["new_password"] = newPassword;

    networkManager->sendMessage(changeMsg);

    // 禁用确认按钮，防止重复点击
    confirmButton->setEnabled(false);
    errorLabel->setText("正在发送修改请求...");
    qDebug() << "发送修改密码请求:" << username;
}

void ChangePasswordDialog::onCancelClicked()
{
    close();
}

void ChangePasswordDialog::onMessageReceived(const NetworkMessage &message)
{
    if (message.type == CHANGE_PASSWORD_RESPONSE) {
        confirmButton->setEnabled(true); // 重新启用按钮

        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            QMessageBox::information(this, "修改成功", "密码已成功修改！");
            accept();
        } else {
            errorLabel->setText(resultMsg);
            QMessageBox::warning(this, "修改失败", resultMsg);
        }
    }
}

void ChangePasswordDialog::closeEvent(QCloseEvent *event)
{
    // 清理输入框
    oldPasswordEdit->clear();
    newPasswordEdit->clear();
    confirmPasswordEdit->clear();
    errorLabel->clear();

    // 重置按钮状态
    confirmButton->setEnabled(true);

    // 接受关闭事件
    event->accept();
}
