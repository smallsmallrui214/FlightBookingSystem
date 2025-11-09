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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TestClient
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QLabel *inputLabel;
    QTextEdit *textEditInput;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *sendButton;
    QPushButton *loginButton;
    QLabel *outputLabel;
    QTextEdit *textEditOutput;

    void setupUi(QWidget *TestClient)
    {
        if (TestClient->objectName().isEmpty())
            TestClient->setObjectName("TestClient");
        TestClient->resize(600, 500);
        verticalLayout = new QVBoxLayout(TestClient);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        connectButton = new QPushButton(TestClient);
        connectButton->setObjectName("connectButton");

        horizontalLayout->addWidget(connectButton);

        disconnectButton = new QPushButton(TestClient);
        disconnectButton->setObjectName("disconnectButton");

        horizontalLayout->addWidget(disconnectButton);


        verticalLayout->addLayout(horizontalLayout);

        inputLabel = new QLabel(TestClient);
        inputLabel->setObjectName("inputLabel");

        verticalLayout->addWidget(inputLabel);

        textEditInput = new QTextEdit(TestClient);
        textEditInput->setObjectName("textEditInput");
        textEditInput->setMaximumHeight(60);

        verticalLayout->addWidget(textEditInput);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        sendButton = new QPushButton(TestClient);
        sendButton->setObjectName("sendButton");

        horizontalLayout_2->addWidget(sendButton);

        loginButton = new QPushButton(TestClient);
        loginButton->setObjectName("loginButton");

        horizontalLayout_2->addWidget(loginButton);


        verticalLayout->addLayout(horizontalLayout_2);

        outputLabel = new QLabel(TestClient);
        outputLabel->setObjectName("outputLabel");

        verticalLayout->addWidget(outputLabel);

        textEditOutput = new QTextEdit(TestClient);
        textEditOutput->setObjectName("textEditOutput");
        textEditOutput->setReadOnly(true);

        verticalLayout->addWidget(textEditOutput);


        retranslateUi(TestClient);

        QMetaObject::connectSlotsByName(TestClient);
    } // setupUi

    void retranslateUi(QWidget *TestClient)
    {
        TestClient->setWindowTitle(QCoreApplication::translate("TestClient", "\350\210\252\347\217\255\345\256\242\346\210\267\347\253\257\346\265\213\350\257\225", nullptr));
        connectButton->setText(QCoreApplication::translate("TestClient", "\350\277\236\346\216\245\346\234\215\345\212\241\345\231\250", nullptr));
        disconnectButton->setText(QCoreApplication::translate("TestClient", "\346\226\255\345\274\200\350\277\236\346\216\245", nullptr));
        inputLabel->setText(QCoreApplication::translate("TestClient", "\350\276\223\345\205\245\346\266\210\346\201\257:", nullptr));
        textEditInput->setPlaceholderText(QCoreApplication::translate("TestClient", "\345\234\250\350\277\231\351\207\214\350\276\223\345\205\245\350\246\201\345\217\221\351\200\201\347\232\204\346\266\210\346\201\257...", nullptr));
        sendButton->setText(QCoreApplication::translate("TestClient", "\345\217\221\351\200\201\346\265\213\350\257\225\346\266\210\346\201\257", nullptr));
        loginButton->setText(QCoreApplication::translate("TestClient", "\346\265\213\350\257\225\347\231\273\345\275\225", nullptr));
        outputLabel->setText(QCoreApplication::translate("TestClient", "\346\266\210\346\201\257\350\276\223\345\207\272:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TestClient: public Ui_TestClient {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TESTCLIENT_H
