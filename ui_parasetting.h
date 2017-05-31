/********************************************************************************
** Form generated from reading UI file 'parasetting.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARASETTING_H
#define UI_PARASETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Parasetting
{
public:
    QPushButton *pushButton;

    void setupUi(QDialog *Parasetting)
    {
        if (Parasetting->objectName().isEmpty())
            Parasetting->setObjectName(QStringLiteral("Parasetting"));
        Parasetting->resize(400, 300);
        pushButton = new QPushButton(Parasetting);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(120, 140, 75, 23));

        retranslateUi(Parasetting);

        QMetaObject::connectSlotsByName(Parasetting);
    } // setupUi

    void retranslateUi(QDialog *Parasetting)
    {
        Parasetting->setWindowTitle(QApplication::translate("Parasetting", "Dialog", 0));
        pushButton->setText(QApplication::translate("Parasetting", "\346\226\260\347\252\227\345\217\243", 0));
    } // retranslateUi

};

namespace Ui {
    class Parasetting: public Ui_Parasetting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARASETTING_H
