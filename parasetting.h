#ifndef PARASETTING_H
#define PARASETTING_H

#include <QDialog>

namespace Ui {
class Parasetting;
}

class Parasetting : public QDialog
{
    Q_OBJECT

public:
    explicit Parasetting(QWidget *parent = 0);
    ~Parasetting();

private slots:
    void on_pushButton_clicked();

private:
    Ui::Parasetting *ui;
};

#endif // PARASETTING_H
