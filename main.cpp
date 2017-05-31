#include "mainwindow.h"
#include "database/connect.h"
#include "logindialog.h"
#include <QApplication>
#include <QDebug>
#include <QMessageBox>


void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;

    case QtWarningMsg:
        text = QString("Warning:");
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        break;

    case QtFatalMsg:
        text = QString("Fatal:");
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString current_date = QString("(%1)").arg(current_date_time);
    QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);

    QFile file("gacfile/qtlog.log");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();

    mutex.unlock();
}


int main(int argc, char *argv[])
{


    //--------------线程测试begin-----------
/*
•	在GUI程序中，主线程也被称为GUI线程，因为它是唯一一个允许执行GUI相关操作的线程。
必须在创建一个QThread之前创建QApplication对象。
*/
   // QApplication app(argc, argv);
   // ThreadDialog *threaddialog = new ThreadDialog;
   // threaddialog->exec();
   // return app.exec();

    //---------------线程测试end----------
    QApplication a(argc, argv);
    qInstallMessageHandler(outputMessage);

   // QMessageBox::aboutQt(NULL, "About Qt");
    if(!createConnection() ) return 0;//这里闪退出去了
    MainWindow w;//生成mainwindow界面窗口
    //loginDialog login;//生成loginDialog界面窗口

    w.show();
    return a.exec();
    /*
    if (login.exec() == QDialog::Accepted) {//登录界面
        // w.show();
        //创建线程ui窗口begin-----
        //ThreadDialog *threaddialog = new ThreadDialog;
        //threaddialog->exec();
        //创建线程ui窗口end-----

        return a.exec();
    } else {
         qDebug()<<"exit ";
        return 0;
    }
    */


}
