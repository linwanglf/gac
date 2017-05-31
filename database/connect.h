#ifndef CONNECT
#define CONNECT
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSettings>
#include <QMessageBox>


static bool createConnection()
{

    QSettings setting("gacfile/gacconfig.ini", QSettings::IniFormat);
    setting.beginGroup("dbconfig");
    QString strHost = setting.value("host").toString();
    qDebug()<<"strHost="<<strHost;
    QString strDbname = setting.value("dbname").toString();
    qDebug()<<"strDbname="<<strDbname;

    QString strUsername = setting.value("username").toString();
    qDebug()<<"strUsername="<<strUsername;
    QString strPasswd = setting.value("password").toString();
    qDebug()<<"strPasswd="<<strPasswd;

    setting.endGroup();

  /* 连接Mysql数据库*/
//    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
//    db.setHostName(strHost);
//    db.setDatabaseName(strDbname);
//    db.setUserName(strUsername);
//    db.setPassword(strPasswd);

    /* 切换为SQLIte数据库,还没有测试*/
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("gacsqlite.db");

    qDebug("This is a debug message");
    if(!db.open()){
        qDebug()<<"Unable to open database";
        return false;
    }else{
        qDebug()<<"Database connection established";
        return true;
    }

}

#endif // CONNECT

