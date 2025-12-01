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
#include <QTimer>
#include <QCloseEvent>  // 新增头文件


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
    : QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint),
    networkManager(networkManager),
    usernameEdit(nullptr),
    passwordEdit(nullptr),
    confirmPasswordEdit(nullptr),
    emailEdit(nullptr),
    registerButton(nullptr),
    cancelButton(nullptr)
{
    // 设置窗口属性
    setAttribute(Qt::WA_DeleteOnClose, false);
    setModal(true);

    setupUI();
    applyBeautifyStyles();
    setWindowTitle("用户注册");
    setFixedSize(450, 350);

    // 初始化 pendingRegistration
    pendingRegistration.checked = false;

    // 连接网络消息信号
    connect(networkManager, &ClientNetworkManager::messageReceived,
            this, &RegisterDialog::onMessageReceived);

    // 连接按钮信号
    connect(registerButton, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
    connect(cancelButton, &QPushButton::clicked, this, &RegisterDialog::onCancelClicked);
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
}
void RegisterDialog::applyBeautifyStyles()
{
    // 1. 设置窗口背景为黄蓝色渐变，匹配登录界面主题
    this->setStyleSheet(
        "QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #FFD700, stop:0.5 #87CEEB, stop:1 #1E90FF);"  // 黄蓝渐变
        "  border-radius: 15px;"
        "  border: 3px solid rgba(255, 255, 255, 0.5);"
        "}"
        );

    // 2. 美化 GroupBox
    const QList<QGroupBox*> groupBoxes = findChildren<QGroupBox*>();
    for (QGroupBox* const &groupBox : groupBoxes) {
        groupBox->setStyleSheet(
            "QGroupBox {"
            "  background-color: rgba(255, 255, 255, 0.92);"  // 更透明
            "  border-radius: 12px;"
            "  border: 2px solid rgba(255, 255, 255, 0.8);"
            "  padding: 20px;"
            "  margin: 15px;"
            "  font-size: 14px;"
            "}"
            "QGroupBox::title {"
            "  subcontrol-origin: margin;"
            "  subcontrol-position: top center;"
            "  padding: 6px 18px;"
            "  background-color: #FFA500;"  // 橙色，匹配黄蓝主题
            "  color: white;"
            "  border-radius: 8px;"
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
            "  color: #1e3a5f;"  // 深蓝色文字
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
            "  border-radius: 6px;"
            "  padding: 8px 12px;"
            "  font-size: 14px;"
            "  color: #2c3e50;"
            "  min-width: 200px;"
            "}"
            "QLineEdit:focus {"
            "  border-color: #FFD700;"  // 黄色边框，匹配主题
            "  background-color: #fffaf0;"
            "}"
            );
    }

    // 5. 美化按钮
    const QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* const &btn : buttons) {
        btn->setFixedHeight(35);
        btn->setFixedWidth(90);  // 稍微加宽

        if (btn->text().contains("注册")) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FFD700, stop:1 #FFA500);"  // 黄橙渐变
                "  color: white;"
                "  border: none;"
                "  border-radius: 6px;"
                "  padding: 10px 15px;"
                "  font-size: 14px;"
                "  font-weight: bold;"
                "  min-width: 90px;"
                "}"
                "QPushButton:hover {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FFA500, stop:1 #FF8C00);"
                "}"
                "QPushButton:pressed {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FF8C00, stop:1 #FF7F50);"
                "}"
                );
        } else if (btn->text().contains("取消")) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1E90FF, stop:1 #4169E1);"  // 蓝色渐变
                "  color: white;"
                "  border: none;"
                "  border-radius: 6px;"
                "  padding: 10px 15px;"
                "  font-size: 14px;"
                "  font-weight: bold;"
                "  min-width: 90px;"
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
    // 直接关闭对话框
    close();
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
                                     QString("用户 %1 注册成功，请返回登录").arg(username));
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

void RegisterDialog::closeEvent(QCloseEvent *event)
{
    // 清理输入框
    usernameEdit->clear();
    passwordEdit->clear();
    confirmPasswordEdit->clear();
    emailEdit->clear();

    // 重置按钮状态
    registerButton->setEnabled(true);

    // 重置注册状态
    pendingRegistration = PendingRegistration();

    // 恢复输入框默认样式
    usernameEdit->setStyleSheet("");

    // 接受关闭事件
    event->accept();
}
