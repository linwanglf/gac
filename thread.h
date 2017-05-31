#ifndef THREAD_H
#define THREAD_H
#include <QThread>
#include <iostream>

 class Thread : public QThread
{
     Q_OBJECT
 public:
     Thread();
     void setMessage(QString message);
     void stop();


 protected:
     void run();
     void printMessage();

 private:
     QString messageStr;
     volatile bool stopped;

     /*
     stopped被声明为易失性变量（volatile variable，断电或中断时数据丢失而不可再恢复的变量类型），
     这是因为不同的线程都需要访问它，并且我们也希望确保它能在任何需要的时候都保持最新读取的数值。
     如果省略关键字volatile，则编译器就会对这个变量的访问进行优化，可能导致不正确的结果
     */

 signals:
     void Add_State(const QString &);
     void Timer_OS_Progress_thread();
     void Timer_COM2_Progress_thread();
 };

#endif // THREAD_H
