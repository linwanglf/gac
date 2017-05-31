#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>
#include <QSqlQuery>

loginDialog::loginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loginDialog)
{
    ui->setupUi(this);

}

loginDialog::~loginDialog()
{
    delete ui;
}

void loginDialog::on_exitpushButton_clicked()
{
    QDialog::reject();
}

void loginDialog::on_loginpushButton_clicked()
{
    if (ui->passwordlineEdit->text().isEmpty()) {
        QMessageBox::information(this, tr("Please enter password"),
                                 tr("Enter password first！"), QMessageBox::Ok);
        ui->passwordlineEdit->setFocus();
    } else {
        QSqlQuery query;
        query.exec("select pwd from password");
        query.next();
        if (query.value(0).toString() == ui->passwordlineEdit->text()) {
        //if ("sg3f"== ui->passwordlineEdit->text()) {
            QDialog::accept();
        } else {
            QMessageBox::warning(this, tr("password wrong"),
                                 tr("Enter password first!"), QMessageBox::Ok);
            ui->passwordlineEdit->clear();
            ui->passwordlineEdit->setFocus();
        }
    }
}
