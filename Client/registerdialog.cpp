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
    cancelButton(nullptr),
    isUsernameAvailable(false)
{
    setupUI();
    applyBeautifyStyles();//新增调用美化函数
    setWindowTitle("用户注册");
    setFixedSize(450, 350);

    // 连接网络消息信号
    connect(networkManager, &ClientNetworkManager::messageReceived,
            this, &RegisterDialog::onMessageReceived);

    connect(usernameEdit, &QLineEdit::textChanged, this, &RegisterDialog::checkUsernameAvailability);
    connect(cancelButton, &QPushButton::clicked, this, &RegisterDialog::onCancelClicked);

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
    // 1. 设置窗口背景
    this->setStyleSheet(
        "QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #2c3e50, stop:1 #34495e);"
        "  border-radius: 15px;"
        "  border: 2px solid #34495e;"
        "}"
        );

    // 2. 美化 GroupBox
    const QList<QGroupBox*> groupBoxes = findChildren<QGroupBox*>();
    for (QGroupBox* const &groupBox : groupBoxes) {
        groupBox->setStyleSheet(
            "QGroupBox {"
            "  background-color: rgba(255, 255, 255, 0.95);"
            "  border-radius: 10px;"
            "  border: 2px solid rgba(255, 255, 255, 0.5);"
            "  padding: 20px;"
            "  margin: 15px;"
            "  font-size: 14px;"
            "}"
            "QGroupBox::title {"
            "  subcontrol-origin: margin;"
            "  subcontrol-position: top center;"
            "  padding: 5px 15px;"
            "  background-color: #3498db;"
            "  color: white;"
            "  border-radius: 5px;"
            "  font-size: 16px;"
            "  font-weight: bold;"
            "}"
            );
    }

    // 3. 🌟 美化 Label —— 自动对齐所有标签
    const QList<QLabel*> labels = findChildren<QLabel*>();

    // (1) 获取所有 label 文本的最大宽度
    int maxWidth = 0;
    QFontMetrics fm(this->font());
    for (QLabel* const &label : labels) {
        int w = fm.horizontalAdvance(label->text());
        maxWidth = std::max(maxWidth, w);
    }
    maxWidth += 20; // 增加一点边距，使布局更美观

    // (2) 为所有 label 统一设置宽度 + 右对齐
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
            "  border-color: #3498db;"
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
                "  background-color: #27ae60;"
                "  color: white;"
                "  border: none;"
                "  border-radius: 5px;"
                "  padding: 10px 15px;"
                "  font-size: 14px;"
                "  font-weight: bold;"
                "  min-width: 80px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #2ecc71;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #229954;"
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

//新增美化函数定义
void RegisterDialog::checkUsernameAvailability()
{
    QString username = usernameEdit->text().trimmed();

    if (username.isEmpty()) {
        isUsernameAvailable = false;
        usernameEdit->setStyleSheet(""); // 清除样式
        return;
    }

    // 延迟检查，避免频繁请求
    QTimer::singleShot(500, this, [this, username]() {
        if (username == usernameEdit->text().trimmed() && !username.isEmpty()) {
            // 发送检查用户名请求
            NetworkMessage msg;
            msg.type = CHECK_USERNAME_REQUEST;
            msg.data["username"] = username;
            networkManager->sendMessage(msg);

            pendingUsername = username;
            qDebug() << "检查用户名是否存在:" << username;
        }
    });
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
        confirmPasswordEdit->setFocus(); // 聚焦到确认密码输入框
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
        confirmPasswordEdit->setFocus(); // 聚焦到确认密码输入框
        confirmPasswordEdit->selectAll(); // 可选：选中所有文本方便修改
        return;
    }

    // 检查用户名是否可用
    if (!isUsernameAvailable || pendingUsername != username) {
        QMessageBox::warning(this, "输入错误", "请等待用户名检查完成或用户名不可用");
        usernameEdit->setFocus();
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
    // 处理用户名检查响应
    if (message.type == CHECK_USERNAME_RESPONSE) {
        bool exists = message.data["exists"].toBool();  // 服务器返回用户名是否存在
        QString checkedUsername = message.data["username"].toString();

        // 确保检查的是当前输入框中的用户名
        if (checkedUsername == usernameEdit->text().trimmed()) {
            isUsernameAvailable = !exists;  // 如果不存在，就是可用的

            if (exists) {
                // 用户名已存在，给用户视觉提示
                usernameEdit->setStyleSheet("border: 1px solid red;");
                QMessageBox::warning(this, "用户名不可用", "该用户名已被使用，请选择其他用户名");
                usernameEdit->setFocus();
                usernameEdit->selectAll();
            } else {
                // 用户名可用
                usernameEdit->setStyleSheet("border: 1px solid green;");
                qDebug() << "用户名可用:" << checkedUsername;
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
            // 注册失败时重置用户名可用状态
            isUsernameAvailable = false;
            usernameEdit->setStyleSheet(""); // 清除样式
        }
    }

}
