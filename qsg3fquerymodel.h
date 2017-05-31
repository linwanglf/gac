#ifndef QSG3FQUERYMODEL_H
#define QSG3FQUERYMODEL_H

#include <QObject>
#include <QSqlQueryModel>

class QSg3fQueryModel : public QSqlQueryModel
{
public:
    QSg3fQueryModel();
    ~QSg3fQueryModel();
    //重构data函数
    QVariant data(const QModelIndex &index,int role=Qt::DisplayRole) const;
};

#endif // QSG3FQUERYMODEL_H
