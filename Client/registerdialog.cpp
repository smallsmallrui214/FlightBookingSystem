#include "registerdialog.h"
#include "clientnetworkmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QDebug>
#include <QEvent>
#include<QTimer>


class PlaceholderFilter : public QObject
{
public:
    PlaceholderFilter(QLineEdit *edit, const QString &text)
        : QObject(edit), lineEdit(edit), placeholder(text) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (obj == lineEdit) {
            if (event->type() == QEvent::FocusIn) {
                lineEdit->setPlaceholderText(""); // 获得焦点隐藏占位符
            } else if (event->type() == QEvent::FocusOut) {
                if (lineEdit->text().isEmpty())
                    lineEdit->setPlaceholderText(placeholder); // 失去焦点且为空显示占位符
            }
        }
        return false;
    }

private:
    QLineEdit *lineEdit;
    QString placeholder;
};


RegisterDialog::RegisterDialog(ClientNetworkManager* networkManager, QWidget *parent)
    : QDialog(parent),
    networkManager(networkManager),
    usernameEdit(nullptr),
    passwordEdit(nullptr),
    confirmPasswordEdit(nullptr),
    emailEdit(nullptr),
    registerButton(nullptr),
    cancelButton(nullptr)
// 移除 isUsernameAvailable 的初始化
{
    setupUI();
    applyBeautifyStyles();
    setWindowTitle("用户注册");
    setFixedSize(450, 350);

    // 初始化 pendingRegistration
    pendingRegistration.checked = false;

    // 连接网络消息信号
    connect(networkManager, &ClientNetworkManager::messageReceived,
            this, &RegisterDialog::onMessageReceived);

    // 移除实时用户名检查的连接
    // connect(usernameEdit, &QLineEdit::textChanged, this, &RegisterDialog::checkUsernameAvailability);

    connect(cancelButton, &QPushButton::clicked, this, &RegisterDialog::onCancelClicked);

    connect(this, &RegisterDialog::finished, this, [this](int) {
        // 清空所有输入框
        usernameEdit->clear();
        passwordEdit->clear();
        confirmPasswordEdit->clear();
        emailEdit->clear();

        // 重新启用注册按钮（如果之前被禁用）
        registerButton->setEnabled(true);

        // 重置 pendingRegistration
        pendingRegistration = PendingRegistration();
    });
}

RegisterDialog::~RegisterDialog()
{
    // 所有Qt对象会自动被父对象删除，不需要手动delete
}

void RegisterDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 创建组框
    QGroupBox *groupBox = new QGroupBox("用户注册", this);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);

    // 用户名行
    QHBoxLayout *usernameLayout = new QHBoxLayout();
    usernameLayout->addWidget(new QLabel("用户名:", this));
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("请输入用户名");
    usernameEdit->installEventFilter(new PlaceholderFilter(usernameEdit, "请输入用户名"));
    usernameLayout->addWidget(usernameEdit);
    groupLayout->addLayout(usernameLayout);

    // 密码行
    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordLayout->addWidget(new QLabel("密 码:", this));
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->installEventFilter(new PlaceholderFilter(passwordEdit, "请输入密码"));
    passwordLayout->addWidget(passwordEdit);
    groupLayout->addLayout(passwordLayout);

    // 确认密码行
    QHBoxLayout *confirmLayout = new QHBoxLayout();
    confirmLayout->addWidget(new QLabel("确 认:", this));
    confirmPasswordEdit = new QLineEdit(this);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("请再次输入密码");
    confirmPasswordEdit->installEventFilter(new PlaceholderFilter(confirmPasswordEdit, "请再次输入密码"));
    confirmLayout->addWidget(confirmPasswordEdit);
    groupLayout->addLayout(confirmLayout);

    // 邮箱行
    QHBoxLayout *emailLayout = new QHBoxLayout();
    emailLayout->addWidget(new QLabel("邮 箱:", this));
    emailEdit = new QLineEdit(this);
    emailEdit->setPlaceholderText("请输入邮箱(可选)");
    emailEdit->installEventFilter(new PlaceholderFilter(emailEdit, "请输入邮箱(可选)"));
    emailLayout->addWidget(emailEdit);
    groupLayout->addLayout(emailLayout);

    // 按钮行
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    cancelButton = new QPushButton("取消", this);
    registerButton = new QPushButton("注册", this);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(registerButton);
    groupLayout->addLayout(buttonLayout);

    mainLayout->addWidget(groupBox);

    // 连接信号槽
    connect(registerButton, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
    connect(cancelButton, &QPushButton::clicked, this, &RegisterDialog::onCancelClicked);
}
void RegisterDialog::applyBeautifyStyles()
{
    // 1. 设置窗口背景为天蓝色
    this->setStyleSheet(
        "QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #87CEEB, stop:1 #B0E2FF);"  // 天蓝色渐变
        "  border-radius: 15px;"
        "  border: 2px solid #4682B4;"
        "}"
        );

    // 2. 美化 GroupBox
    const QList<QGroupBox*> groupBoxes = findChildren<QGroupBox*>();
    for (QGroupBox* const &groupBox : groupBoxes) {
        groupBox->setStyleSheet(
            "QGroupBox {"
            "  background-color: rgba(255, 255, 255, 0.95);"
            "  border-radius: 10px;"
            "  border: 2px solid rgba(255, 255, 255, 0.8);"
            "  padding: 20px;"
            "  margin: 15px;"
            "  font-size: 14px;"
            "}"
            "QGroupBox::title {"
            "  subcontrol-origin: margin;"
            "  subcontrol-position: top center;"
            "  padding: 5px 15px;"
            "  background-color: #4682B4;"  // 钢蓝色
            "  color: white;"
            "  border-radius: 5px;"
            "  font-size: 16px;"
            "  font-weight: bold;"
            "}"
            );
    }

    // 3. 美化 Label
    const QList<QLabel*> labels = findChildren<QLabel*>();

    // 获取所有 label 文本的最大宽度
    int maxWidth = 0;
    QFontMetrics fm(this->font());
    for (QLabel* const &label : labels) {
        int w = fm.horizontalAdvance(label->text());
        maxWidth = std::max(maxWidth, w);
    }
    maxWidth += 20;

    // 为所有 label 统一设置宽度 + 右对齐
    for (QLabel* const &label : labels) {
        label->setMinimumWidth(maxWidth);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        label->setStyleSheet(
            "QLabel {"
            "  color: #2c3e50;"
            "  font-size: 14px;"
            "  font-weight: bold;"
            "  background: transparent;"
            "  padding: 5px 0px;"
            "}"
            );
    }

    // 4. 美化 LineEdit
    const QList<QLineEdit*> lineEdits = findChildren<QLineEdit*>();
    for (QLineEdit* const &edit : lineEdits) {
        edit->setFixedHeight(35);
        edit->setFixedWidth(200);

        edit->setStyleSheet(
            "QLineEdit {"
            "  background-color: white;"
            "  border: 2px solid #bdc3c7;"
            "  border-radius: 5px;"
            "  padding: 8px 12px;"
            "  font-size: 14px;"
            "  color: #2c3e50;"
            "  min-width: 200px;"
            "}"
            "QLineEdit:focus {"
            "  border-color: #4682B4;"  // 钢蓝色
            "  background-color: #f8f9fa;"
            "}"
            );
    }

    // 5. 美化按钮
    const QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* const &btn : buttons) {
        btn->setFixedHeight(35);
        btn->setFixedWidth(80);

        if (btn->text().contains("注册")) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background-color: #4682B4;"  // 钢蓝色
                "  color: white;"
                "  border: none;"
                "  border-radius: 5px;"
                "  padding: 10px 15px;"
                "  font-size: 14px;"
                "  font-weight: bold;"
                "  min-width: 80px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #5F9EA0;"  // 青蓝色
                "}"
                "QPushButton:pressed {"
                "  background-color: #36648B;"  // 深钢蓝色
                "}"
                );
        } else if (btn->text().contains("取消")) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background-color: #e74c3c;"
                "  color: white;"
                "  border: none;"
                "  border-radius: 5px;"
                "  padding: 10px 15px;"
                "  font-size: 14px;"
                "  font-weight: bold;"
                "  min-width: 80px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #ec7063;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #cb4335;"
                "}"
                );
        }
    }
}

void RegisterDialog::onRegisterClicked()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();
    QString confirmPassword = confirmPasswordEdit->text().trimmed();
    QString email = emailEdit->text().trimmed();

    // 检查用户名是否为空
    if (username.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名");
        usernameEdit->setFocus();
        return;
    }

    // 检查密码是否为空
    if (password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入密码");
        passwordEdit->setFocus();
        return;
    }

    // 检查用户名长度
    if (username.length() < 3 || username.length() > 20) {
        QMessageBox::warning(this, "输入错误", "用户名长度应在3-20个字符之间");
        usernameEdit->setFocus();
        usernameEdit->selectAll();
        return;
    }

    // 检查确认密码是否为空
    if (confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请再次输入密码");
        confirmPasswordEdit->setFocus();
        return;
    }

    // 检查密码长度
    if (password.length() < 6 || password.length() > 20) {
        QMessageBox::warning(this, "输入错误", "密码长度应在6-20个字符之间");
        passwordEdit->setFocus();
        passwordEdit->selectAll();
        return;
    }

    // 检查两次密码是否一致
    if (password != confirmPassword) {
        QMessageBox::warning(this, "输入错误", "两次输入的密码不一致");
        confirmPasswordEdit->setFocus();
        confirmPasswordEdit->selectAll();
        return;
    }

    if (!networkManager->isConnected()) {
        QMessageBox::warning(this, "连接错误", "未连接到服务器");
        return;
    }

    // 保存当前输入的信息
    pendingRegistration.username = username;
    pendingRegistration.password = password;
    pendingRegistration.email = email;
    pendingRegistration.checked = false;

    // 发送检查用户名请求（在注册前先检查）
    NetworkMessage checkMsg;
    checkMsg.type = CHECK_USERNAME_REQUEST;
    checkMsg.data["username"] = username;
    networkManager->sendMessage(checkMsg);

    // 禁用注册按钮，防止重复点击
    registerButton->setEnabled(false);
    qDebug() << "发送用户名检查请求:" << username;
}

void RegisterDialog::onCancelClicked()
{
    reject(); // 关闭对话框
}

void RegisterDialog::onMessageReceived(const NetworkMessage &message)
{
    // 处理用户名检查响应
    if (message.type == CHECK_USERNAME_RESPONSE) {
        bool exists = message.data["exists"].toBool();
        QString checkedUsername = message.data["username"].toString();

        // 确保检查的是当前待注册的用户名
        if (checkedUsername == pendingRegistration.username && !pendingRegistration.checked) {
            pendingRegistration.checked = true;

            if (exists) {
                // 用户名已存在
                usernameEdit->setStyleSheet("border: 2px solid #e74c3c;");
                QMessageBox::warning(this, "用户名不可用", "该用户名已被使用，请选择其他用户名");
                usernameEdit->setFocus();
                usernameEdit->selectAll();
                registerButton->setEnabled(true); // 重新启用注册按钮
            } else {
                // 用户名可用，继续注册流程
                usernameEdit->setStyleSheet("border: 2px solid #27ae60;");
                qDebug() << "用户名可用，继续注册:" << checkedUsername;

                // 发送注册请求
                NetworkMessage registerMsg;
                registerMsg.type = REGISTER_REQUEST;
                registerMsg.data["username"] = pendingRegistration.username;
                registerMsg.data["password"] = pendingRegistration.password;
                registerMsg.data["email"] = pendingRegistration.email;
                networkManager->sendMessage(registerMsg);

                qDebug() << "发送注册请求:" << pendingRegistration.username;
                // 注册按钮保持禁用状态，等待注册响应
            }
        }
    }

    // 处理注册响应
    if (message.type == REGISTER_RESPONSE) {
        registerButton->setEnabled(true);  // 重新启用注册按钮

        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            QString username = message.data["username"].toString();
            QMessageBox::information(this, "注册成功",
                                     QString("用户 %1 注册成功").arg(username));
            emit registrationSuccess();
            accept();
        } else {
            QMessageBox::warning(this, "注册失败", resultMsg);
            // 注册失败时清除样式
            usernameEdit->setStyleSheet("");
        }

        // 重置注册状态
        pendingRegistration = PendingRegistration();
    }
}
