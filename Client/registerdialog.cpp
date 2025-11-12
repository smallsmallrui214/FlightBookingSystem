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
{
    setupUI();
    setWindowTitle("用户注册");
    setFixedSize(400, 300);

    // 连接网络消息信号
    connect(networkManager, &ClientNetworkManager::messageReceived,
            this, &RegisterDialog::onMessageReceived);

    connect(this, &RegisterDialog::finished, this, [this](int result) {
        // 清空所有输入框
        usernameEdit->clear();
        passwordEdit->clear();
        confirmPasswordEdit->clear();
        emailEdit->clear();

        // 重新启用注册按钮（如果之前被禁用）
        registerButton->setEnabled(true);
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
    passwordLayout->addWidget(new QLabel("密码:", this));
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->installEventFilter(new PlaceholderFilter(passwordEdit, "请输入密码"));
    passwordLayout->addWidget(passwordEdit);
    groupLayout->addLayout(passwordLayout);

    // 确认密码行
    QHBoxLayout *confirmLayout = new QHBoxLayout();
    confirmLayout->addWidget(new QLabel("确认密码:", this));
    confirmPasswordEdit = new QLineEdit(this);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("请再次输入密码");
    confirmPasswordEdit->installEventFilter(new PlaceholderFilter(confirmPasswordEdit, "请再次输入密码"));
    confirmLayout->addWidget(confirmPasswordEdit);
    groupLayout->addLayout(confirmLayout);

    // 邮箱行
    QHBoxLayout *emailLayout = new QHBoxLayout();
    emailLayout->addWidget(new QLabel("邮箱:", this));
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

void RegisterDialog::onRegisterClicked()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();
    QString confirmPassword = confirmPasswordEdit->text().trimmed();
    QString email = emailEdit->text().trimmed();

    // 检查用户名是否为空
    if (username.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名");
        usernameEdit->setFocus(); // 聚焦到用户名输入框
        return;
    }

    // 检查密码是否为空
    if (password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入密码");
        passwordEdit->setFocus(); // 聚焦到密码输入框
        return;
    }

    // 检查确认密码是否为空
    if (confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请再次输入密码");
        confirmPasswordEdit->setFocus(); // 聚焦到确认密码输入框
        return;
    }

    // 检查两次密码是否一致
    if (password != confirmPassword) {
        QMessageBox::warning(this, "输入错误", "两次输入的密码不一致");
        confirmPasswordEdit->setFocus(); // 聚焦到确认密码输入框
        confirmPasswordEdit->selectAll(); // 可选：选中所有文本方便修改
        return;
    }

    if (!networkManager->isConnected()) {
        QMessageBox::warning(this, "连接错误", "未连接到服务器");
        return;
    }

    // 发送注册请求
    NetworkMessage msg;
    msg.type = REGISTER_REQUEST;
    msg.data["username"] = username;
    msg.data["password"] = password;
    msg.data["email"] = email;
    networkManager->sendMessage(msg);

    // 禁用注册按钮，防止重复点击
    registerButton->setEnabled(false);
    qDebug() << "发送注册请求:" << username;
}

void RegisterDialog::onCancelClicked()
{
    reject(); // 关闭对话框
}

void RegisterDialog::onMessageReceived(const NetworkMessage &message)
{
    // 处理注册响应
    if (message.type == REGISTER_RESPONSE) {
        registerButton->setEnabled(true);  // 重新启用注册按钮

        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            QString username = message.data["username"].toString();
            QMessageBox::information(this, "注册成功",
                                     QString("用户 %1 注册成功").arg(username));
            emit registrationSuccess();  // 发射注册成功信号
            accept(); // 关闭对话框
        } else {
            QMessageBox::warning(this, "注册失败", resultMsg);
        }
    }
}
