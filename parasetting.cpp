#include "parasetting.h"
#include "ui_parasetting.h"

Parasetting::Parasetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Parasetting)
{
    ui->setupUi(this);
}

Parasetting::~Parasetting()
{
    delete ui;
}

void Parasetting::on_pushButton_clicked()
{
    ;
}
