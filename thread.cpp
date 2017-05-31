#include "thread.h"
#include "mainwindow.h"

#include <QDebug>
/*
•	QTread提供了一个terminate()函数，该函数可以再一个线程还在运行的时候就终止它的执行，但不推荐用terminate()，
因为terminate()不会立刻终止这个线程，该线程何时终止取决于操作系统的调度策略，也就是说，它可以随时停止线程执行而不给
这个线程自我清空的机会。更安全的方法是用stopped变量和stop()函数，如例子所示。
•	调用setMessage()让第一个线程每隔1秒打印字母“A”，而让第二个线程每隔1秒打印字母“B”。
•	线程会因为调用printf()而持有一个控制I/O的锁，多个线程同时调用printf()在某些情况下回造成控制台输出阻塞，
而用qDebug()作为控制台输出一般不会出现上述问题。


*/
 Thread::Thread()
 {
     stopped = false;

 }

 void Thread::run()//需要循环执行的内容可以放在此函数中
 {

     //-------不进入事件循环---------
     while(!stopped)
     {
         printMessage();
     }

     stopped = false;
     //----------------
 }

 void Thread::stop()
 {
     stopped = true;
 }

 void Thread::setMessage(QString message)
 {
     messageStr = message;
 }

 void Thread::printMessage()
 {
     //emit Add_State("6666666");//发射信号
     emit Timer_OS_Progress_thread();
     emit Timer_COM2_Progress_thread();
     //qDebug()<<messageStr;//写到qtlog.log文件里面
     //sleep(1);
     msleep(200);
 }
