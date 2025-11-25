/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QDate>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QFrame *headerFrame;
    QHBoxLayout *horizontalLayout;
    QLabel *welcomeLabel;
    QSpacerItem *horizontalSpacer;
    QLabel *userLabel;
    QPushButton *logoutButton;
    QFrame *searchFrame;
    QVBoxLayout *verticalLayout_2;
    QLabel *searchTitleLabel;
    QHBoxLayout *searchLayout;
    QLineEdit *departureEdit;
    QPushButton *swapButton;
    QLineEdit *arrivalEdit;
    QDateEdit *dateEdit;
    QPushButton *searchButton;
    QHBoxLayout *filterLayout;
    QLabel *sortLabel;
    QComboBox *sortComboBox;
    QLabel *airlineLabel;
    QComboBox *airlineComboBox;
    QSpacerItem *horizontalSpacer_2;
    QListWidget *flightListWidget;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 800);
        MainWindow->setStyleSheet(QString::fromUtf8("QMainWindow {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #f8f9fa, stop:1 #e9ecef);\n"
"    font-family: \"Microsoft YaHei\", \"PingFang SC\", sans-serif;\n"
"}"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        headerFrame = new QFrame(centralwidget);
        headerFrame->setObjectName("headerFrame");
        headerFrame->setMinimumSize(QSize(0, 80));
        headerFrame->setStyleSheet(QString::fromUtf8("QFrame {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1e88e5, stop:1 #1565c0);\n"
"    border: none;\n"
"    border-bottom: 2px solid #0d47a1;\n"
"}"));
        horizontalLayout = new QHBoxLayout(headerFrame);
        horizontalLayout->setObjectName("horizontalLayout");
        welcomeLabel = new QLabel(headerFrame);
        welcomeLabel->setObjectName("welcomeLabel");
        welcomeLabel->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    color: white;\n"
"    font-size: 20px;\n"
"    font-weight: bold;\n"
"    padding: 5px;\n"
"}"));

        horizontalLayout->addWidget(welcomeLabel);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        userLabel = new QLabel(headerFrame);
        userLabel->setObjectName("userLabel");
        userLabel->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    color: rgba(255,255,255,0.9);\n"
"    font-size: 14px;\n"
"    background: rgba(255,255,255,0.1);\n"
"    padding: 8px 15px;\n"
"    border-radius: 15px;\n"
"}"));

        horizontalLayout->addWidget(userLabel);

        logoutButton = new QPushButton(headerFrame);
        logoutButton->setObjectName("logoutButton");
        logoutButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: rgba(255,255,255,0.2);\n"
"    color: white;\n"
"    border: 1px solid rgba(255,255,255,0.3);\n"
"    border-radius: 6px;\n"
"    padding: 8px 20px;\n"
"    font-size: 14px;\n"
"    font-weight: bold;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background-color: rgba(255,255,255,0.3);\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgba(255,255,255,0.4);\n"
"}"));

        horizontalLayout->addWidget(logoutButton);


        verticalLayout->addWidget(headerFrame);

        searchFrame = new QFrame(centralwidget);
        searchFrame->setObjectName("searchFrame");
        searchFrame->setStyleSheet(QString::fromUtf8("QFrame {\n"
"    background: white;\n"
"    border: 2px solid #e3f2fd;\n"
"    border-radius: 12px;\n"
"    padding: 25px;\n"
"    margin: 15px;\n"
"}"));
        verticalLayout_2 = new QVBoxLayout(searchFrame);
        verticalLayout_2->setObjectName("verticalLayout_2");
        searchTitleLabel = new QLabel(searchFrame);
        searchTitleLabel->setObjectName("searchTitleLabel");
        searchTitleLabel->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    color: #1e88e5;\n"
"    font-size: 18px;\n"
"    font-weight: bold;\n"
"    padding: 5px 0px;\n"
"}"));

        verticalLayout_2->addWidget(searchTitleLabel);

        searchLayout = new QHBoxLayout();
        searchLayout->setObjectName("searchLayout");
        departureEdit = new QLineEdit(searchFrame);
        departureEdit->setObjectName("departureEdit");
        departureEdit->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background: white;\n"
"    border: 2px solid #e0e0e0;\n"
"    border-radius: 8px;\n"
"    padding: 12px 15px;\n"
"    font-size: 14px;\n"
"    color: #333;\n"
"    min-width: 140px;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    border-color: #1e88e5;\n"
"    background-color: #f8fdff;\n"
"}"));

        searchLayout->addWidget(departureEdit);

        swapButton = new QPushButton(searchFrame);
        swapButton->setObjectName("swapButton");
        swapButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background: #1e88e5;\n"
"    color: white;\n"
"    border: none;\n"
"    border-radius: 6px;\n"
"    padding: 10px;\n"
"    font-size: 16px;\n"
"    font-weight: bold;\n"
"    min-width: 40px;\n"
"    max-width: 40px;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background: #1976d2;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background: #1565c0;\n"
"}"));

        searchLayout->addWidget(swapButton);

        arrivalEdit = new QLineEdit(searchFrame);
        arrivalEdit->setObjectName("arrivalEdit");
        arrivalEdit->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background: white;\n"
"    border: 2px solid #e0e0e0;\n"
"    border-radius: 8px;\n"
"    padding: 12px 15px;\n"
"    font-size: 14px;\n"
"    color: #333;\n"
"    min-width: 140px;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    border-color: #1e88e5;\n"
"    background-color: #f8fdff;\n"
"}"));

        searchLayout->addWidget(arrivalEdit);

        dateEdit = new QDateEdit(searchFrame);
        dateEdit->setObjectName("dateEdit");
        dateEdit->setCalendarPopup(true);
        dateEdit->setDate(QDate(2025, 11, 24));
        dateEdit->setStyleSheet(QString::fromUtf8("QDateEdit {\n"
"    background: white;\n"
"    border: 2px solid #e0e0e0;\n"
"    border-radius: 8px;\n"
"    padding: 12px 15px;\n"
"    font-size: 14px;\n"
"    color: #333;\n"
"    min-width: 140px;\n"
"}\n"
"\n"
"QDateEdit:focus {\n"
"    border-color: #1e88e5;\n"
"    background-color: #f8fdff;\n"
"}\n"
"\n"
"QDateEdit::drop-down {\n"
"    subcontrol-origin: padding;\n"
"    subcontrol-position: top right;\n"
"    width: 25px;\n"
"    border-left-width: 1px;\n"
"    border-left-color: #e0e0e0;\n"
"    border-left-style: solid;\n"
"    border-top-right-radius: 6px;\n"
"    border-bottom-right-radius: 6px;\n"
"}"));

        searchLayout->addWidget(dateEdit);

        searchButton = new QPushButton(searchFrame);
        searchButton->setObjectName("searchButton");
        searchButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff5722, stop:1 #e64a19);\n"
"    color: white;\n"
"    border: none;\n"
"    border-radius: 8px;\n"
"    padding: 12px 30px;\n"
"    font-size: 16px;\n"
"    font-weight: bold;\n"
"    min-width: 100px;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e64a19, stop:1 #d84315);\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #d84315, stop:1 #c62828);\n"
"}"));

        searchLayout->addWidget(searchButton);


        verticalLayout_2->addLayout(searchLayout);

        filterLayout = new QHBoxLayout();
        filterLayout->setObjectName("filterLayout");
        sortLabel = new QLabel(searchFrame);
        sortLabel->setObjectName("sortLabel");
        sortLabel->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    color: #666;\n"
"    font-size: 13px;\n"
"    padding: 8px 5px;\n"
"}"));

        filterLayout->addWidget(sortLabel);

        sortComboBox = new QComboBox(searchFrame);
        sortComboBox->addItem(QString());
        sortComboBox->addItem(QString());
        sortComboBox->addItem(QString());
        sortComboBox->addItem(QString());
        sortComboBox->setObjectName("sortComboBox");
        sortComboBox->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"    background: white;\n"
"    border: 1px solid #e0e0e0;\n"
"    border-radius: 6px;\n"
"    padding: 8px 12px;\n"
"    font-size: 13px;\n"
"    min-width: 120px;\n"
"}\n"
"\n"
"QComboBox:focus {\n"
"    border-color: #1e88e5;\n"
"}\n"
"\n"
"QComboBox::drop-down {\n"
"    subcontrol-origin: padding;\n"
"    subcontrol-position: top right;\n"
"    width: 20px;\n"
"    border-left-width: 1px;\n"
"    border-left-color: #e0e0e0;\n"
"    border-left-style: solid;\n"
"}"));

        filterLayout->addWidget(sortComboBox);

        airlineLabel = new QLabel(searchFrame);
        airlineLabel->setObjectName("airlineLabel");
        airlineLabel->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    color: #666;\n"
"    font-size: 13px;\n"
"    padding: 8px 5px;\n"
"}"));

        filterLayout->addWidget(airlineLabel);

        airlineComboBox = new QComboBox(searchFrame);
        airlineComboBox->addItem(QString());
        airlineComboBox->addItem(QString());
        airlineComboBox->addItem(QString());
        airlineComboBox->addItem(QString());
        airlineComboBox->addItem(QString());
        airlineComboBox->setObjectName("airlineComboBox");
        airlineComboBox->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"    background: white;\n"
"    border: 1px solid #e0e0e0;\n"
"    border-radius: 6px;\n"
"    padding: 8px 12px;\n"
"    font-size: 13px;\n"
"    min-width: 120px;\n"
"}\n"
"\n"
"QComboBox:focus {\n"
"    border-color: #1e88e5;\n"
"}"));

        filterLayout->addWidget(airlineComboBox);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        filterLayout->addItem(horizontalSpacer_2);


        verticalLayout_2->addLayout(filterLayout);


        verticalLayout->addWidget(searchFrame);

        flightListWidget = new QListWidget(centralwidget);
        flightListWidget->setObjectName("flightListWidget");
        flightListWidget->setStyleSheet(QString::fromUtf8("QListWidget {\n"
"    background: white;\n"
"    border: 1px solid #e0e0e0;\n"
"    border-radius: 8px;\n"
"    margin: 15px;\n"
"    outline: none;\n"
"}\n"
"\n"
"QListWidget::item {\n"
"    border-bottom: 1px solid #f5f5f5;\n"
"    padding: 0px;\n"
"}\n"
"\n"
"QListWidget::item:selected {\n"
"    background-color: #e3f2fd;\n"
"    border: none;\n"
"}\n"
"\n"
"QListWidget::item:hover {\n"
"    background-color: #f8f9fa;\n"
"}"));

        verticalLayout->addWidget(flightListWidget);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\350\210\252\347\217\255\347\256\241\347\220\206\347\263\273\347\273\237", nullptr));
        welcomeLabel->setText(QCoreApplication::translate("MainWindow", "\342\234\210\357\270\217 \350\210\252\347\217\255\347\245\250\345\212\241\347\256\241\347\220\206\347\263\273\347\273\237", nullptr));
        userLabel->setText(QCoreApplication::translate("MainWindow", "\346\254\242\350\277\216\357\274\214\347\224\250\346\210\267", nullptr));
        logoutButton->setText(QCoreApplication::translate("MainWindow", "\351\200\200\345\207\272\347\231\273\345\275\225", nullptr));
        searchTitleLabel->setText(QCoreApplication::translate("MainWindow", "\360\237\224\215 \346\234\272\347\245\250\346\237\245\350\257\242", nullptr));
        departureEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\207\272\345\217\221\345\237\216\345\270\202", nullptr));
        swapButton->setText(QCoreApplication::translate("MainWindow", "\342\207\204", nullptr));
        arrivalEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "\345\210\260\350\276\276\345\237\216\345\270\202", nullptr));
        searchButton->setText(QCoreApplication::translate("MainWindow", "\346\237\245\350\257\242", nullptr));
        sortLabel->setText(QCoreApplication::translate("MainWindow", "\346\216\222\345\272\217:", nullptr));
        sortComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "\345\207\272\345\217\221\346\227\266\351\227\264", nullptr));
        sortComboBox->setItemText(1, QCoreApplication::translate("MainWindow", "\344\273\267\346\240\274\344\273\216\344\275\216\345\210\260\351\253\230", nullptr));
        sortComboBox->setItemText(2, QCoreApplication::translate("MainWindow", "\344\273\267\346\240\274\344\273\216\351\253\230\345\210\260\344\275\216", nullptr));
        sortComboBox->setItemText(3, QCoreApplication::translate("MainWindow", "\351\243\236\350\241\214\346\227\266\351\225\277", nullptr));

        airlineLabel->setText(QCoreApplication::translate("MainWindow", "\350\210\252\347\251\272\345\205\254\345\217\270:", nullptr));
        airlineComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "\346\211\200\346\234\211\350\210\252\347\251\272\345\205\254\345\217\270", nullptr));
        airlineComboBox->setItemText(1, QCoreApplication::translate("MainWindow", "\344\270\255\345\233\275\345\233\275\350\210\252", nullptr));
        airlineComboBox->setItemText(2, QCoreApplication::translate("MainWindow", "\344\270\234\346\226\271\350\210\252\347\251\272", nullptr));
        airlineComboBox->setItemText(3, QCoreApplication::translate("MainWindow", "\345\215\227\346\226\271\350\210\252\347\251\272", nullptr));
        airlineComboBox->setItemText(4, QCoreApplication::translate("MainWindow", "\346\265\267\345\215\227\350\210\252\347\251\272", nullptr));

    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
