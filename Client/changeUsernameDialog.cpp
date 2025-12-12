#include "changeUsernameDialog.h"
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
#include <QEvent>
#include <QCloseEvent>
#include <QFontMetrics>

class UsernamePlaceholderFilter : public QObject
{
public:
    UsernamePlaceholderFilter(QLineEdit *edit, const QString &text)
        : QObject(edit), lineEdit(edit), placeholder(text) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (obj == lineEdit) {
            if (event->type() == QEvent::FocusIn) {
                lineEdit->setPlaceholderText("");
            } else if (event->type() == QEvent::FocusOut) {
                if (lineEdit->text().isEmpty())
                    lineEdit->setPlaceholderText(placeholder);
            }
        }
        return false;
    }

private:
    QLineEdit *lineEdit;
    QString placeholder;
};

ChangeUsernameDialog::ChangeUsernameDialog(ClientNetworkManager* networkManager,
                                           const QString& currentUsername,
                                           QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint),
    networkManager(networkManager),
    currentUsername(currentUsername),
    titleLabel(nullptr),
    currentUsernameLabel(nullptr),
    newUsernameLabel(nullptr),
    errorLabel(nullptr),
    newUsernameEdit(nullptr),
    confirmButton(nullptr),
    cancelButton(nullptr),
    isCheckingUsername(false)
{
    // 设置窗口属性
    setAttribute(Qt::WA_DeleteOnClose, false);
    setModal(true);

    setupUI();
    applyBeautifyStyles();
    setWindowTitle("修改用户名");
    setFixedSize(450, 350);

    // 连接网络消息信号
    connect(networkManager, &ClientNetworkManager::messageReceived,
            this, &ChangeUsernameDialog::onMessageReceived);

    // 连接按钮信号
    connect(confirmButton, &QPushButton::clicked, this, &ChangeUsernameDialog::onConfirmClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ChangeUsernameDialog::onCancelClicked);
}

ChangeUsernameDialog::~ChangeUsernameDialog()
{
    // 所有Qt对象会自动被父对象删除
}

void ChangeUsernameDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 创建组框
    QGroupBox *groupBox = new QGroupBox("修改用户名", this);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);

    // 标题
    titleLabel = new QLabel("✏️ 修改用户名", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    groupLayout->addWidget(titleLabel);

    // 当前用户名显示
    currentUsernameLabel = new QLabel("当前用户名: " + currentUsername, this);
    groupLayout->addWidget(currentUsernameLabel);

    // 新用户名输入
    QHBoxLayout *newUsernameLayout = new QHBoxLayout();
    newUsernameLabel = new QLabel("新用户名:", this);
    newUsernameEdit = new QLineEdit(this);
    newUsernameEdit->setPlaceholderText("请输入新用户名");
    newUsernameEdit->installEventFilter(new UsernamePlaceholderFilter(newUsernameEdit, "请输入新用户名"));
    newUsernameLayout->addWidget(newUsernameLabel);
    newUsernameLayout->addWidget(newUsernameEdit);
    groupLayout->addLayout(newUsernameLayout);

    // 错误信息标签
    errorLabel = new QLabel("", this);
    errorLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
    groupLayout->addWidget(errorLabel);

    // 添加弹性空间
    groupLayout->addStretch();

    // 按钮行
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    cancelButton = new QPushButton("取消", this);
    confirmButton = new QPushButton("确认修改", this);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(confirmButton);
    groupLayout->addLayout(buttonLayout);

    mainLayout->addWidget(groupBox);
}

void ChangeUsernameDialog::applyBeautifyStyles()
{
    // 1. 设置窗口背景为黄蓝色渐变，与注册界面一致
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
            "  background-color: rgba(255, 255, 255, 0.92);"
            "  border-radius: 12px;"
            "  border: 2px solid rgba(255, 255, 255, 0.8);"
            "  padding: 25px;"
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
        if (label != titleLabel) {  // 排除标题label
            int w = fm.horizontalAdvance(label->text());
            maxWidth = std::max(maxWidth, w);
        }
    }
    maxWidth += 15;

    // 为所有非标题label统一设置宽度
    for (QLabel* const &label : labels) {
        if (label != titleLabel) {
            label->setMinimumWidth(maxWidth);
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }

        if (label == titleLabel) {
            label->setStyleSheet(
                "QLabel {"
                "  color: #1e3a5f;"
                "  font-size: 18px;"
                "  font-weight: bold;"
                "  padding: 10px 0px;"
                "  background: transparent;"
                "}"
                );
        } else if (label == currentUsernameLabel) {
            label->setStyleSheet(
                "QLabel {"
                "  color: #3498db;"  // 蓝色显示当前用户名
                "  font-size: 14px;"
                "  font-weight: bold;"
                "  background: transparent;"
                "  padding: 8px 0px;"
                "}"
                );
        } else {
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
        btn->setFixedWidth(100);

        if (btn->text().contains("确认修改")) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FFD700, stop:1 #FFA500);"  // 黄橙渐变
                "  color: white;"
                "  border: none;"
                "  border-radius: 6px;"
                "  padding: 10px 15px;"
                "  font-size: 14px;"
                "  font-weight: bold;"
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

bool ChangeUsernameDialog::validateUsername(const QString& username)
{
    if (username.isEmpty()) {
        errorLabel->setText("用户名不能为空");
        newUsernameEdit->setFocus();
        return false;
    }

    if (username == currentUsername) {
        errorLabel->setText("新用户名不能与原用户名相同");
        newUsernameEdit->setFocus();
        newUsernameEdit->selectAll();
        return false;
    }

    if (username.length() < 3 || username.length() > 20) {
        errorLabel->setText("用户名长度应在3-20个字符之间");
        newUsernameEdit->setFocus();
        newUsernameEdit->selectAll();
        return false;
    }

    errorLabel->clear();
    return true;
}

void ChangeUsernameDialog::onConfirmClicked()
{
    QString newUsername = newUsernameEdit->text().trimmed();

    if (!validateUsername(newUsername)) {
        return;
    }

    if (!networkManager->isConnected()) {
        QMessageBox::warning(this, "连接错误", "未连接到服务器");
        return;
    }

    // 先检查用户名是否可用
    isCheckingUsername = true;
    pendingNewUsername = newUsername;

    NetworkMessage checkMsg;
    checkMsg.type = CHECK_USERNAME_REQUEST;
    checkMsg.data["username"] = newUsername;
    networkManager->sendMessage(checkMsg);

    // 禁用确认按钮，防止重复点击
    confirmButton->setEnabled(false);
    qDebug() << "发送用户名检查请求:" << newUsername;
}

void ChangeUsernameDialog::onCancelClicked()
{
    close();
}

void ChangeUsernameDialog::onMessageReceived(const NetworkMessage &message)
{
    // 处理用户名检查响应
    if (message.type == CHECK_USERNAME_RESPONSE) {
        bool exists = message.data["exists"].toBool();
        QString checkedUsername = message.data["username"].toString();

        if (isCheckingUsername && checkedUsername == pendingNewUsername) {
            isCheckingUsername = false;
            confirmButton->setEnabled(true); // 重新启用按钮

            if (exists) {
                // 用户名已存在
                newUsernameEdit->setStyleSheet(
                    "QLineEdit {"
                    "  background-color: white;"
                    "  border: 2px solid #e74c3c;"
                    "  border-radius: 6px;"
                    "  padding: 8px 12px;"
                    "  font-size: 14px;"
                    "  color: #2c3e50;"
                    "}"
                    );
                errorLabel->setText("该用户名已被使用，请选择其他用户名");
                newUsernameEdit->setFocus();
                newUsernameEdit->selectAll();
            } else {
                // 用户名可用，发送修改请求
                newUsernameEdit->setStyleSheet(
                    "QLineEdit {"
                    "  background-color: white;"
                    "  border: 2px solid #27ae60;"  // 绿色边框表示可用
                    "  border-radius: 6px;"
                    "  padding: 8px 12px;"
                    "  font-size: 14px;"
                    "  color: #2c3e50;"
                    "}"
                    );

                NetworkMessage changeMsg;
                changeMsg.type = CHANGE_USERNAME_REQUEST;
                changeMsg.data["old_username"] = currentUsername;
                changeMsg.data["new_username"] = pendingNewUsername;
                networkManager->sendMessage(changeMsg);

                confirmButton->setEnabled(false); // 禁用按钮等待响应
                qDebug() << "发送修改用户名请求:" << currentUsername << "->" << pendingNewUsername;
            }
        }
    }

    // 处理修改用户名响应
    if (message.type == CHANGE_USERNAME_RESPONSE) {
        confirmButton->setEnabled(true); // 重新启用按钮

        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();
        QString newUsername = message.data["new_username"].toString();

        if (success) {
            QMessageBox::information(this, "修改成功",
                                     QString("用户名已成功修改为: %1").arg(newUsername));
            emit usernameChanged(newUsername);
            accept();
        } else {
            QMessageBox::warning(this, "修改失败", resultMsg);
            // 恢复输入框样式
            newUsernameEdit->setStyleSheet(
                "QLineEdit {"
                "  background-color: white;"
                "  border: 2px solid #bdc3c7;"
                "  border-radius: 6px;"
                "  padding: 8px 12px;"
                "  font-size: 14px;"
                "  color: #2c3e50;"
                "}"
                );
            errorLabel->setText("修改失败，请重试");
        }
    }
}

void ChangeUsernameDialog::closeEvent(QCloseEvent *event)
{
    // 清理输入框
    newUsernameEdit->clear();
    errorLabel->clear();

    // 重置按钮状态
    confirmButton->setEnabled(true);

    // 恢复输入框默认样式
    newUsernameEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: white;"
        "  border: 2px solid #bdc3c7;"
        "  border-radius: 6px;"
        "  padding: 8px 12px;"
        "  font-size: 14px;"
        "  color: #2c3e50;"
        "}"
        );

    // 重置状态
    isCheckingUsername = false;
    pendingNewUsername.clear();

    // 接受关闭事件
    event->accept();
}
