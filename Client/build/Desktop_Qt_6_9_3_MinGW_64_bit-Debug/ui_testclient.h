/********************************************************************************
** Form generated from reading UI file 'testclient.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TESTCLIENT_H
#define UI_TESTCLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TestClient
{
public:
    QFrame *loginFrame;
    QVBoxLayout *frameLayout;
    QHBoxLayout *titleLayout;
    QSpacerItem *titleLeftSpacer;
    QLabel *titleLabel;
    QSpacerItem *titleRightSpacer;
    QHBoxLayout *usernameLayout;
    QLabel *usernameLabel;
    QLineEdit *usernameEdit;
    QHBoxLayout *passwordLayout;
    QLabel *passwordLabel;
    QLineEdit *passwordEdit;
    QHBoxLayout *buttonLayout;
    QPushButton *loginButton;
    QLabel *outputLabel;
    QTextEdit *textEditOutput;

    void setupUi(QWidget *TestClient)
    {
        if (TestClient->objectName().isEmpty())
            TestClient->setObjectName("TestClient");
        TestClient->resize(640, 500);
        TestClient->setStyleSheet(QString::fromUtf8("/* \344\270\273\347\252\227\345\217\243\346\240\267\345\274\217 - \344\270\216RegisterDialog\347\233\270\345\220\214\347\232\204\346\267\261\350\223\235\350\211\262\346\270\220\345\217\230 */\n"
"QWidget {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2c3e50, stop:1 #34495e);\n"
"    border-radius: 15px;\n"
"    border: 2px solid #34495e;\n"
"}\n"
"\n"
"/* \346\211\200\346\234\211\346\240\207\347\255\276\351\273\230\350\256\244\346\240\267\345\274\217 - \351\273\221\350\211\262\345\255\227\344\275\223 */\n"
"QLabel {\n"
"    color: black !important;\n"
"    font-size: 14px;\n"
"    font-weight: bold;\n"
"    background: transparent;\n"
"}"));
        loginFrame = new QFrame(TestClient);
        loginFrame->setObjectName("loginFrame");
        loginFrame->setGeometry(QRect(140, 100, 350, 311));
        loginFrame->setStyleSheet(QString::fromUtf8("/* Frame\346\240\267\345\274\217 - \347\247\273\351\231\244padding */\n"
"QFrame {\n"
"    background-color: rgba(255, 255, 255, 0.95);\n"
"    border-radius: 10px;\n"
"    border: 2px solid rgba(255, 255, 255, 0.5);\n"
"}\n"
"\n"
"/* Frame\345\206\205\347\232\204\346\240\207\347\255\276\346\240\267\345\274\217 */\n"
"QFrame QLabel {\n"
"    color: black !important;\n"
"    font-size: 14px;\n"
"    font-weight: bold;\n"
"    background: transparent;\n"
"}"));
        loginFrame->setFrameShape(QFrame::Shape::StyledPanel);
        loginFrame->setFrameShadow(QFrame::Shadow::Raised);
        frameLayout = new QVBoxLayout(loginFrame);
        frameLayout->setSpacing(10);
        frameLayout->setObjectName("frameLayout");
        frameLayout->setContentsMargins(20, 20, 20, 20);
        titleLayout = new QHBoxLayout();
        titleLayout->setObjectName("titleLayout");
        titleLeftSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        titleLayout->addItem(titleLeftSpacer);

        titleLabel = new QLabel(loginFrame);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setMinimumSize(QSize(120, 35));
        titleLabel->setMaximumSize(QSize(120, 35));
        titleLabel->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    background-color: #3498db;\n"
"    color: white;\n"
"    border-radius: 5px;\n"
"    font-size: 16px;\n"
"    font-weight: bold;\n"
"    padding: 5px 15px;\n"
"}"));
        titleLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        titleLayout->addWidget(titleLabel);

        titleRightSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        titleLayout->addItem(titleRightSpacer);


        frameLayout->addLayout(titleLayout);

        usernameLayout = new QHBoxLayout();
        usernameLayout->setObjectName("usernameLayout");
        usernameLabel = new QLabel(loginFrame);
        usernameLabel->setObjectName("usernameLabel");
        usernameLabel->setMinimumSize(QSize(60, 30));

        usernameLayout->addWidget(usernameLabel);

        usernameEdit = new QLineEdit(loginFrame);
        usernameEdit->setObjectName("usernameEdit");
        usernameEdit->setMinimumSize(QSize(228, 55));
        usernameEdit->setMaximumSize(QSize(228, 55));
        usernameEdit->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background-color: white;\n"
"    border: 2px solid #bdc3c7;\n"
"    border-radius: 5px;\n"
"    padding: 8px 12px;\n"
"    font-size: 14px;\n"
"    color: #2c3e50;\n"
"    min-width: 200px;\n"
"    max-width: 200px;\n"
"    min-height: 35px;\n"
"    max-height: 35px;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    border-color: #3498db;\n"
"    background-color: #f8f9fa;\n"
"}"));

        usernameLayout->addWidget(usernameEdit);


        frameLayout->addLayout(usernameLayout);

        passwordLayout = new QHBoxLayout();
        passwordLayout->setObjectName("passwordLayout");
        passwordLabel = new QLabel(loginFrame);
        passwordLabel->setObjectName("passwordLabel");
        passwordLabel->setMinimumSize(QSize(60, 30));

        passwordLayout->addWidget(passwordLabel);

        passwordEdit = new QLineEdit(loginFrame);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setMinimumSize(QSize(228, 55));
        passwordEdit->setMaximumSize(QSize(228, 55));
        passwordEdit->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background-color: white;\n"
"    border: 2px solid #bdc3c7;\n"
"    border-radius: 5px;\n"
"    padding: 8px 12px;\n"
"    font-size: 14px;\n"
"    color: #2c3e50;\n"
"    min-width: 200px;\n"
"    max-width: 200px;\n"
"    min-height: 35px;\n"
"    max-height: 35px;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    border-color: #3498db;\n"
"    background-color: #f8f9fa;\n"
"}"));
        passwordEdit->setEchoMode(QLineEdit::EchoMode::Password);

        passwordLayout->addWidget(passwordEdit);


        frameLayout->addLayout(passwordLayout);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName("buttonLayout");
        loginButton = new QPushButton(loginFrame);
        loginButton->setObjectName("loginButton");
        loginButton->setMinimumSize(QSize(96, 24));
        loginButton->setMaximumSize(QSize(96, 24));
        loginButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: #27ae60;\n"
"    color: white;\n"
"    border: none;\n"
"    border-radius: 5px;\n"
"    padding: 2px 8px;\n"
"    font-size: 12px;\n"
"    font-weight: bold;\n"
"    min-width: 80px;\n"
"    max-width: 80px;\n"
"    min-height: 20px;\n"
"    max-height: 20px;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background-color: #2ecc71;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: #229954;\n"
"}"));

        buttonLayout->addWidget(loginButton);


        frameLayout->addLayout(buttonLayout);

        outputLabel = new QLabel(TestClient);
        outputLabel->setObjectName("outputLabel");
        outputLabel->setGeometry(QRect(20, 420, 80, 20));
        outputLabel->setVisible(false);
        outputLabel->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    color: black;\n"
"    background: transparent;\n"
"}"));
        textEditOutput = new QTextEdit(TestClient);
        textEditOutput->setObjectName("textEditOutput");
        textEditOutput->setGeometry(QRect(100, 420, 520, 60));
        textEditOutput->setVisible(false);
        textEditOutput->setReadOnly(true);

        retranslateUi(TestClient);

        QMetaObject::connectSlotsByName(TestClient);
    } // setupUi

    void retranslateUi(QWidget *TestClient)
    {
        TestClient->setWindowTitle(QCoreApplication::translate("TestClient", "\350\210\252\347\217\255\347\256\241\347\220\206\347\263\273\347\273\237 - \347\231\273\345\275\225", nullptr));
        titleLabel->setText(QCoreApplication::translate("TestClient", "\347\224\250\346\210\267\347\231\273\345\275\225", nullptr));
        usernameLabel->setText(QCoreApplication::translate("TestClient", "\347\224\250\346\210\267\345\220\215:", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("TestClient", "\350\257\267\350\276\223\345\205\245\347\224\250\346\210\267\345\220\215", nullptr));
        passwordLabel->setText(QCoreApplication::translate("TestClient", "\345\257\206   \347\240\201:", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("TestClient", "\350\257\267\350\276\223\345\205\245\345\257\206\347\240\201", nullptr));
        loginButton->setText(QCoreApplication::translate("TestClient", "\347\231\273\345\275\225", nullptr));
        outputLabel->setText(QCoreApplication::translate("TestClient", "\346\266\210\346\201\257\350\276\223\345\207\272:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TestClient: public Ui_TestClient {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TESTCLIENT_H
