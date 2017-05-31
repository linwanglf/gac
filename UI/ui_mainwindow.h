/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGroupBox *groupBox;
    QLineEdit *eqmid3_lineEdit;
    QLabel *eqmid_label;
    QPushButton *reset_pushButton;
    QPushButton *forceacc_pushButton;
    QGroupBox *groupBox_2;
    QTableView *com3_tableView;
    QListWidget *listWidget_start;
    QPushButton *pushButton_B;
    QPushButton *pushButton_quit;
    QGroupBox *groupBox_3;
    QGroupBox *groupBox_4;
    QLabel *label;
    QLineEdit *eqmid4_lineEdit;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QGroupBox *groupBox_5;
    QTableView *com4_tableView;
    QListWidget *listWidget_end;
    QLabel *label_2;
    QLabel *label_4;
    QPushButton *pushButton_A;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1032, 677);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(10, 2, 511, 51));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        eqmid3_lineEdit = new QLineEdit(groupBox);
        eqmid3_lineEdit->setObjectName(QStringLiteral("eqmid3_lineEdit"));
        eqmid3_lineEdit->setGeometry(QRect(70, 10, 141, 31));
        eqmid_label = new QLabel(groupBox);
        eqmid_label->setObjectName(QStringLiteral("eqmid_label"));
        eqmid_label->setGeometry(QRect(10, 13, 51, 30));
        reset_pushButton = new QPushButton(groupBox);
        reset_pushButton->setObjectName(QStringLiteral("reset_pushButton"));
        reset_pushButton->setGeometry(QRect(270, 10, 90, 30));
        forceacc_pushButton = new QPushButton(groupBox);
        forceacc_pushButton->setObjectName(QStringLiteral("forceacc_pushButton"));
        forceacc_pushButton->setGeometry(QRect(420, 10, 80, 31));
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 60, 511, 531));
        sizePolicy.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy);
        com3_tableView = new QTableView(groupBox_2);
        com3_tableView->setObjectName(QStringLiteral("com3_tableView"));
        com3_tableView->setGeometry(QRect(10, 15, 491, 391));
        sizePolicy.setHeightForWidth(com3_tableView->sizePolicy().hasHeightForWidth());
        com3_tableView->setSizePolicy(sizePolicy);
        listWidget_start = new QListWidget(groupBox_2);
        listWidget_start->setObjectName(QStringLiteral("listWidget_start"));
        listWidget_start->setGeometry(QRect(10, 408, 491, 121));
        pushButton_B = new QPushButton(groupBox_2);
        pushButton_B->setObjectName(QStringLiteral("pushButton_B"));
        pushButton_B->setGeometry(QRect(250, -10, 75, 23));
        pushButton_quit = new QPushButton(groupBox_2);
        pushButton_quit->setObjectName(QStringLiteral("pushButton_quit"));
        pushButton_quit->setGeometry(QRect(350, 0, 75, 23));
        groupBox_3 = new QGroupBox(centralWidget);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(520, -1, 511, 591));
        groupBox_4 = new QGroupBox(groupBox_3);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        groupBox_4->setGeometry(QRect(0, 13, 501, 41));
        label = new QLabel(groupBox_4);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 5, 61, 31));
        eqmid4_lineEdit = new QLineEdit(groupBox_4);
        eqmid4_lineEdit->setObjectName(QStringLiteral("eqmid4_lineEdit"));
        eqmid4_lineEdit->setGeometry(QRect(90, 8, 151, 30));
        pushButton = new QPushButton(groupBox_4);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(250, 10, 75, 30));
        pushButton_2 = new QPushButton(groupBox_4);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(410, 10, 75, 30));
        groupBox_5 = new QGroupBox(groupBox_3);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        groupBox_5->setGeometry(QRect(0, 61, 511, 531));
        com4_tableView = new QTableView(groupBox_5);
        com4_tableView->setObjectName(QStringLiteral("com4_tableView"));
        com4_tableView->setGeometry(QRect(11, 15, 491, 391));
        listWidget_end = new QListWidget(groupBox_5);
        listWidget_end->setObjectName(QStringLiteral("listWidget_end"));
        listWidget_end->setGeometry(QRect(10, 410, 491, 121));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(370, 580, 391, 51));
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(450, 592, 311, 51));
        pushButton_A = new QPushButton(centralWidget);
        pushButton_A->setObjectName(QStringLiteral("pushButton_A"));
        pushButton_A->setGeometry(QRect(170, 50, 75, 23));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1032, 23));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        groupBox->setTitle(QApplication::translate("MainWindow", "\345\211\215\345\267\245\345\272\217Com3", 0));
        eqmid_label->setText(QApplication::translate("MainWindow", "\350\256\276\345\244\207ID", 0));
        reset_pushButton->setText(QApplication::translate("MainWindow", "\346\265\213\350\257\225", 0));
        forceacc_pushButton->setText(QApplication::translate("MainWindow", "\345\274\272\345\210\266\346\224\276\350\241\214", 0));
        groupBox_2->setTitle(QApplication::translate("MainWindow", "\345\211\215\345\267\245\345\272\217\345\210\227\350\241\250", 0));
        pushButton_B->setText(QApplication::translate("MainWindow", "Start B", 0));
        pushButton_quit->setText(QApplication::translate("MainWindow", "quit", 0));
        groupBox_3->setTitle(QApplication::translate("MainWindow", "\345\220\216\345\267\245\345\272\217Com4", 0));
        groupBox_4->setTitle(QString());
        label->setText(QApplication::translate("MainWindow", "\350\256\276\345\244\207ID", 0));
        pushButton->setText(QApplication::translate("MainWindow", "\345\244\215\344\275\215\351\207\215\350\257\273", 0));
        pushButton_2->setText(QApplication::translate("MainWindow", "\345\274\272\345\210\266\346\224\276\350\241\214", 0));
        groupBox_5->setTitle(QApplication::translate("MainWindow", "\345\220\216\345\267\245\345\272\217\345\210\227\350\241\250", 0));
        label_2->setText(QApplication::translate("MainWindow", "\347\211\210\346\235\203\346\211\200\346\234\211 \345\271\277\346\261\275\344\274\240\347\245\272\345\212\250\345\212\233\346\200\273\346\210\220\346\234\254\351\203\250  3F\345\274\200\345\217\221\345\233\242\351\230\237--\346\236\227\345\260\232\350\212\263", 0));
        label_4->setText(QApplication::translate("MainWindow", "Windows & Linux \350\267\250\345\271\263\345\217\260", 0));
        pushButton_A->setText(QApplication::translate("MainWindow", "Start A", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
