#include "qsg3fquerymodel.h"
#include <QSqlQueryModel>
#include <QModelIndex>
#include <QDebug>
#include <QColor>

QSg3fQueryModel::QSg3fQueryModel()
{

}

QSg3fQueryModel::~QSg3fQueryModel()
{

}

QVariant QSg3fQueryModel::data(const QModelIndex &index, int role) const
{
  /*
    QVariant value = QSqlQueryModel::data(index,role);
    if (role == Qt::BackgroundRole && index.data(Qt::DisplayRole).toString() =="否")
      {
          qDebug()<<index.row()<<index.column();
          return qVariantFromValue(QColor(Qt::red)); //第一个属性的字体颜色为红色
      }
    //测试
    return value;

    */
    //如果某一行的第二列的值等于"sg3f02"，标记整行背景色为红色
     QVariant value = QSqlQueryModel::data(index, role);

     if (role == Qt::BackgroundRole && QSqlQueryModel::data(this->index(index.row(), 5), Qt::DisplayRole).toString() == "否")
         value = QColor(Qt::red);
     if (role == Qt::BackgroundRole && QSqlQueryModel::data(this->index(index.row(), 5), Qt::DisplayRole).toString() == "是")
         value = QColor(Qt::green);

     return value;

}








