/********************************************************************************
** Form generated from reading UI file 'logindialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINDIALOG_H
#define UI_LOGINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_loginDialog
{
public:
    QLabel *label_2;
    QLineEdit *passwordlineEdit;
    QPushButton *loginpushButton;
    QPushButton *exitpushButton;
    QLabel *label_3;
    QLabel *label_4;

    void setupUi(QDialog *loginDialog)
    {
        if (loginDialog->objectName().isEmpty())
            loginDialog->setObjectName(QStringLiteral("loginDialog"));
        loginDialog->resize(400, 300);
        label_2 = new QLabel(loginDialog);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(80, 120, 54, 12));
        passwordlineEdit = new QLineEdit(loginDialog);
        passwordlineEdit->setObjectName(QStringLiteral("passwordlineEdit"));
        passwordlineEdit->setGeometry(QRect(150, 120, 151, 20));
        passwordlineEdit->setEchoMode(QLineEdit::Password);
        loginpushButton = new QPushButton(loginDialog);
        loginpushButton->setObjectName(QStringLiteral("loginpushButton"));
        loginpushButton->setGeometry(QRect(150, 180, 75, 23));
        exitpushButton = new QPushButton(loginDialog);
        exitpushButton->setObjectName(QStringLiteral("exitpushButton"));
        exitpushButton->setGeometry(QRect(230, 180, 75, 23));
        label_3 = new QLabel(loginDialog);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(50, 240, 311, 51));
        label_4 = new QLabel(loginDialog);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(130, 260, 311, 51));

        retranslateUi(loginDialog);

        QMetaObject::connectSlotsByName(loginDialog);
    } // setupUi

    void retranslateUi(QDialog *loginDialog)
    {
        loginDialog->setWindowTitle(QApplication::translate("loginDialog", "\347\231\273\345\275\225", 0));
        label_2->setText(QApplication::translate("loginDialog", "\345\257\206\347\240\201", 0));
        loginpushButton->setText(QApplication::translate("loginDialog", "\347\231\273\345\275\225", 0));
        exitpushButton->setText(QApplication::translate("loginDialog", "\351\200\200\345\207\272", 0));
        label_3->setText(QApplication::translate("loginDialog", "\347\211\210\346\235\203\346\211\200\346\234\211 \345\271\277\346\261\275\344\274\240\347\245\272\345\212\250\345\212\233\346\200\273\346\210\220\346\234\254\351\203\250  3F\345\274\200\345\217\221\345\233\242\351\230\237--\346\236\227\345\260\232\350\212\263", 0));
        label_4->setText(QApplication::translate("loginDialog", "Windows & Linux \350\267\250\345\271\263\345\217\260", 0));
    } // retranslateUi

};

namespace Ui {
    class loginDialog: public Ui_loginDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINDIALOG_H
