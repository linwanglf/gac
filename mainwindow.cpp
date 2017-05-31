#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qsg3fquerymodel.h"
#include "comvar.h"
#include "parasetting.h"
#include <QtSql>
#include <QMessageBox>
#include <QTableView>
#include <QAbstractItemView>
#include <QPalette>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSettings>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    threadA.setMessage("A");
    threadB.setMessage("B");

    initialCom_Var();//初始化公用变量
    initialConfiguration();//读取配置文档的内容初始化IR_Infor.ControlType等等变量    
    InitIR_RFID_ComArray(IR_Infor.ControlType);//读取数据表中的内容,初始化与本工序相关的作业设定rfid


    initComm2();//初始化串口2
    initComm3();//初始化串口3
    initComm4();//初始化串口4
    initialTimer();//初始化定时器，实现类同delphi平台结构

    //connect(&threadA, SIGNAL(Add_State(const QString)), this, SLOT(AddState(const QString)));

    connect(&threadA, SIGNAL(Timer_OS_Progress_thread()),this,SLOT(Timer_OS_Progress()) );
    connect(&threadB, SIGNAL(Timer_COM2_Progress_thread()),this,SLOT(Timer_COM2_Progress()) );
    startOrStopThreadA();//启动线程
    startOrStopThreadB();//启动线程

}

//初始化公用变量
void MainWindow::initialCom_Var()
{
    qint32 i_count;

     this->Start_stationNO=3;
     this->End_stationNO=4;
     CMD_COUMUNICATION.CMD_IDRec_CAN="AA";
     CMD_COUMUNICATION.CMD_IDSend_CAN="55";
     CMD_COUMUNICATION.CMD_READ = "43";
     RFID_CAN_MaxStationNO=0;
     strEnd_stationNO="04";
     strStart_stationNO="03";
     CAN_COM="02";
     strtemp_NG="否";
     strtemp_OK="是";
     bool_ok=true;
     bool_ng=false;
     PC_ANDON_WriteFIN="00";
     PC_ANDON_WriteFIN_Second="00";
        //用于判断是否已经成功读取一次的记忆变量
     ReadIDValue_03_old="00";
     ReadIDValue_04_old="00";
     TEST1="1";
     TEST2="1";
     COUNT_NO=0;
     COUNT_NO2=0;
     Prg_Total=16;//控制器可以使用的程序

     PC_TC_BarCodeCheck="00";

     IR_Infor.filepath_01win02linux= "02";//配置文件的 路径，01windons平台，02linux平台

     for(i_count=0;i_count<i_recv_total;i_count++)
        {
          recData_from_COM2[i_count]="**";
        }

}

//定时器每隔1000ms，不断扫描作业进度，并在todo_list_tableview上刷新展示
void MainWindow::scanJobProgress()
{
    /*
    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(refreshJobProgress()) );
    timer->start(this->strScanjobprogresstimer.toInt());
    */

    refreshJobProgress();

}

//根据IP,端口展示此站点的作业进度  展示的Model定制化
void MainWindow ::refreshJobProgress()
{

    QSg3fQueryModel * todolist_model =  new QSg3fQueryModel(); //model需要new一个对象,不能直接使用
    QString strSql = "select ir_ip_add,"
                     " ir_rfid_no,"
                     " ir_egtype_no,"
                     " ir_station_no,"
                     " ir_partlist_name,"
                     " ir_finished "
                     " from 3f_ir_run "
                     " where id in ( select min(id) from 3f_ir_run where ir_ip_add = '" +IR_Infor.IP_ADD +
                     " ') ";
    qDebug()<<strSql;
    todolist_model->setQuery(strSql);
    todolist_model->setHeaderData(0,Qt::Horizontal,tr("ir_ip_add"));
    todolist_model->setHeaderData(1,Qt::Horizontal,tr("ir_rfid_no"));
    todolist_model->setHeaderData(2,Qt::Horizontal,tr("ir_egtype_no"));
    todolist_model->setHeaderData(3,Qt::Horizontal,tr("ir_station_no"));
    todolist_model->setHeaderData(4,Qt::Horizontal,tr("ir_partlist_name"));
    todolist_model->setHeaderData(5,Qt::Horizontal,tr("ir_finished"));


    ui->com3_tableView->setModel(todolist_model); //view使用界面定义的控件
}

//初始化定时器，实现类同delphi平台结构
void MainWindow::initialTimer()
{
    //QTimer *Timer_init = new QTimer(this);
    //timeout表示周期到，SLOT里的函数表示触发做什么
    Timer_init = new QTimer(this);
    connect(this->Timer_init,SIGNAL(timeout()),this,SLOT(Timer_initProgress()));
    this->Timer_init->start(this->timeinitInterval.toInt());//启动定时器，类同delphi的Timer_init.Enabled := True;//启动循环扫描

    //QTimer *Timer_RFID_ReadStartCycle_Start = new QTimer(this); //前工序读卡触发定时器
    Timer_RFID_ReadStartCycle_Start = new QTimer(this);
    connect(this->Timer_RFID_ReadStartCycle_Start,SIGNAL(timeout()),this,SLOT(Timer_RFIDStart_Progress()) );

    //QTimer *Timer_RFID_ReadStartCycle_End = new QTimer(this);  //后工序读卡触发定时器
    Timer_RFID_ReadStartCycle_End = new QTimer(this);
    connect(this->Timer_RFID_ReadStartCycle_End,SIGNAL(timeout()),this,SLOT(Timer_RFIDEnd_Progress()) );

    //QTimer *timeOS = new QTimer(this); //程序循环触发定时器
    timeOS = new QTimer(this);
    //connect(this->timeOS,SIGNAL(timeout()),this,SLOT(Timer_OS_Progress()) );

    time_com2 = new QTimer(this);
    connect(this->time_com2,SIGNAL(timeout()),this,SLOT(readMyCom2()) );

}


//读取配置文档的内容
void MainWindow::initialConfiguration()
{
    if (IR_Infor.filepath_01win02linux=="01")
    {
        QSettings setting("D:/ubuntu14share/qtprojects/gac/gacfile/gacconfig.ini", QSettings::IniFormat);
        setting.beginGroup("comconfig");

        this->strCom1 = setting.value("com1").toString();
        this->strBaudRate1 = setting.value("baudrate1").toString();
        this->strCom2 = setting.value("com2").toString();
        this->strBaudRate2 = setting.value("baudrate2").toString();
        this->strCom3 = setting.value("com3").toString();
        this->strBaudRate3 = setting.value("baudrate3").toString();
        this->strCom4 = setting.value("com4").toString();
        this->strBaudRate4 = setting.value("baudrate4").toString();


        this->strLocalIP = setting.value("localip").toString();
        qDebug()<<"localip :"<<this->strLocalIP << "strCom"<<this->strCom2 <<"baudrate"<<this->strBaudRate2 ;
        setting.endGroup();
       //-----------------------------
        setting.beginGroup("timmer");
        this->strScanjobprogresstimer = setting.value("scanjobprogresstimer").toString();//从配置文档中读取循环间隔时间
        this->timeinitInterval= setting.value("timeinitInterval").toString();//从配置文档中读取循环间隔时间
        this->Timer_RFID_ReadStartCycle_StartInterval = setting.value("Timer_RFID_ReadStartCycle_StartInterval").toString();//从配置文档中读取前工序读卡循环间隔时间
        this->Timer_RFID_ReadStartCycle_EndInterval= setting.value("Timer_RFID_ReadStartCycle_EndInterval").toString();//从配置文档中读取后工序读卡循环间隔时间
        this->timeOSInterval= setting.value("timeOSInterval").toString();//从配置文档中读取程序循环间隔时间


        qDebug()<<"strScanjobprogresstimer :"<<this->strScanjobprogresstimer;

        setting.endGroup();
       //-------------------------------
        setting.beginGroup("ir");
        IR_RFID.IR_RFID_Type= setting.value("RFIDtype").toString();//从配置文档中读取设定的rfid类型,1为3f,2为omron
        IR_Infor.ControlType= setting.value("ControlType").toString();//从配置文档中读取设定的控制范围类型,01为单机,00为整线
        IR_Infor.IP_ADD= setting.value("IR_IP").toString();//从配置文档中读取设定的联锁IP
        IR_Infor.ControlforLine_Manu_Auto= setting.value("ControlModel_Manu_Auto").toString();//从配置文档中读取设定的控制线手动,自动
        IR_Infor.StrCheckTotalPLC= setting.value("CheckTotalPLCConnect").toString();//是否检测plc在线,默认=00,不检测
        IR_Infor.PC_DB_IO= setting.value("01DB02IO").toString();//检测是通过数据库复位还是通过io复位
        IR_Infor.Current_EGtypeFlow_first= setting.value("Process03_EGTypeFlow").toString();//读取当前作业联锁的信息
        IR_Infor.Current_EGtypeFlow_second= setting.value("Process04_EGTypeFlow").toString();//读取当前作业联锁的信息


        IR_Infor.StrReset_Valid= setting.value("StrReset_Valid").toString();//读取是否启用复位开关信号的信息
        IR_Infor.StrReset_DelRecord= setting.value("StrReset_DelRecord").toString();//读取接收到复位开关的信号时是否清除记录的信息
        //IR_Infor.filepath_01win02linux= setting.value("StrReset_DelRecord").toString();//读取接收到复位开关的信号时是否清
        setting.endGroup();
    }
    else if (IR_Infor.filepath_01win02linux=="02")
    {
        QSettings setting("gacfile/gacconfig.ini", QSettings::IniFormat);
        setting.beginGroup("comconfig");

        this->strCom1 = setting.value("com1").toString();
        this->strBaudRate1 = setting.value("baudrate1").toString();
        this->strCom2 = setting.value("com2").toString();
        this->strBaudRate2 = setting.value("baudrate2").toString();
        this->strCom3 = setting.value("com3").toString();
        this->strBaudRate3 = setting.value("baudrate3").toString();
        this->strCom4 = setting.value("com4").toString();
        this->strBaudRate4 = setting.value("baudrate4").toString();


        this->strLocalIP = setting.value("localip").toString();
        qDebug()<<"localip :"<<this->strLocalIP << "strCom"<<this->strCom2 <<"baudrate"<<this->strBaudRate2 ;
        setting.endGroup();
       //-----------------------------
        setting.beginGroup("timmer");
        this->strScanjobprogresstimer = setting.value("scanjobprogresstimer").toString();//从配置文档中读取循环间隔时间
        this->timeinitInterval= setting.value("timeinitInterval").toString();//从配置文档中读取循环间隔时间
        this->Timer_RFID_ReadStartCycle_StartInterval = setting.value("Timer_RFID_ReadStartCycle_StartInterval").toString();//从配置文档中读取前工序读卡循环间隔时间
        this->Timer_RFID_ReadStartCycle_EndInterval= setting.value("Timer_RFID_ReadStartCycle_EndInterval").toString();//从配置文档中读取后工序读卡循环间隔时间
        this->timeOSInterval= setting.value("timeOSInterval").toString();//从配置文档中读取程序循环间隔时间


        qDebug()<<"strScanjobprogresstimer :"<<this->strScanjobprogresstimer;

        setting.endGroup();
       //-------------------------------
        setting.beginGroup("ir");
        IR_RFID.IR_RFID_Type= setting.value("RFIDtype").toString();//从配置文档中读取设定的rfid类型,1为3f,2为omron
        IR_Infor.ControlType= setting.value("ControlType").toString();//从配置文档中读取设定的控制范围类型,01为单机,00为整线
        IR_Infor.IP_ADD= setting.value("IR_IP").toString();//从配置文档中读取设定的联锁IP
        IR_Infor.ControlforLine_Manu_Auto= setting.value("ControlModel_Manu_Auto").toString();//从配置文档中读取设定的控制线手动,自动
        IR_Infor.StrCheckTotalPLC= setting.value("CheckTotalPLCConnect").toString();//是否检测plc在线,默认=00,不检测
        IR_Infor.PC_DB_IO= setting.value("01DB02IO").toString();//检测是通过数据库复位还是通过io复位
        IR_Infor.Current_EGtypeFlow_first= setting.value("Process03_EGTypeFlow").toString();//读取当前作业联锁的信息
        IR_Infor.Current_EGtypeFlow_second= setting.value("Process04_EGTypeFlow").toString();//读取当前作业联锁的信息
        IR_Infor.StrReset_Valid= setting.value("StrReset_Valid").toString();//读取是否启用复位开关信号的信息
        IR_Infor.StrReset_DelRecord= setting.value("StrReset_DelRecord").toString();//读取接收到复位开关的信号时是否清除记录的信息
        //IR_Infor.filepath_01win02linux= setting.value("StrReset_DelRecord").toString();//读取接收到复位开关的信号时是否清
        setting.endGroup();
    }




    //ui->eqmid3_lineEdit->setText(IR_Infor.Current_EGtypeFlow_first);//显示在机型文本框
    //ui->eqmid4_lineEdit->setText(IR_Infor.Current_EGtypeFlow_second);//显示在机型文本框


    if(IR_RFID.IR_RFID_Type=="1")
      {
       AddState("RFID类型设定:SIGE");
       AddState2("RFID类型设定:SIGE");
      }
    else if  (IR_RFID.IR_RFID_Type=="2")
          {
       AddState("RFID类型设定:OMRON");
       AddState2("RFID类型设定:OMRON");
      }
    this->AddState("本机控制IP:"+IR_Infor.IP_ADD+"");
    this->AddState2("本机控制IP:"+IR_Infor.IP_ADD+"");


}

void MainWindow::OMRON_RFID_ReadAutoCycle(const qint32 IntRFID_NO)
{
   QString strtemp_hex="";
   QString strtempr_RFID_NO="00";
   qint32 i=0;

    if  (IR_RFID.IR_RFID_Type=="3")//条码枪
    {

        //exit(0);//退出应用程序
        return;
    }
    else if  (IR_RFID.IR_RFID_Type=="2")//rfid为OMRON
    {

       i=IntRFID_NO;
       if (i==3)
       {
         //this->AddState("'"+IR_CAN_RFID_Arr[i].IR_RFID_ReadSta+"'");
         strtempr_RFID_NO="03";
        }
        else if (i==4)
       {
        //this->AddState2("'"+IR_CAN_RFID_Arr[i].IR_RFID_ReadSta+"'");
        strtempr_RFID_NO="04";
       }



        //判断是否处于读卡完毕状态IR_CAN_RFID_Arr[i].IR_RFID_ReadSta:='01'，如果不是则让
            //卡头处于等待读ID的状态 IR_CAN_RFID_Arr[i].IR_RFID_ReadSta='00'


            if (IR_CAN_RFID_Arr[i].IR_RFID_ReadSta=="00")
            {
               strtemp_hex=ReadMDS_OMRON_BySpcom("0010","0020");//读ID卡,相关操作间串口部分程序
               ReadWrite_RFID_BySpcom(strtempr_RFID_NO,strtemp_hex);

               IR_CAN_RFID_Arr[i].IR_RFID_ReadSta="01";//处于读卡状态


            }

   }//RFID类型为OMRON
}

//写ID，使用
QString MainWindow::WriteMDS_OMRON_BySpcom(const QString strAddr,const QString strv)
{
  QString strTemp="";

  strTemp="WTSAA1"+strAddr+strv+"*"+toascii(13);
  strTemp=ChangeAreaStrToHEX(strTemp);   //读ID
  return strTemp;
}

//读ID，轴瓦选配用
QString MainWindow::ReadMDS_OMRON_BySpcom(const QString mdsAddr,const QString MDSLength)
{
  QString strTemp="";

  strTemp="RDSAA1"+mdsAddr+MDSLength+"*"+toascii(13);
  //strTemp= ChangeAreaStrToHEX(strTemp);   //读ID
  return strTemp;

}

//将字符串转化为串口能够发送的16进制字符串（如要发送“123”，分解为“1”“2”“3”，
//对应16进制转换为“31”“32”“33”，综合为“313233”）
QString MainWindow::ChangeAreaStrToHEX(const QString arr)
{
  QString reTmpStr="";
  reTmpStr=arr.toLocal8Bit().toHex();
  reTmpStr=reTmpStr.toUpper();
    return reTmpStr;
}

QString MainWindow::ChangeAreaHEXToStr_QByteArray(QByteArray arr)
{
  QString reTmpStr="";
  QByteArray text =QByteArray::fromHex(arr);//----这样用变量就
  reTmpStr=text.data();

  return reTmpStr;
}
//将16进制的QString字符串"313233"转换为asii码字符串"123",这样是否可以?还是有其他现有函数
QString MainWindow::ChangeAreaHEXToStr(QString arr)
{
  QString reTmpStr="";
  QByteArray reTmpStr2;
  reTmpStr2=arr.toLocal8Bit();//QString转换为QByteArray
  reTmpStr=ChangeAreaHEXToStr_QByteArray(reTmpStr2);//调用函数转换asii

   return reTmpStr;
/*
 qint32 i;
 QString reTmpStr="";
 QByteArray reTmpStr2="";
 reTmpStr=reTmpStr2.data();//QByteArray转换为QString
 reTmpStr2=arr.toLocal8Bit();//QString转换为QByteArray
 QString str_hex;
 qint32 int_dex;
 if ((arr.length()/2)>0)
 {
   for(i=0;i<(arr.length()/2);i++)
  {

    str_hex=arr.mid(2*i,2);//截取两个字符
    int_dex=str_hex.toInt();//转换为10制
    if (i=0)
    {
     reTmpStr=toascii(int_dex);//转化为asii码
    }
    else
    {
     reTmpStr=reTmpStr+toascii(int_dex);
    }
  }

 }
  return reTmpStr;
 */


}

//函 数 名：AscToHex()
 //功能描述：把ASCII转换为16进制
 unsigned char MainWindow::AscToHex(unsigned char aHex)
 {
    if((aHex<=9))
        aHex += 0x30;
    else if((aHex>=10)&&(aHex<=15))//A-F
        aHex += 0x37;
    else
        aHex = 0xff;
    return aHex;
 }

    //函 数 名：HexToAsc()
 //功能描述：把16进制转换为ASCII
unsigned char MainWindow::HexToAsc(unsigned char aChar)
{
    if((aChar>=0x30)&&(aChar<=0x39))
        aChar -= 0x30;
    else if((aChar>=0x41)&&(aChar<=0x46))//大写字母
        aChar -= 0x37;
    else if((aChar>=0x61)&&(aChar<=0x66))//小写字母
        aChar -= 0x57;
    else
        aChar = 0xff;
    return aChar;
}

//十进制 to 16位二进制
QString MainWindow::IntToBin_ok(const qint32 Value,const qint32 Size)
{
   QString strtemp;
   qint32 i,length;

   strtemp=strtemp.setNum(Value,2);
   length=strtemp.length();
   for(i=0;i<Size-length;i++)
   {

       strtemp='0'+strtemp;
   }

  return strtemp;
}

//向RFID发送指令，以读取ID中相关的信息
void MainWindow::ReadWrite_RFID_BySpcom(const QString str_RFID_NO,const QString str_CMD_Value)
{
 QString strValue="";
 qint32 i=0;

 if ((str_RFID_NO=="01" )|(str_RFID_NO=="02" ) |(str_RFID_NO=="03" ) |(str_RFID_NO=="04" )|(str_RFID_NO=="05" )| (str_RFID_NO=="06" ))
{

   i=str_RFID_NO.toInt();
   if (IR_RFID.IR_RFID_Type=="1")     //3F的RFID
      {
       /*
       strValue=str_CMD_Value;
       orderLst_CAN_RFID_COM[i].append(strValue);
       sendData_CAN_RFID_BySpcom(i);
       */
       }
   else if (IR_RFID.IR_RFID_Type=="2")  //OMRON的RFID
    {
       strValue=str_CMD_Value;
       orderLst_CAN_RFID_COM[i]=strValue;
       sendData_CAN_RFID_BySpcom(i);
    }
 }
}

//串口2发送数据给IO模块
void MainWindow::sendData_CAN_RFID_BySpcom_IO(const qint32 IntRFID_NO)
{
   QString  orderStr="";
   orderStr=orderLst_CAN_RFID_COM[IntRFID_NO];
   sendMsg_IO(IntRFID_NO,orderStr);
}

//发送数据
void MainWindow::sendMsg_IO(const qint32 IntRFID_NO,const QString str)
{

    QByteArray buf;
    //if(ui->sendAsHexcheckBox->isChecked())
    {
        //QString str;
        bool ok;
        char data;
        QStringList list;
        list = str.split(" ");
        for(int i = 0; i < list.count(); i++){
            if(list.at(i) == " ")
                continue;
            if(list.at(i).isEmpty())
                continue;
            data = (char)list.at(i).toInt(&ok, 16);
            if(!ok){
                QMessageBox::information(this, tr("提示消息"), tr("输入的数据格式有错误！"), QMessageBox::Ok);
                if(obotimer != NULL)
                    obotimer->stop();
                return;
            }
            buf.append(data);
        }
    }
/*
    else{
#if QT_VERSION < 0x050000
        buf = str.toAscii();
#else
        buf = str.toLocal8Bit();
#endif
    }
*/

       //发送数据
        if (IntRFID_NO==1)
        {

        }
        else if (IntRFID_NO==2)
        {
            this->AddState("com2=>>:"+str+"");
            this->AddState2("com2=>>:"+str+"");
            myCom2->write(buf);
        }
        else if (IntRFID_NO==3)
        {
            this->AddState("com3=>>:"+str+"");
            myCom3->write(buf);
        }
        else if (IntRFID_NO==4)
        {
            this->AddState2("com4=>>:"+str+"");
            myCom4->write(buf);
        }
        else if (IntRFID_NO==5)
        {

        }
        else if (IntRFID_NO==6)
        {

        }
}

 //串口发送数据
void MainWindow::sendData_CAN_RFID_BySpcom(const qint32 IntRFID_NO)
{
    QString  orderStr="";
    orderStr=orderLst_CAN_RFID_COM[IntRFID_NO];
    sendMsg(IntRFID_NO,orderStr);
    //sendMsg(IntRFID_NO,"4141212121");//太长报错,这里没有弄明白,难道是因为整数长度不能超过的极限么?
    //sendMsg(IntRFID_NO,"abcdefghijk");
}


//---------------------------------------------------------------------
//发送数据
void MainWindow::sendMsg(const qint32 IntRFID_NO,const QString str)
{
    QByteArray buf;
   // QStringList list;
   // QString tempstr;


        /*
        list = str.split(" ");
        for(int i = 0; i < list.count(); i++)
        {
            if(list.at(i) == " ")
                continue;
            if(list.at(i).isEmpty())
                continue;
            data = (char)list.at(i).toInt(&ok, 16);
            if(!ok){
                QMessageBox::information(this, tr("提示消息"), tr("输入的数据格式有错误！"), QMessageBox::Ok);
                if(obotimer != NULL)//设置连续发送计时器
                    obotimer->stop();
                return;
            }
            buf.append(data);

        }
        *///如果str=("4141212121");//太长报错,这里没有弄明白,难道是因为整数长度不能超过的极限么?


        //buf.append(str.toLocal8Bit());//将16进制数据发送
buf.append(str);//将16进制数据发送
       //发送数据
        if (IntRFID_NO==1)
        {

        }
        else if (IntRFID_NO==2)
        {

            this->AddState("com2=>>:"+str+"");
            this->AddState2("com2=>>:"+str+"");
            myCom2->write(buf);
        }
        else if (IntRFID_NO==3)
        {
         //   this->AddState("com3=>>:"+str+"");
            myCom3->write(buf);
        }
        else if (IntRFID_NO==4)
        {
         //   this->AddState2("com4=>>:"+str+"");
            myCom4->write(buf);
        }
        else if (IntRFID_NO==5)
        {

        }
        else if (IntRFID_NO==6)
        {

        }
}

//---------------------------------------------------------------------


void MainWindow::Timer_initProgress()
{
    this->Timer_init->stop();//关闭定时器
      if (IR_Infor.StrCheckTotalPLC=="01")//检测连接的PLC是否在线默认=00
      {

                 AddState("'"+IR_Infor.StrCheckTotalPLC+"'号站发复位指令");
                 AddState2("'"+IR_Infor.StrCheckTotalPLC+"'号站发复位指令");
                 //Panel_PLCStatus.Color:=clGreen
                 //StartWork;//delphi版本的启动线程

                 this->timeOS->start(this->timeOSInterval.toInt());//启动定时器，
                 this->time_com2->start(100);//启动定时器，

                  if (IR_RFID.IR_RFID_Type=="2")  //omron
                  {
                      this->Timer_RFID_ReadStartCycle_Start->start(this->Timer_RFID_ReadStartCycle_StartInterval.toInt());//启动定时器，
                      this->Timer_RFID_ReadStartCycle_End->start(this->Timer_RFID_ReadStartCycle_EndInterval.toInt());//启动定时器，
                   }

        }
      else   if (IR_Infor.StrCheckTotalPLC=="00")
        {

          AddState("'"+IR_Infor.StrCheckTotalPLC+"'号站发复位指令");
          AddState2("'"+IR_Infor.StrCheckTotalPLC+"'号站发复位指令");
                 // Panel_PLCStatus.Color:=clGreen;
                 //StartWork;//delphi版本的启动线程

                 this->timeOS->start(this->timeOSInterval.toInt());//启动定时器，
                 this->time_com2->start(100);//启动定时器，

                 if (IR_RFID.IR_RFID_Type=="2") //omron
                 {
                     this->Timer_RFID_ReadStartCycle_Start->start(this->Timer_RFID_ReadStartCycle_StartInterval.toInt());//启动定时器，
                     this->Timer_RFID_ReadStartCycle_End->start(this->Timer_RFID_ReadStartCycle_EndInterval.toInt());//启动定时器，
                 }

       }


}

void MainWindow::Timer_RFIDStart_Progress()
{
    OMRON_RFID_ReadAutoCycle(this->Start_stationNO);

}

void MainWindow::Timer_RFIDEnd_Progress()
{
    OMRON_RFID_ReadAutoCycle(this->End_stationNO);
}


void MainWindow::Timer_OS_Progress()
{
   // AddState("6666666");//发射信号
    ///*
    //if(CheckPLCOK)//20130729临时屏蔽
         {
           //调用主界面中读取PLC的运行状态函数

           //4、作业联锁 完成的查询及处理(包括零件指示、拧紧作业联锁)
                RequestReadIDFinished_CAN_Total();
           //5、将读取的机型、流水、分组记录发送给PLC

           //6、系统启动时读取联锁记录表3F_IR_RUN中的记录显示处理
                 if (IR_Infor.ControlType=="01")
                     {
                        if (TEST1=="1")
                           {
                             TEST1="2";
                             Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_first,strStart_stationNO,IR_Infor.IP_ADD);
                             Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_second,strEnd_stationNO,IR_Infor.IP_ADD);

                           }
                        if (COUNT_NO==1)
                           {
                             COUNT_NO=2;
                             //Timer1.Enabled:=TRUE;
                             Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_first,strStart_stationNO,IR_Infor.IP_ADD);
                           }
                        if (COUNT_NO2==1)
                           {
                             COUNT_NO2=2;
                             //Timer2.Enabled=true;
                             Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_second,strEnd_stationNO,IR_Infor.IP_ADD);
                           }

                    }
                 else
                    {

                    ;
                    }

        //8、作业延迟报警检测 (自动线时使用)
               if (IR_Infor.ControlforLine_Manu_Auto=="02")//02自动线
                  {
                    Alram_for_CycleArraved();
                  }
        //10、判断料架ID与当前托盘ID是否为同一机型

               //Check_PickBoxID_DeskTableID;

        //12、检测ANDON总控PC是否已经将作业完成情况发送PLC

        //
               //QueryFinFromPC_ResetFromPLC;
          //此段内加上流程   结束
       }
       //*/
}


//线体上的ID是统一管理的情况时，读取各工位“读卡完毕信号”
void MainWindow::RequestReadIDFinished_CAN_Total()
{
    QString Record_ID,itemp;
    qint32 i;
    for(i=Start_stationNO;i<End_stationNO+1;i++)
     {
       if (IR_CAN_RFID_Arr[i].RFID_Station_NO!="")
        {



        //判断该RFID是否用于绑定作业联锁的RFID
         if (IR_CAN_RFID_Arr[i].IR_IO_Type=="02" )       //(01只是读写机型给PLC的RFID，02-绑定作业联锁的RFID)
          {

        //需要追加在联锁作业设定表[3F_IR_INPUT]中是否有设定记录,字段[IR_RFID_No],如果有
        //则进行相关查询操作,IR_Infor.IP_ADD为当前工序配置的IP号

          //0开始---------------

           if (QueryIR_INPUT_LIST_BE(IR_CAN_RFID_Arr[i].RFID_IP_ADD,IR_CAN_RFID_Arr[i].RFID_Station_NO))
            {

           //1、查询联锁数据表[3F_IR_RUN]中的 ‘IR_Finished’完成标识位，
           //如果全部作业完成，则需要更新RFID读取的机型记录表

                 if (Query_IR_Finished_IO(IR_CAN_RFID_Arr[i].RFID_Station_NO,IR_CAN_RFID_Arr[i].RFID_IP_ADD)=="all_finished")
                  {//--------------------------------------1begin

                      //取得[3F_IR_RUN]数据表中与当前RFID对应的[IR_MIN_ID_OF_LIST]，
                        Record_ID=QueryIR_Olddest_ID_FromLISTTable_IO(IR_CAN_RFID_Arr[i].RFID_Station_NO,IR_CAN_RFID_Arr[i].RFID_IP_ADD);


                        if (Record_ID.length()!=0)//--------------------
                        {
                        //全部作业完成，更新数据表[3F_IR_RFID_READDATALIST] 的字段ALL_FINISHED
                        if (Record_ID.toInt()>0)
                           {
                              //更新数据表[3F_IR_RFID_READDATALIST]

                              UPdate_IR_RFID_READDATALIST(Record_ID);

                              Delete_IR_RFID_READDATALIST_COM34(Record_ID);


                               //删除[3F_IR_RUN]作业完成记录
                              DeleteFin_IR_RUN_Record_ByRFIDNO_IP_COM34(IR_CAN_RFID_Arr[i].RFID_Station_NO,IR_CAN_RFID_Arr[i].RFID_IP_ADD);
                              /*
                              if (i==3) {
                                this->AddState("RFID03作业完成,更新READDATALIST完成-"+Record_ID+"");
                              }
                              else if (i==4) {
                                this->AddState2("RFID04作业完成,更新READDATALIST完成-"+Record_ID+"");
                              }
                              */
                        //使用单台PC读取数据的各工位完成状态
                       // 01DB02IO=011
                           if (IR_Infor.PC_DB_IO=="01")
                            {
                              if ((PC_ANDON_WriteFIN=="00") &&(IR_CAN_RFID_Arr[i].RFID_Station_NO=="03"))
                               {
                                 if (IR_Infor.Sub_Device_First=="02" )
                                   {
                                        if  (PC_TC_BarCodeCheck=="01")
                                          {
                                                PC_TC_BarCodeCheck="00";
                                                //20150704追加使用PC读写表[3F_IR_ANDON]
                                                UPdate_IR_ANDON_Fin(IR_CAN_RFID_Arr[i].RFID_Station_NO,IR_CAN_RFID_Arr[i].RFID_IP_ADD,"01");
                                               //写安东记录
                                                ADD_IR_ANDON_Fin("03",IR_CAN_RFID_Arr[i].RFID_IP_ADD,ui->eqmid3_lineEdit->text());
                                                PC_ANDON_WriteFIN="01";
                                          }
                                    }
                                   else
                                     {
                                         //20150704追加使用PC读写表[3F_IR_ANDON]
                                          UPdate_IR_ANDON_Fin(IR_CAN_RFID_Arr[i].RFID_Station_NO,IR_CAN_RFID_Arr[i].RFID_IP_ADD,"01");

                                        //写安东记录
                                          ADD_IR_ANDON_Fin("03",IR_CAN_RFID_Arr[i].RFID_IP_ADD, ui->eqmid3_lineEdit->text());
                                          PC_ANDON_WriteFIN="01";
                                     }



                              }
                             else if ((PC_ANDON_WriteFIN_Second=="00") && (IR_CAN_RFID_Arr[i].RFID_Station_NO=="04"))
                               {

                                 //20150704追加使用PC读写表[3F_IR_ANDON]
                                 UPdate_IR_ANDON_Fin(IR_CAN_RFID_Arr[i].RFID_Station_NO,IR_CAN_RFID_Arr[i].RFID_IP_ADD,"01");
                                 //写安东记录
                                 ADD_IR_ANDON_Fin("04",IR_CAN_RFID_Arr[i].RFID_IP_ADD,ui->eqmid4_lineEdit->text());
                                 PC_ANDON_WriteFIN_Second="01";
                              }
                            }//if (IR_Infor.PC_DB_IO=="01")判断结束


                           //------------------------------------------------------------------
                              //防止一直读取RFID的内容
                              //作业完成后，更新作业完成标志
                            IR_CAN_RFID_Arr[i].IR_RFID_ReadData_Only="00";
                             //对所绑定的RFID的控制区域的阻挡器进行强制输出（判断是否需要作业联锁有效还是无效）
                              //先要判断[3F_IR_RUN]中是否有记录
                            if (IR_CAN_RFID_Arr[i].IR_Belong_STOPGO.STOPGO_State=="1")
                             {
                                   if (i==3) {
                                     this->AddState("RFID03作业完成,阻挡放行");
                                   }
                                   else if (i==4) {
                                     this->AddState2("RFID04作业完成,阻挡放行");
                                   }
                                   EnableSTOPGO_GO_PC(i);
                              }
                           }
                          }//-----------------------if (Record_ID.length()!=0)
                          else
                          {
                               if (i==3)
                                 {
                                        AddState("03号号RFID:出错了");
                                 }
                               else if(i==4)
                                 {
                                        AddState2("04号RFID:出错了");
                                 }
                          }

                        //置位当前RFID绑定的当前机型联锁全部完成
                        IR_CAN_RFID_Arr[i].IR_Finished=true;
                        IR_CAN_RFID_Arr[i].RFID_IO_Init="1";

                 }//--------------------------------------1end全部作业完成
           //如果作业未全部完成，则继续读取记录，并发送给各站点
                else
                 {
                    if (IR_CAN_RFID_Arr[i].RFID_IO_Init=="1")//----在启动软件时初始化为"1"
                      {
                        IR_CAN_RFID_Arr[i].IR_Finished=false; //未全部完成
                         //查询记录 [3F_IR_RUN]还有哪几个绑定的RFID还有作业联锁未完成
                        if (i<10 )     //此处的i是RFID的编号
                        {
                            itemp="0"+itemp.setNum(i,10);
                        }
                        else
                        {
                            itemp=itemp.setNum(i,10);
                        }
                        //如果还有作业未完成的记录

                        if (QueryIR_RUN_NO_FIN(IR_CAN_RFID_Arr[i].RFID_IP_ADD,itemp))
                           {

                               //重启系统后，调用
                               IR_Cycle_Continue_IO(IR_CAN_RFID_Arr[i].RFID_IP_ADD,itemp);
                           }
                        IR_CAN_RFID_Arr[i].RFID_IO_Init="2";
                      }
                 }

              //如果全部作业完成，重新读取下一机型对应的联锁内容
               if (IR_CAN_RFID_Arr[i].IR_Finished)
                   {
                     if (IR_CAN_RFID_Arr[i].RFID_IO_Init=="1")
                      {
                        //因为RFID与零件指示模块是有不同的站点，需要将RFID的站点与其相关的零件指示站点绑定
                        IR_CAN_RFID_Arr[i].MIN_ID_YES=false;//设定为“不是最老的记录
                        //-------------------------------------------------
                        IR_Cycle_Start_IO(IR_CAN_RFID_Arr[i].RFID_IP_ADD,IR_CAN_RFID_Arr[i].RFID_Station_NO);
                        IR_CAN_RFID_Arr[i].RFID_IO_Init="3";
                     }
                   }
              //0结束---------------
            }
          else
            {
     //---------------------------------------------------------20160820lin取消设定无联锁-------------
               //AddState("作业联锁未设定--");
               //AddState("--作业联锁未设定");
                //使用单台PC读取数据的各工位完成状态
                       // 01DB02IO=01
             if (IR_Infor.PC_DB_IO == "01" )
               {
                   if( (PC_ANDON_WriteFIN == "00") && (IR_CAN_RFID_Arr[i].RFID_Station_NO == "03") )
                        {   //20150704追加使用PC读写表[3F_IR_ANDON]
                            UPdate_IR_ANDON_Fin(IR_CAN_RFID_Arr[i].RFID_Station_NO, IR_CAN_RFID_Arr[i].RFID_IP_ADD, "01");
                            //写安东记录
                            ADD_IR_ANDON_Fin("03", IR_CAN_RFID_Arr[i].RFID_IP_ADD, ui->eqmid3_lineEdit->text());
                            PC_ANDON_WriteFIN = "01";
                           // frm_IR_Run.Panel15.color = clLime;
                          //frm_IR_Run.Edit2.Text="无联锁,作业放行";
                        }
                    else if ((PC_ANDON_WriteFIN_Second == "00") && (IR_CAN_RFID_Arr[i].RFID_Station_NO == "04"))
                        {
                                 //20150704追加使用PC读写表[3F_IR_ANDON]
                            UPdate_IR_ANDON_Fin(IR_CAN_RFID_Arr[i].RFID_Station_NO, IR_CAN_RFID_Arr[i].RFID_IP_ADD, "01");
                                 //写安东记录
                            ADD_IR_ANDON_Fin("04", IR_CAN_RFID_Arr[i].RFID_IP_ADD, ui->eqmid4_lineEdit->text());
                            //frm_IR_Run.Edit3.Text="无联锁,作业放行";
                            PC_ANDON_WriteFIN_Second = "01";
                           //frm_IR_Run.Panel3.color = clLime;

                         }
                 }
              else
                 {
                    EnableSTOPGO_GO_PC(i);
                 }
               //-----------------------------------20160820lin取消设定无联锁-------------

            }

       } //判断IR_CAN_RFID_Arr[i].IR_IO_Type="02" 结束

    }//判断IR_CAN_RFID_Arr[i].RFID_Station_NO<>""结束

   } //FOR循环结束
}


//线程使用
//根据本系统配置的IP地址、绑定RFID的站号，查询对应的作业3F_IR_INPUT是否有相关记录
bool  MainWindow::QueryIR_INPUT_LIST_BE(const QString StrIP,const QString strstationNO)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;

    strSQL="select count(IR_RFID_No) from 3F_IR_INPUT Where IR_RFID_No='"+strstationNO+"' and IR_SET_IPaddr='"+StrIP+"'";

    ADOQ.setQuery(strSQL);
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    //strCount = ADOQ.columnCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    if (strCount>0)
    {
            return true;
    }
        else
    {
            return false;
    }

}

//线程使用
//检查是否全部作业完成
QString MainWindow::Query_IR_Finished_IO(const QString Str_RFID_NO,const QString StrIP)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;
    QString reTmpStr;//无记录
    QString strFinished ="是";
    //strRet=0;

     strSQL="select Count(ID)  from 3F_IR_RUN Where  (IR_RFID_No='"+Str_RFID_NO+"') and (IR_IP_ADD='"+StrIP+"') and ((IR_Finished is null) or (IR_Finished<>'"+strFinished+"') )";

    ADOQ.setQuery(strSQL);
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    //strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    if (strCount>0)
    {
            reTmpStr="not_finished";
    }
    else
    {
            reTmpStr="all_finished";
    }

            return reTmpStr;
}




QString MainWindow::QueryIR_Olddest_ID_FromLISTTable_IO(const QString Str_RFID_No,const QString StrIP)
{

    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;
    QString reTmpStr="0";

    strSQL="select count(IR_MIN_ID_OF_LIST)  from 3F_IR_RUN Where IR_RFID_No='"+Str_RFID_No+"' and IR_IP_ADD='"+StrIP+"'";

    ADOQ.setQuery(strSQL);
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    //strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    if (strCount>0)
    {
        strSQL="select Min(IR_MIN_ID_OF_LIST)  from 3F_IR_RUN Where IR_RFID_No='"+Str_RFID_No+"' and IR_IP_ADD='"+StrIP+"'";
        ADOQ.setQuery(strSQL);
        reTmpStr=ADOQ.data(ADOQ.index(0, 0)).toString();

    }

       return reTmpStr;
}


//更新数据表[3F_IR_RFID_READDATALIST] 的字段ALL_FINISHED
void MainWindow::UPdate_IR_RFID_READDATALIST(const QString StrMIN_ID)
{
   QSqlQuery * query = new QSqlQuery();

   query->exec("Update 3F_IR_RFID_READDATALIST SET  ALL_FINISHED=1 where ID='"+StrMIN_ID+"'");

}

void MainWindow::Delete_IR_RFID_READDATALIST_COM34(const QString StrID)
{

    QSqlQuery * query = new QSqlQuery();

    query->exec("delete  from  3F_IR_RFID_READDATALIST where ID='"+StrID+"'");

}


//更新数据表[3F_IR_ANDON] 的字段 FINISH_STATE
void MainWindow::UPdate_IR_ANDON_Fin(const QString StrRFID_NO,const QString StrIP,const QString StrState)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;
    QString StrEGTypeFlow="0";
    QString Strtemp="00";

    strSQL="select count(ID) from 3F_IR_ANDON Where IP_IR_PC='"+StrIP+"' AND RFID_NO='"+StrRFID_NO+"'";

    ADOQ.setQuery(strSQL);
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    //strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    if (strCount==0)
    {
    //新增记录
        QSqlQuery * query = new QSqlQuery();

        query->exec("insert into 3F_IR_ANDON(IP_ADD,RFID_NO,RESET_STATE,FINISH_STATE,ALL_STATE,EGTypeFlow) values('"+StrIP+"','"+StrRFID_NO+"','"+Strtemp+"','"+StrState+"','"+StrState+"','"+Strtemp+"')");


    }
  else
    {
     //更新记录
      if (StrRFID_NO=="03")
         {
               AddState("03号RFID更新ANDON-作业完成");
               StrEGTypeFlow=ui->eqmid3_lineEdit->text();
         }
      else if (StrRFID_NO=="04")
         {
               AddState2("04号RFID更新ANDON-作业完成");
               StrEGTypeFlow=ui->eqmid4_lineEdit->text();
         }
      QSqlQuery * query = new QSqlQuery();

      query->exec("Update 3F_IR_ANDON SET  ALL_STATE='"+StrState+"',EGTypeFlow='"+StrEGTypeFlow+"' where IP_IR_PC='"+StrIP+"' AND RFID_NO='"+StrRFID_NO+"'");


  }
}


//更新数据表[3F_IR_ANDON] 的字段 FINISH_STATE
void MainWindow::ADD_IR_ANDON_Fin(const QString StrRFID_NO,const QString StrIP,const QString StrState)
{
 QString strinputdatetime ;//FormatDateTime('yyyy-MM-dd HH:mm:ss',now);
 QString Strdatetime =strinputdatetime;
 QString strEGTypeFlow=StrState;
 QString Strtemp,Strtemp2;
 Strtemp="00";
 Strtemp2="Fin_OK";
  //保存记录
  //新增记录
      QSqlQuery * query = new QSqlQuery();

      query->exec("insert into IR_ANDON_RECORD_DETAIL(IP_IR_PC,RFID_NO,EGTypeFlow,Datetime,ANDON_NO,Type_ANDON) values('"+StrIP+"','"+StrRFID_NO+"','"+strEGTypeFlow+"','"+Strdatetime+"','"+Strtemp+"','"+Strtemp2+"')");

}

//向绑定的RFID强制放行阻挡气缸
void MainWindow::EnableSTOPGO_GO_PC(qint32 Rfid_NO)
{
  qint32 i;
  QString str_i;
  QString str_CMD_Value,str_IR_Station_NO;

          //1、根据RFID编号找到对应阻挡器、复位检测开关连接用的IO模块的站号
          str_IR_Station_NO=IR_CAN_RFID_Arr[Rfid_NO].IR_Belong_STOPGO.STOPGO_Station_NO ;

          str_CMD_Value="0000000000000000";
          IR_CAN_RFID_Arr[Rfid_NO].IR_Belong_STOPGO.STOPGO_Eanbled=true;
          //2、使能阻挡气缸的引脚为1，强制输出

          i=IR_CAN_RFID_Arr[Rfid_NO].IR_Belong_STOPGO.STOPGO_Pin_Output_PinNo.toInt();


          if ((IR_CAN_RFID_Arr[Rfid_NO].RFID_EGTypeGroupvalue=="1") ||(IR_CAN_RFID_Arr[Rfid_NO].RFID_EGTypeGroupvalue=="2"))
             {
                 str_CMD_Value[13]='1';
                 str_CMD_Value[17-i]='1';
             }
          else
             {
                 str_CMD_Value[17-i]='1';
             }


          //采用IO输入复位读卡循环，且使用的是发3F的rfid（01DB02IO=02），（RFIDtype=2OMRON，1是sige）
          //if( (IR_Infor.PC_DB_IO=="02") && (IR_RFID.IR_RFID_Type=="1"))
          if( (IR_Infor.PC_DB_IO=="02") )
          {
            bool bok=false;
            str_CMD_Value=str_CMD_Value.setNum(str_CMD_Value.toInt(&bok,2),16) ;
            //取消数据库查询阻挡---20160809-lin
            if( Rfid_NO==3)
             {
                str_IR_Station_NO="252";
                str_CMD_Value="FF";
             }
            else if (Rfid_NO==4)
             {
                str_IR_Station_NO="254";
                str_CMD_Value="FF";
             }

            if (Rfid_NO==3)
            {
               AddState("复位站03--'"+str_IR_Station_NO+"'");
            }
            else if (Rfid_NO==4)
            {
              AddState2("复位站04--"+str_IR_Station_NO+"'");
            }
           SendCMD_TO_Station_By_CANCOM(str_IR_Station_NO,str_CMD_Value);

          }
          IR_CAN_RFID_Arr[Rfid_NO].IR_Belong_STOPGO.STOPGO_State="3";
}


//节拍到达，检测是否作业完成，如果没有则驱动蜂鸣器报警
void MainWindow::Alram_for_CycleArraved()
{
  qint32 i,Max_i;
    //RFID_CAN_MaxStationNO,查询3F_CAN_RFID_STATIONNO_SET数据表中最大的RFID_Station_NO
    if (RFID_CAN_MaxStationNO>End_stationNO)
       {
          Max_i=End_stationNO;
       }
    else
       {
          Max_i=RFID_CAN_MaxStationNO;
       }
    for(i=Start_stationNO;i<(Max_i+1);i++)
    {
        //如果节拍到，且作业未完成，则点亮蜂鸣器
        if ((IR_CAN_RFID_Arr[i].IR_CycleTimeOver=="01") && (!( IR_CAN_RFID_Arr[i].IR_Finished )))
           {
              //1、输出指令，点亮蜂鸣器
               IR_CAN_RFID_Arr[i].IR_CycleTimeOver_alramsta="01";//报警响标识
              //2、提示作业延迟信息
               if (i==Start_stationNO)
                  {
                   AddState("本工序作业延迟！！");

                   if ((Max_i==End_stationNO)||(Max_i>End_stationNO))
                     {
                         AddState("有后工序作业设定！！");
                         if (IR_CAN_RFID_Arr[i+1].IR_CycleTimeOver_alramsta=="00")
                            {
                                 AddState("后工序作业无蜂鸣器输出！！");
                                 SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"01");
                                 SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"01");
                            }
                         else
                            {
                                 AddState("后工序作业有蜂鸣器输出！！");
                                 SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"03");
                                 SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"03");
                            }
                     }
                    else
                     {
                         AddState("无后工序作业设定！！");
                         SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"00");
                         SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"00");
                     }
                  }
               else if (i==End_stationNO)
                  {
                   AddState2("本工序作业延迟！！");

                   if (IR_CAN_RFID_Arr[i-1].IR_CycleTimeOver_alramsta=="00")
                       {
                           AddState2("前工序作业无蜂鸣器输出！！");
                           SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"02");
                           SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"02");
                       }
                   else
                       {
                           AddState2("前工序作业有蜂鸣器输出！！");
                           SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"03");
                           SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"03");
                       }
                  }
           }
        //节拍到，而且作业完成
        else  if ((IR_CAN_RFID_Arr[i].IR_CycleTimeOver_alramsta=="01") && (IR_CAN_RFID_Arr[i].IR_Finished ))
           {
               IR_CAN_RFID_Arr[i].IR_CycleTimeOver="00";
               IR_CAN_RFID_Arr[i].IR_CycleTimeOver_alramsta="00";
               if (i==Start_stationNO)
                  {
                   AddState("本工序作业完成！！");
                   if ((Max_i==End_stationNO)||(Max_i>End_stationNO))
                     {
                         if (IR_CAN_RFID_Arr[i+1].IR_CycleTimeOver_alramsta=="00")
                            {
                                 SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"00");
                                 SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"00");
                            }
                         else
                            {
                                 SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"02");
                                 SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"02");
                            }
                     }
                    else
                     {
                         SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"00");
                         SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"00");
                     }

                  }
               else if (i==End_stationNO)
                  {
                   AddState2("本工序作业完成！！");
                   if (IR_CAN_RFID_Arr[i-1].IR_CycleTimeOver_alramsta=="00")
                       {
                           SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"00");
                           SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"00");
                       }
                   else
                       {
                           SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"01");
                           SendCMD_TO_Station_By_CANCOM(IR_Infor.CycleTime_Arravl_from_IO_StationNO,"01");
                       }
                  }
           }
    }//for循环结束
}


bool MainWindow::QueryIR_RUN_NO_FIN(const QString StrIP,const QString strstationNO)
{
QSqlQueryModel ADOQ;
QString strSQL;
qint32 strCount;

strSQL="select count(ID) from 3F_IR_RUN where IR_IP_ADD='"+StrIP+"' and IR_RFID_No='"+strstationNO+"'  and IR_Finished='"+strtemp_NG+"'";

ADOQ.setQuery(strSQL);
strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
//strCount = (ADOQ.record(0).value("id")).toInt();
//取得一共有多少条件记录
//strCount = ADOQ.rowCount();
//循环将各条记录的各字段内容赋值给各数组变量
if (strCount>0)
{
        return true;
}
    else
{
        return false;
}


}



//-----------------------------------------------------------

//如果启动系统时检测到[3F_IR_RUN] 表中还有未完成的联锁作业，则
//联锁循环继续
void  MainWindow::IR_Cycle_Continue_IO(const QString strIP,const QString strRFID_NO)//strStation_NO为RFID绑定的站号
{
 qint32 i;
 QString strtemp;

 //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$   can   strRFID_NO:string;

     i=strRFID_NO.toInt(); //RFID的站号编号
         //--------------------------------------------开始
                   //根据RFID站号和IP, 从[3F_IR_RUN]查询对应的取得机型
       IR_CAN_RFID_Arr[i].EGType_Flow=QueryEGType_IR_RUN_NO_FIN(strRFID_NO,strIP);
       IR_CAN_RFID_Arr[i].RFID_EGTypeGroupvalue=QueryTypeGroupNO_Cycle_Continue(IR_CAN_RFID_Arr[i].EGType_Flow);

      if (!Check_Interlock_Set(IR_CAN_RFID_Arr[i].EGType_Flow,strRFID_NO,strIP))
       {
                  //阻挡气缸
                   IR_CAN_RFID_Arr[i].IR_Belong_STOPGO.STOPGO_State="1";
       }
       else
       {


                   //查询绑定的RFID受控的IO站数 [3F_IR_RUN]的字段[IR_Station_NO]中最大的值
                   strtemp= Query_IO_CAN_MaxStationNO_Cycle(IR_CAN_RFID_Arr[i].EGType_Flow,strRFID_NO,strIP);

                   //阻挡气缸
                   IR_CAN_RFID_Arr[i].IR_Belong_STOPGO.STOPGO_State="1";


                  //追加一个DataModule_3F.Query_IO_CAN_MaxStationNO变量为空时的处理，防止转换为整数时出错
                   if (strtemp.length()!=0)
                     {
                   if (strtemp.toInt()==0)
                        {
                            AddState("查询联锁IO站数时出错");      //将相关信息写到画面
                            return;
                        }
                   else
                        {

                            IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO=strtemp.toInt();

                            //setlength(IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr,IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO+1);
                            IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr=new IR_IOTYPE[IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO+1];
                            //一、初始化对应RFID管辖的IO站点信息,包括站点和模块类型
                            InitIOStationInfor(strIP,i,strRFID_NO,IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO);

                            //发送给IO站点   1、零件指示
                            //给数组IR_CAN_RFID_Arr[i]变量附值,包括各绑定RFID受控的IO 站点的设定值
                            //设定值包括零件指示、拧紧枪联锁


                            //二、初始化零件指示联锁
                            Query_RFID_CAN_SetInfor_IO_ByUsefor(IR_CAN_RFID_Arr[i].EGType_Flow,strRFID_NO,strIP,"02");

                            //三、发送零件指示程序给各站点
                            SendPartshowPrgNO_to_IOStation(i,IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO);

                            //发送给IO站点   2、拧紧枪程序选择、联锁指示
                            //给数组IR_CAN_RFID_Arr[i]变量初始化
                           //四、初始化拧紧联锁,发送拧紧程序给各站点
                            SendGunPrgNO_to_IOStation_Continue(strIP,i,strRFID_NO,IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO);

                         }
                    }

              //--------------------------------------------结束
      }
}
//-----------------------------------------------------------

 //根据RFID站号，从[3F_IR_RUN] 查询对应的机型短码
QString  MainWindow::QueryEGType_IR_RUN_NO_FIN(const QString Str_RFID_No,const QString StrIP)
{

    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;
    QString reTmpStr="";//无记录
    //strRet=0;
    strSQL="select count(ID) from 3F_IR_RUN Where IR_RFID_No='"+Str_RFID_No+"' and IR_IP_ADD='"+StrIP+"'";

    ADOQ.setQuery(strSQL);
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //取得一共有多少条件记录
    //循环将各条记录的各字段内容赋值给各数组变量
    if (strCount>0)
    {

       strSQL="select distinct(IR_EGtype_NO) from 3F_IR_RUN Where IR_RFID_No='"+Str_RFID_No+"' and IR_IP_ADD='"+StrIP+"'";

       ADOQ.setQuery(strSQL);
       reTmpStr=ADOQ.data(ADOQ.index(0, 0)).toString();
    }

    return reTmpStr;
}

//防止站号重复，查询[3F_IR_INPUT]   QueryTypeGroupNO
QString  MainWindow::QueryTypeGroupNO_Cycle_Continue(const QString StrEGtype)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;
    QString reTmpStr="00";//无记录
    //strRet=0;

     strSQL="select count(机型分组) from 3F_EGtype where  短型号='"+StrEGtype+"' ";

    ADOQ.setQuery(strSQL);
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    //strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    if (strCount>0)
    {
        strSQL="select distinct(机型分组) from 3F_EGtype where  短型号='"+StrEGtype+"'";

        ADOQ.setQuery(strSQL);
        reTmpStr = ADOQ.data(ADOQ.index(0, 0)).toString();
    }

    return reTmpStr;

}

//检索各CAN站点RFID读取的数据在数据表中是否已经存在，如果存在则加个后缀@
bool MainWindow::Check_Interlock_Set(const QString strFlow,const QString strStation,const QString strIP)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;

    strSQL="select count(ID) from 3F_IR_INPUT where  IR_EGtype_NO='"+strFlow+"' and IR_RFID_No='"+strStation+"'and IR_SET_IPaddr='"+strIP+"'";

    ADOQ.setQuery(strSQL);
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();


    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    //strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    if (strCount>0)
    {

            return true;

    }
    else
    {
            return false;
    }

}

//查询绑定的RFID受控的IO站数 [3F_IR_RUN]
QString  MainWindow::Query_IO_CAN_MaxStationNO_Cycle(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP)
{
QSqlQueryModel ADOQ;
QString strSQL;
qint32 strCount;
QString reTmpStr="0";

strSQL="select count(IR_Station_NO) from 3F_IR_RUN where IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"'";

ADOQ.setQuery(strSQL);
strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
//strCount = (ADOQ.record(0).value("id")).toInt();
//取得一共有多少条件记录
//strCount = ADOQ.rowCount();
//循环将各条记录的各字段内容赋值给各数组变量
if (strCount>0)
{
    strSQL="select max(IR_Station_NO) from 3F_IR_RUN where IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"'";

    ADOQ.setQuery(strSQL);
    reTmpStr= ADOQ.data(ADOQ.index(0, 0)).toString();
}

return reTmpStr;

}




//初始化对应RFID管辖的IO站点信息
void  MainWindow::InitIOStationInfor(const QString strIP,qint32 intRFID_NO,const QString strRFID_NO,qint32 intMaxstationNO)//strStation_NO为RFID绑定的站号
{
    qint32 j;
    QString str_temp,result_temp,str_temp_qd;
    j=0 ;
    for(j=1;j<intMaxstationNO+1;j++)
    {
        //1初始化需要连接的IO站点为空
           IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Station_NO="";
        //2在数据表[3f_ir_run]中是否有对应IO站点的设定记录
           str_temp=str_temp.setNum(j,10);

           if (j<10)
               {
                   result_temp=QueryIR_StationNO_From_IR_RUN(IR_CAN_RFID_Arr[intRFID_NO].EGType_Flow.mid(0,3),strRFID_NO,strIP,"0"+str_temp);


               }
           else
               {
                   result_temp=QueryIR_StationNO_From_IR_RUN(IR_CAN_RFID_Arr[intRFID_NO].EGType_Flow.mid(0,3),strRFID_NO,strIP,str_temp);

               }
           IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Station_NO=result_temp.mid(0,2);
           IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Userfor=result_temp.mid(3,2);
           qDebug()<<"InitIOStationInfor:IR_IO_Station_NO"<<IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Station_NO << "--IR_IO_Userfor:"<<IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Userfor<<"--rfid:" <<str_temp.setNum(intRFID_NO,10);


           /*
           if (strRFID_NO==strStart_stationNO)    //前工序
           {
                AddState("查询联锁IO站数成功99-"+result_temp+"");      //将相关信息写到画面
           }
           else if (strRFID_NO==strEnd_stationNO)  //后工序
           {
                AddState2("查询联锁IO站数成功99-"+IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Userfor+"");      //将相关信息写到画面
           }
          */

           //3如果在数据表[3f_ir_run]中是有对应IO站点的设定记录,则需要判断对应的模块类型
          //在LINUX平台中此语句移动到前面2步骤中一起返回,减少读数据表的次数
          // IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Userfor=QueryIR_IO_Userfor(IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Station_NO,strRFID_NO,strIP);

    }
}



//读取数据库3F_IR_INPUT中配置绑定的RFID（CAN总线）受控的站点及对应的设备信息
QString MainWindow::QueryIR_StationNO_From_IR_RUN(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP,const QString Str_StationNO)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;
    QString reTmpStr="00";

    //strSQL="select * from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"' and IR_Station_NO='"+Str_StationNO+"'";

    strSQL="select count(ID) from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"' and IR_Station_NO='"+Str_StationNO+"'";

    ADOQ.setQuery(strSQL);
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();


    //取得一共有多少条件记录
    //strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    if (strCount>0)
    {
        strSQL="select IR_Station_NO, IR_Model_Usefor from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"' and IR_Station_NO='"+Str_StationNO+"'";

        ADOQ.setQuery(strSQL);
        reTmpStr = (ADOQ.record(0).value("IR_Station_NO")).toString();
        reTmpStr =reTmpStr+":"+ (ADOQ.record(0).value("IR_Model_Usefor")).toString();//两个参数一起返回


    }

    return reTmpStr;

}

//根据RFID号、站点号、IP值查询对应的在数据库表3F_IR_RUN中的模块类型
//其实就是要实现先进先出
QString MainWindow::QueryIR_IO_Userfor(const QString Str_Station_No,const QString Str_RFID_No,const QString StrIP)
{
  QSqlQueryModel ADOQ;
  QString strSQL;
  qint32 strCount;
  QString reTmpStr="0";

  strSQL="select count(ID) from 3F_IR_RUN Where IR_RFID_No='"+Str_RFID_No+"' and IR_IP_ADD='"+StrIP+"' and  IR_Station_NO='"+Str_Station_No+"'";

  ADOQ.setQuery(strSQL);
  strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
  //循环将各条记录的各字段内容赋值给各数组变量
  if (strCount>0)
  {
      strSQL="select distinct IR_Model_Usefor from 3F_IR_RUN Where IR_RFID_No='"+Str_RFID_No+"' and IR_IP_ADD='"+StrIP+"' and  IR_Station_NO='"+Str_Station_No+"'";

      ADOQ.setQuery(strSQL);
      reTmpStr = (ADOQ.record(0).value("IR_Station_NO")).toInt();
  }
  return reTmpStr;
}






void  MainWindow::Query_RFID_CAN_SetInfor_IO_ByUsefor(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP,const QString StrModel_Usefor)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount,j,i,k;
    QString pin_type,strtemp_stationNO,ttemp;
    j=0;
    i= Str_RFID_No.toInt();

    if (i==3){
    this->AddState("---03读取现有-"+ttemp.setNum(strCount,10)+"-作业清单"+Str_EGShortNo+"--"+Str_RFID_No+"--"+StrIP+"--"+StrModel_Usefor+"");
    }else if (i==4){
    this->AddState2("---04读取现有-"+ttemp.setNum(strCount,10)+"-作业清单"+Str_EGShortNo+"--"+Str_RFID_No+"--"+StrIP+"--"+StrModel_Usefor+"");
    }

    strSQL="select count(ID) from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"' AND IR_Model_Usefor='"+StrModel_Usefor+"'";

    ADOQ.setQuery(strSQL);

    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    if (strCount<1)
    {
        return;
    }

    if (i==3){
    this->AddState("---03读取现有-"+ttemp.setNum(strCount,10)+"-作业清单"+Str_EGShortNo+"--"+Str_RFID_No+"--"+StrIP+"--"+StrModel_Usefor+"");
    }else if (i==4){
    this->AddState2("---04读取现有-"+ttemp.setNum(strCount,10)+"-作业清单"+Str_EGShortNo+"--"+Str_RFID_No+"--"+StrIP+"--"+StrModel_Usefor+"");
    }

    strSQL="select * from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"' AND IR_Model_Usefor='"+StrModel_Usefor+"'";

    ADOQ.setQuery(strSQL);
    //取得一共有多少条件记录
    //strCount = ADOQ.rowCount();


    //循环将各条记录的各字段内容赋值给各数组变量
    for(k=0;k<strCount;k++)
    {

           j=ADOQ.record(k).value("IR_Station_NO").toInt();//IO模块CAN站的站号，用于绑定该模块对应数组信息
           strtemp_stationNO=ADOQ.record(k).value("IR_Station_NO").toString();


           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Input_length=ADOQ.record(k).value("IR_PIN_InPut_Length").toString();
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_length=ADOQ.record(k).value("IR_PIN_OutPut_Length").toString();
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_MAP_StationNO=Str_RFID_No;
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Module_Type=ADOQ.record(k).value("IR_Model_No").toString();
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Station_NO=ADOQ.record(k).value("IR_Station_NO").toString();
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Actived_Station_NO=ADOQ.record(k).value("IR_Valid").toString();
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Userfor=ADOQ.record(k).value("IR_Model_Usefor").toString();

           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_MAP_PLC_D=ADOQ.record(k).value("IR_PLC_D").toString(); //PLC对应的D地址
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].Prg_NO_Hex=ADOQ.record(k).value("IR_Prg_NO").toString();


           //模块类型的判断,清除原来的数据 （站点数据，程序选择数据）
           Clear_Array_PIN_PRG_CAN(IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Module_Type,i,j);

           //赋值
            //02零件指示联锁,直接找设定的输出脚
           if (IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Userfor=="02")
                {
                 pin_type="OUT";
                 Query_RFID_CAN_SetValue_IO(Str_EGShortNo,Str_RFID_No,StrIP,strtemp_stationNO,pin_type);

                 pin_type="IN";
                 Query_RFID_CAN_SetValue_IO(Str_EGShortNo,Str_RFID_No,StrIP,strtemp_stationNO,pin_type);
           }
           //04拧紧作业联锁,先查找设定的拧紧程序，然后根据程序匹配到对应的输出脚
           else  if( IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Userfor=="04")
                {
                 /*
                if IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].Prg_NO_Hex<>""
                  {
                    //1、首先查询设定的端子功能代码(包括输入、输出)
                     pin_type="01"; //01输入，02输出
                     Query_RFID_CAN_SetPIN_IO(Str_EGShortNo,Str_RFID_No,StrIP,strtemp_stationNO,pin_type);

                     pin_type="02"; //01输入，02输出
                     Query_RFID_CAN_SetPIN_IO(Str_EGShortNo,Str_RFID_No,StrIP,strtemp_stationNO,pin_type);



                     //2、其次查询设定的模块程序
                     Query_RFID_CAN_SetPrg_IO(Str_EGShortNo,Str_RFID_No,StrIP,strtemp_stationNO,pin_type);


                     //3、根据设定的端子功能代码，将查询得到的程序输出到相应的端子上（即将相应的功能端子置位1）

                     pin_type="02"; //01输入，02输出

                     Make_RFID_CAN_Pin_Prg_IO(Str_RFID_No,StrIP,strtemp_stationNO);


                     //4、查找作业完成输入信号脚、复位输出信号脚

                     Make_RFID_CAN_Pin_Finished_Reset_IO(Str_EGShortNo,Str_RFID_No,StrIP,strtemp_stationNO,pin_type);



                  }  //判断 IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].Prg_NO_Hex<>""结束

                  */
                }


      }
}




//读取数据库3F_IR_INPUT中配置绑定的RFID（CAN总线）受控的站点及对应的设备信息
void  MainWindow::Query_RFID_CAN_SetInfor_IO_ByPrg(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP,const QString StrMinPrgNo,const QString StrstationNo)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount,j,i,k;
    QString pin_type,strtemp_stationNO;

    j=0;

    i= Str_RFID_No.toInt();

    strSQL="select count(ID) from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"' AND IR_Prg_NO='"+StrMinPrgNo+"' and IR_Station_NO='"+StrstationNo+"'";

    ADOQ.setQuery(strSQL);
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();

    if(strCount<1){
        if (k==3){
            qDebug()<<"Query_RFID_CAN_SetInfor_IO_ByPrg:03no_prg";

        }
        else if(k==4){

            qDebug()<<"Query_RFID_CAN_SetInfor_IO_ByPrg:04no_prg";

        }
     return ;
    }

    strSQL="select * from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"' AND IR_Prg_NO='"+StrMinPrgNo+"' and IR_Station_NO='"+StrstationNo+"'";

    ADOQ.setQuery(strSQL);
    //strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    for(k=0;k<strCount;k++)
    {


           j=ADOQ.record(k).value("IR_Station_NO").toInt();//IO模块CAN站的站号，用于绑定该模块对应数组信息
           strtemp_stationNO=ADOQ.record(k).value("IR_Station_NO").toString();


           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Input_length=ADOQ.record(k).value("IR_PIN_InPut_Length").toString();
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_length=ADOQ.record(k).value("IR_PIN_OutPut_Length").toString();
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_MAP_StationNO=Str_RFID_No;
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Module_Type=ADOQ.record(k).value("IR_Model_No").toString();
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Station_NO=ADOQ.record(k).value("IR_Station_NO").toString();
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Actived_Station_NO=ADOQ.record(j).value("IR_Valid").toString();
           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Userfor=ADOQ.record(k).value("IR_Model_Usefor").toString();

           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_MAP_PLC_D=ADOQ.record(k).value("IR_PLC_D").toString(); //PLC对应的D地址


           IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].Prg_NO_Hex=ADOQ.record(k).value("IR_Prg_NO").toString();


           //模块类型的判断,清除原来的数据 （站点数据，程序选择数据）
           Clear_Array_PIN_PRG_CAN(IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Module_Type,i,j);
           //赋值
            //02零件指示联锁,直接找设定的输出脚
           if (IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Userfor=="02")
                {
                 pin_type="OUT";
                 Query_RFID_CAN_SetValue_IO(Str_EGShortNo,Str_RFID_No,StrIP,strtemp_stationNO,pin_type);

                 pin_type="IN";
                 Query_RFID_CAN_SetValue_IO(Str_EGShortNo,Str_RFID_No,StrIP,strtemp_stationNO,pin_type);
                }
           //04拧紧作业联锁,先查找设定的拧紧程序，然后根据程序匹配到对应的输出脚
           else  if (IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Userfor=="04")
                {
                if (IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].Prg_NO_Hex!="")
                  {
                    //1、首先查询设定的端子功能代码(包括输入、输出)
                     pin_type="01"; //01输入，02输出
                     Query_RFID_CAN_SetPIN_IO_Cycle_Start(Str_RFID_No,StrIP,strtemp_stationNO,pin_type);

                     pin_type="02"; //01输入，02输出
                     Query_RFID_CAN_SetPIN_IO_Cycle_Start(Str_RFID_No,StrIP,strtemp_stationNO,pin_type);


                     //2、其次查询设定的模块程序
                     Query_RFID_CAN_SetPrg_IO_ByPrg_Cycle_Start(Str_EGShortNo,Str_RFID_No,StrIP,strtemp_stationNO,StrMinPrgNo);

                     //3、根据设定的端子功能代码，将查询得到的程序输出到相应的端子上（即将相应的功能端子置位1）

                     pin_type="02"; //01输入，02输出

                     Make_RFID_CAN_Pin_Prg_IO(Str_RFID_No,StrIP,strtemp_stationNO);

                     //4、查找作业完成输入信号脚、复位输出信号脚

                     //Make_RFID_CAN_Pin_Finished_Reset_IO(Str_RFID_No,strtemp_stationNO);



                  }  //判断 IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].Prg_NO_Hex<>''结束
                }
      }
}



void  MainWindow::Query_RFID_CAN_SetPrg_IO_ByPrg_Cycle_Start(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP,const QString strtemp_stationNO,const QString StrMinPrgNo)
{
QSqlQueryModel ADOQ;
QString strSQL;
qint32 strCount,j;
qint32 k,m;
QString PrgNO,strtemp;

k=Str_RFID_No.toInt();
m=strtemp_stationNO.toInt();

IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].Prg_NO_Bin="0000000000000000";

strSQL="select count(ID) from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"' AND IR_Station_NO='"+strtemp_stationNO+"' AND IR_Finished='"+strtemp_NG+"' AND IR_Prg_NO='"+StrMinPrgNo+"'";

ADOQ.setQuery(strSQL);
strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();

if(strCount<1){
    if (k==3){
        qDebug()<<"Query_RFID_CAN_SetPrg_IO_ByPrg_Cycle_Start:03no_prg";

    }
    else if(k==4){

        qDebug()<<"Query_RFID_CAN_SetPrg_IO_ByPrg_Cycle_Start:04no_prg";

    }
 return;
}

strSQL="select * from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"' AND IR_Station_NO='"+strtemp_stationNO+"' AND IR_Finished='"+strtemp_NG+"' AND IR_Prg_NO='"+StrMinPrgNo+"'";
if (k==3){
this->AddState("程序查找");
}
else if(k==4){

this->AddState2("程序查找");
}
ADOQ.setQuery(strSQL);
//取得一共有多少条件记录

strCount = ADOQ.rowCount();
//循环将各条记录的各字段内容赋值给各数组变量
for(j=0;j<strCount;j++)
{

    //取得程序号 16进制
    strtemp=strtemp.setNum((ADOQ.record(j).value("IR_Prg_NO")).toInt(),16);
    IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].Prg_NO_Hex= strtemp;//程序号

    //将取得程序号转换为2进制的输出文本，以实现与设定端子针脚扯上关系
    strtemp=IntToBin_ok((ADOQ.record(j).value("IR_Prg_NO")).toInt(),16);
    /*
    if (k==3){
        this->AddState("IO站"+strtemp_stationNO+"的设定程序号--"+strtemp+"");
    }
    else if (k==4){
        this->AddState2("IO站"+strtemp_stationNO+"的设定程序号--"+strtemp+"");
    }
    */

    PrgNO= strtemp;

    //取得程序号 2进制
    IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].Prg_NO_Bin=PrgNO;//程序号



}

}



//匹配输出针脚与程序的关系，产生一个输出模块的程序
//需要注意复位脚的输出
void MainWindow::Make_RFID_CAN_Pin_Prg_IO(const QString Str_RFID_No,const QString StrIP,const QString strtemp_stationNO)
{
  qint32 i,k,m;
  QString strTemp,strTempPRG,strTempPRG_OUT;


  k=Str_RFID_No.toInt();
  m=strtemp_stationNO.toInt();
    IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Output_State="0000000000000000";

    for(i=1;i<17;i++)
      {
          //strIR_OPUT_PIN[i]为人工分配的端子功能代码
          strTemp=IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].strIR_OPUT_PIN[i];

         if (strTemp.length()>0)
            {

                strTempPRG=IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].Prg_NO_Bin;

                // 追加判断当前输出脚是否为复位输出脚
                if (strTemp!="06")   //如果不是复位输出脚
                   {
                       if (IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].Prg_NO_Bin[16-i]=='1')
                        {
                             IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Output_State[16-strTemp.toInt()]='1';
                        }
                      else
                        {
                             IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Output_State[16-strTemp.toInt()]='0';
                        }
                        strTempPRG_OUT=IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Output_State;

                  }
               else        //如果是复位输出脚
                  {

                        IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Reset=Query_Reset_Signal(StrIP,Str_RFID_No,strtemp_stationNO,strTemp);

                }
            }

    }
    /*
    if (k==3){
       this->AddState("IO站"+strtemp_stationNO+"的程序引脚输出状态--"+strTempPRG_OUT+"");
       this->AddState("IO站"+strtemp_stationNO+"的设定复位引脚号--"+IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Reset+"");
     }
    else if (k==4){
        this->AddState2("IO站"+strtemp_stationNO+"的程序引脚输出状态--"+strTempPRG_OUT+"");
        this->AddState2("IO站"+strtemp_stationNO+"的设定复位引脚号--"+IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Reset+"");
      }
      */
}

QString MainWindow::Query_Reset_Signal(const QString StrIP,const QString strRFID_NO,const QString strstation_NO,const QString functionNO)
{

QSqlQueryModel ADOQ;
QString strSQL;
qint32 strCount;
QString strtemp="0";

strSQL="select Count(IR_Pin_No_PC_BIT) from 3F_IR_STATION_PIN_INPUT Where IR_SET_IPaddr='"+StrIP+"' and IR_RFID_No='"+strRFID_NO+"'  AND IR_Station_NO='"+strstation_NO+"' and  IR_Pin_No_Set='"+functionNO+"'";

ADOQ.setQuery(strSQL);
strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
//strCount = (ADOQ.record(0).value("id")).toInt();
//取得一共有多少条件记录
//strCount = ADOQ.rowCount();
//循环将各条记录的各字段内容赋值给各数组变量
 if ( strCount>0 )
{

    strSQL="select IR_Pin_No_PC_BIT from 3F_IR_STATION_PIN_INPUT Where  IR_SET_IPaddr='"+StrIP+"'  and  IR_RFID_No='"+strRFID_NO+"'   AND  IR_Station_NO='"+strstation_NO+"'  and   IR_Pin_No_Set='"+functionNO+"' ";

    ADOQ.setQuery(strSQL);
    strtemp = (ADOQ.record(0).value("IR_Pin_No_PC_BIT")).toString();
}



 return strtemp;
}

//查找作业完成输入信号脚，复位程序信号脚
void MainWindow::Make_RFID_CAN_Pin_Finished_Reset_IO(const QString Str_RFID_No,const QString strtemp_stationNO)
{
qint32 i,k,m;
QString strTemp,strTempPRG,strTempPRG_OUT;
k=Str_RFID_No.toInt();
m=strtemp_stationNO.toInt();

    IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Output_State="0000000000000000";



    for(i=1;i<17;i++)
      {
          //strIR_OPUT_PIN[i]为人工分配的端子功能代码
          strTemp=IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].strIR_OPUT_PIN[i];


          if (strTemp.length()>0)
             {
                strTempPRG=IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].Prg_NO_Bin;

                // 追加判断当前输出脚是否为复位输出脚
                if (strTemp!="06")   //如果不是复位输出脚
                   {
                       if( IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].Prg_NO_Bin[16-i]=='1')
                        {
                             IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Output_State[16-strTemp.toInt()]='1';
                        }
                      else
                        {
                             IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Output_State[16-strTemp.toInt()]='0';
                        }
                        strTempPRG_OUT=IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Output_State;

                  }
               else        //如果是复位输出脚
                  {

                  }

          }

      }
    if (k==3){
       this->AddState("IO站--"+strtemp_stationNO+"的程序引脚输出状态--"+strTempPRG_OUT+"");
       this->AddState("IO站--"+strtemp_stationNO+"的设定复位引脚号--"+IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Reset+"");
     }
    else if (k==4){
        this->AddState2("IO站--"+strtemp_stationNO+"的程序引脚输出状态--"+strTempPRG_OUT+"");
        this->AddState2("IO站--"+strtemp_stationNO+"的设定复位引脚号--"+IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Reset+"");
      }

}

//首先查询数据表[3F_IR_STATION_PIN_INPUT]设定的端子针脚分配
//查询各IO站点对应的IO设定值，也就是拧紧程序号或者零件指示
void MainWindow::Query_RFID_CAN_SetPIN_IO_Cycle_Start(const QString Str_RFID_No,const QString StrIP,const QString strtemp_stationNO,const QString PIN_Type)
{

QSqlQueryModel ADOQ;
QString strSQL;
qint32 strCount,j,i;
qint32 k,m;
k=Str_RFID_No.toInt();
m=strtemp_stationNO.toInt();

strSQL="select count(ID) from 3F_IR_STATION_PIN_INPUT where  IR_SET_IPaddr='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"'  AND IR_Station_NO='"+strtemp_stationNO+"' AND IR_PIN_INPUT_OUTPUT='"+PIN_Type+"'";

ADOQ.setQuery(strSQL);
strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();

if(strCount<1){
    if (k==3){
        qDebug()<<"Query_RFID_CAN_SetPIN_IO_Cycle_Start:03no_prg";

    }
    else if(k==4){

        qDebug()<<"Query_RFID_CAN_SetPIN_IO_Cycle_Start:04no_prg";

    }
 return;
}

//首先查询数据表[3F_IR_STATION_PIN_INPUT]设定的端子针脚分配
strSQL="select * from 3F_IR_STATION_PIN_INPUT where  IR_SET_IPaddr='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"'  AND IR_Station_NO='"+strtemp_stationNO+"' AND IR_PIN_INPUT_OUTPUT='"+PIN_Type+"'";

ADOQ.setQuery(strSQL);

//取得一共有多少条件记录
strCount = ADOQ.rowCount();
//this->AddState("999999999-"+StrIP+"-"+Str_RFID_No+"--"+strtemp_stationNO+"--"+PIN_Type+"");

//循环将各条记录的各字段内容赋值给各数组变量
for(j=0;j<strCount;j++)
//if(strCount>0)
{
    //IO模块CAN站的端子编号
    i=ADOQ.record(j).value("IR_Pin_No_PC_BIT").toInt();

    //IO模块CAN站的端子设定功能代码
    if (PIN_Type=="01")     //输入端子
    {
         IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].strIR_IPUT_PIN[i]=ADOQ.record(j).value("IR_Pin_No_Set").toString();
        // this->AddState("lin03--"+Str_RFID_No+"--"+strtemp_stationNO+"--"+ADOQ.record(j).value("IR_Pin_No_PC_BIT").toString()+"");

    }
    else if (PIN_Type=="02") //输出端子
    {
         IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].strIR_OPUT_PIN[i]=ADOQ.record(j).value("IR_Pin_No_Set").toString();
        // this->AddState("lin04--"+Str_RFID_No+"--"+strtemp_stationNO+"--"+ADOQ.record(j).value("IR_Pin_No_PC_BIT").toString()+"");

    }
}



}


//------------------------------------------
//全部作业完成后初始化站数组和程序数组为0  ,也就是清除站点数据、程序选择数

void MainWindow::Clear_Array_PIN_PRG_CAN(const QString IO_Module_Type,qint32 i,qint32 j)
{

       if (IO_Module_Type=="01")  //8入
          {

          }
       else if (IO_Module_Type=="02") //8入8出模块
          {

             IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State="0000000000000000";
             IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Input_State="0000000000000000";

          }
       else if (IO_Module_Type=="03")  //8出模块
          {

          }
       else if (IO_Module_Type=="04")  //16入模块
          {

          }
       else if (IO_Module_Type=="05" ) //16出模块
          {

          }

}

//查询各IO站点对应的IO设定值，也就是取料零件数量，零件指示
void MainWindow::Query_RFID_CAN_SetValue_IO(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP,const QString strtemp_stationNO,const QString PIN_Type)
{

  qint32 k,m;
  QString strBin;
  QSqlQueryModel ADOQ;
  QString strSQL;
  qint32 strCount,j,i;
  k=Str_RFID_No.toInt();
  m=strtemp_stationNO.toInt();


  strSQL="select count(ID) from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"' AND IR_Station_NO='"+strtemp_stationNO+"' AND IR_Finished='"+strtemp_NG+"'";

  ADOQ.setQuery(strSQL);
  strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();

  if(strCount<1){
      if (k==3){
          qDebug()<<"Query_RFID_CAN_SetPIN_IO_Cycle_Start:03no_prg";

      }
      else if(k==4){

          qDebug()<<"Query_RFID_CAN_SetPIN_IO_Cycle_Start:04no_prg";

      }
   return ;
  }


  strSQL="select * from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_RFID_No='"+Str_RFID_No+"' AND IR_EGtype_NO='"+Str_EGShortNo+"' AND IR_Station_NO='"+strtemp_stationNO+"' AND IR_Finished='"+strtemp_NG+"'";

  ADOQ.setQuery(strSQL);
  //strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
  //strCount = (ADOQ.record(0).value("id")).toInt();
  //取得一共有多少条件记录
  strCount = ADOQ.rowCount();

  //循环将各条记录的各字段内容赋值给各数组变量
  for(j=0;j<strCount;j++)
  {
      i=ADOQ.record(j).value("IR_Pin_No_InPut_Set").toInt();//IO模块CAN站的站号，用于绑定该模块对应数组信息

      j=ADOQ.record(j).value("IR_Pin_No_OutPut_Set").toInt();//此值是输出针脚号也是取料数量值

      if (PIN_Type =="IN")
         {
           IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Input_State[i]='1';
         }
      else if (PIN_Type =="OUT")
         {
          //IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Output_State[j]='1';

         {
           //当作输出阵脚号时，无需转换
           IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Output_State[16-j]='1';
          }
           // 当作取料数量时，需要将此值再转换为输出针脚号
           strBin=strBin.setNum(j,2);
           IR_CAN_RFID_Arr[k].IR_Belong_IO_Arr[m].IR_IO_Pin_Output_State=strBin;

         } ;
  }

}

//发送零件程序给各站点
void MainWindow::SendPartshowPrgNO_to_IOStation(const qint32 intRFID_NO,const qint32 intMaxstationNO)//strStation_NO为RFID绑定的站号
{
  qint32 j;
  QString strtemp_stationNO;
  j=0 ;
  for(j=1;j<intMaxstationNO+1;j++)
     {
        strtemp_stationNO=IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Station_NO;

        if  ((IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Userfor=="02")&& (strtemp_stationNO.length()!=0))
           {
            /*
             if (intRFID_NO==3){
             this->AddState("读取现有作业清单"+IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State+"--"+strtemp_stationNO+"");
             }else if (intRFID_NO==4){
             this->AddState2("读取现有作业清单"+IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State+"--"+strtemp_stationNO+"");
             }
             */
             //delphi版考虑了引脚,其实零件指示根本没有必要考虑,直接全部输出FF,2进制11111111
             //Send_StationNO_PrgNO_TO_IO_PartshowPrgNO(intRFID_NO,j,2,strtemp_stationNO,IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);
             Send_StationNO_PrgNO_TO_IO_PartshowPrgNO(intRFID_NO,j,2,strtemp_stationNO,"11111111");


            }
     }
}

//发送拧紧程序给各站点
void MainWindow::SendGunPrgNO_to_IOStation_Continue(const QString strIP,const qint32 intRFID_NO,const QString strRFID_NO,const qint32 intMaxstationNO)//strStation_NO为RFID绑定的站号
{

    qint32 j,m;
    QString strtemp_stationNO,strtemp_IR_IO_Userfor;
    j=0 ;
    for(j=1;j<intMaxstationNO+1;j++)
    {
       strtemp_stationNO=IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Station_NO;
       strtemp_IR_IO_Userfor=IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Userfor;

       if ((strtemp_IR_IO_Userfor=="04") &&(strtemp_stationNO.length()==2))
         {

                                     //------------因为每个模块即每个站点，也可以理解为每把枪都有可能使用了多个程序
                                     //------------（换套筒时），所以在发送程序时采用由小到大的顺序发送，也就是需要
                                     //------------取得最小的程序号（根据i、j、IP值）然后发送给站点
                                     //查询数据表[3F_IR_RUN]中当前机型、当前RFID、当前站点、当前IP对应的最小程序号
                                          IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].MINPrg=QueryIR_MIN_PrgNO(strtemp_stationNO,strRFID_NO,strIP);


                                          //给数组IR_CAN_RFID_Arr[i]变量附值,包括各绑定RFID受控的IO 站点的设定值
                                          //设定值包括拧紧枪联锁
                                           Query_RFID_CAN_SetInfor_IO_ByPrg(IR_CAN_RFID_Arr[intRFID_NO].EGType_Flow,strRFID_NO,strIP,IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].MINPrg,strtemp_stationNO);

                                           if (intRFID_NO==3)
                                           {
                                               AddState("准备发送拧紧程序、复位给各站点"+strtemp_stationNO+"");
                                           }
                                           if (intRFID_NO==4)
                                           {
                                               AddState2("准备发送拧紧程序、复位给各站点"+strtemp_stationNO+"");
                                           }

                                            //首先单独发送一个程序号   作业联锁
                                          if (strtemp_stationNO.length()==2)
                                           {

                                           m= 0;
                                           if( IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Reset.length()==2)
                                             {
                                               m=IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Reset.toInt();//IO模块的CAN站号
                                             }

                                           if (m==0)
                                              {
                                               if (intRFID_NO==3)
                                               {
                                                   AddState("IO模块的针脚未分配"+IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Reset+"");
                                               }
                                               if (intRFID_NO==4)
                                               {
                                                   AddState2("IO模块的针脚未分配"+IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Reset+"");
                                               }
                                              }
                                           else
                                              {
                                               if (intRFID_NO==3)
                                               {
                                                   AddState("IO模块的程序发送");
                                               }
                                               if (intRFID_NO==4)
                                               {
                                                   AddState2("IO模块的程序发送");
                                               }
                                                  //发送复位信号
                                                  Send_StationNO_PrgNO_TO_IO_GunPrgNO_Continue(intRFID_NO,j,2,strtemp_stationNO,IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);
                                                  IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State[16-m]='1';

                                                  //其次同时发送程序号和复位信号
                                                  Send_StationNO_PrgNO_TO_IO_GunPrgNO_Continue(intRFID_NO,j,2,strtemp_stationNO,IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);

                                                  //-------------
                                                  IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State[16-m]='0';

                                                  //发送程序号
                                                  Send_StationNO_PrgNO_TO_IO_GunPrgNO_Continue(intRFID_NO,j,2,strtemp_stationNO,IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);

                                            }

                                          }//IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Station_NO 不为空判断结束


     }

   }//给数组IR_CAN_RFID_Arr[i]变量初始化 FOR循环结束
}

//根据站号，发送程序号--CAN总线
void MainWindow::Send_StationNO_PrgNO_TO_IO_GunPrgNO_Continue(const qint32 i_rfid_NO,const qint32 int_j,const qint32 Order_NO,const QString str_IR_Station_NO,const QString str_CMD_Value)//根据站号，发送零件指示信息或拧紧程序号
{
  qint32 i,j,k;
  QString Frame_PrgAll_First;
  //QString Frame_PrgAll_Second;
  QString strtemp;
  bool bok=false;
  i=i_rfid_NO;
  j=int_j;
  //CAN通信
   if (Order_NO==1 )      //发送拧紧程序
   {
        //如果某个IO站的联锁有效，则发送
                  if (IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Actived_Station_NO!="0")
                  {
                      for(j = (IR_RFID_First.Prg_NO_HaveSend[i].toInt()+1);j<Prg_Total.toInt()+1;j++)
                      {
                        if (IR_RFID_First.Prg_NO_HaveFinished[i][j])   //收到了作业完成合格信号，改成读数据库
                         {
                           if  (IR_RFID_First.Prg_NO[i][j]!="0")
                             {
                                Frame_PrgAll_First=IR_RFID_First.Prg_NO[i][j];//准备发送给CAN IO板的程序号
                                //frm_IR_Run.INIT_Operation(IR_RFID_First.Actived_Station_NO[i],Frame_PrgAll_First,Order_NO,CMD_COUMUNICATION.CMD_SELECTPRGNO);  //发送拧紧程序号给各站点
                                strtemp=strtemp.setNum(j,10);
                                IR_RFID_First.Prg_NO_HaveSend[i]=strtemp; //记录当前站刚发送完毕的程序号，用于当前机型在一个站内需要使用多个程序号
                                IR_RFID_First.Prg_NO_HaveFinished[i][j]=false;//当前程序未拧紧合格完成标识

                                break;//跳出当前FOR循环，进入下一个站
                             }
                         }
                      }
                     }
      }
     else if  (Order_NO==2)  //发送零件指示
      {
          //首先将2进制的16为“0000 0000 0000 0000” 转换为10进制,然后再转为16进制

            k=str_CMD_Value.toInt(&bok,2);
            strtemp=strtemp.setNum(k,16);

           SendCMD_TO_Station_By_CANCOM(str_IR_Station_NO,strtemp);

      }

}


//根据RFID号、站点号、IP值查询对应的在数据库表3F_IR_RUN中最小的程序号
//其实就是要实现先进先出
QString MainWindow::QueryIR_MIN_PrgNO(const QString Str_Station_No,const QString Str_RFID_No,const QString StrIP)
{
QString reTmpStr="0";
QSqlQueryModel ADOQ;
QString strSQL;
//qint32 strCount,j,i;
strSQL="select Min(IR_Prg_NO) from 3F_IR_RUN Where IR_RFID_No='"+Str_RFID_No+"' and IR_IP_ADD='"+StrIP+"' and  IR_Station_NO='"+Str_Station_No+"' and IR_Finished='"+strtemp_NG+"'";

ADOQ.setQuery(strSQL);
//strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
//strCount = (ADOQ.record(0).value("id")).toInt();
//取得一共有多少条件记录
//strCount = ADOQ.rowCount();
//循环将各条记录的各字段内容赋值给各数组变量
//for(j=0;j<strCount;j++)
//{
//}
reTmpStr= ADOQ.data(ADOQ.index(0, 0)).toString();

return reTmpStr;
}




//根据站号，发送程序号--CAN总线
void MainWindow::Send_StationNO_PrgNO_TO_IO_PartshowPrgNO(const qint32 i_RFID_NO,const qint32 int_j,const qint32 Order_NO,const QString str_IR_Station_NO,const QString str_CMD_Value)//根据站号，发送零件指示信息或拧紧程序号
{
  qint32 i,j,k;
  QString Frame_PrgAll_First;
  QString Frame_PrgAll_Second;
  QString strtemp;
  bool bok=false;
  i=i_RFID_NO;
  j=int_j;

  //CAN通信
   if (Order_NO==1 )      //发送拧紧程序
   {
        //如果某个IO站的联锁有效，则发送
                  if (IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Actived_Station_NO!="0")
                  {
                      for(j = (IR_RFID_First.Prg_NO_HaveSend[i].toInt()+1);j<Prg_Total.toInt()+1;j++)
                      {
                        if (IR_RFID_First.Prg_NO_HaveFinished[i][j])   //收到了作业完成合格信号，改成读数据库
                         {
                           if  (IR_RFID_First.Prg_NO[i][j]!="0")
                             {
                                Frame_PrgAll_First=IR_RFID_First.Prg_NO[i][j];//准备发送给CAN IO板的程序号
                                //frm_IR_Run.INIT_Operation(IR_RFID_First.Actived_Station_NO[i],Frame_PrgAll_First,Order_NO,CMD_COUMUNICATION.CMD_SELECTPRGNO);  //发送拧紧程序号给各站点
                                strtemp=strtemp.setNum(j,10);
                                IR_RFID_First.Prg_NO_HaveSend[i]=strtemp; //记录当前站刚发送完毕的程序号，用于当前机型在一个站内需要使用多个程序号
                                IR_RFID_First.Prg_NO_HaveFinished[i][j]=false;//当前程序未拧紧合格完成标识

                                break;//跳出当前FOR循环，进入下一个站
                             }
                         }
                      }
                     }
      }
     else if  (Order_NO==2)  //发送零件指示
      {
          //首先将2进制的16为“0000 0000 0000 0000” 转换为10进制,然后再转为16进制

            k=str_CMD_Value.toInt(&bok,2);
            strtemp=strtemp.setNum(k,16);

            SendCMD_TO_Station_By_CANCOM(str_IR_Station_NO,strtemp);

      }

}


//联锁循环开始
void MainWindow::IR_Cycle_Start_IO(const QString strIP,const QString strRFID_NO)//strStation_NO为RFID绑定的站号
{
    qint32 i;
    QString strEGtypeFlow;
    //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$   can
    //此处应该插入判断当前是否为未完成作业的最老的ID记录，并确认是否置位IR_RFID_First.MIN_ID_YES ，IR_RFID_Second.MIN_ID_YES
    i=strRFID_NO.toInt(); //RFID的站号编号
    if(!IR_CAN_RFID_Arr[i].MIN_ID_YES)
       {


       //取得没有作业完成的最旧记录[3F_IR_RFID_READDATALIST]的ID,条件是字段ALL_FINISHED=0
       IR_CAN_RFID_Arr[i].MIN_ID=QueryIR_Olddest_ID(IR_CAN_RFID_Arr[i].RFID_Station_NO,strIP);//取得本RFID所管辖的最小ID

       //判断反馈回来的记录是否存在，如果有作业未完成的记录
       if  (IR_CAN_RFID_Arr[i].MIN_ID!="0")
           {
            //--------------------------------------------开始
            IR_CAN_RFID_Arr[i].MIN_ID_YES=true;

            //根据IR_CAN_RFID_Arr[i].MIN_ID
            //根据先入先出，可以知道最先入的ID为最小，故可以根据ID对应的取得机型
            strEGtypeFlow=QueryIR_Olddest(IR_CAN_RFID_Arr[i].MIN_ID);
            //追加显示最新循环的机型和流水
            if (IR_Infor.ControlforLine_Manu_Auto=="02") //02自动线
                {
                  DisplayForRun(strRFID_NO,strEGtypeFlow);
                 }
            IR_CAN_RFID_Arr[i].EGType_Flow=strEGtypeFlow;
            IR_CAN_RFID_Arr[i].ShortEGType=strEGtypeFlow.mid(0,3);

            IR_CAN_RFID_Arr[i].RFID_EGTypeGroupvalue=QueryTypeGroupNO_Cycle_Start(IR_CAN_RFID_Arr[i].ShortEGType);


            //判断当前机型是否有设定作业联锁
            if (!Check_Interlock_Set(IR_CAN_RFID_Arr[i].ShortEGType,IR_CAN_RFID_Arr[i].RFID_Station_NO,strIP))
                 {  //5{--------------------
                    //处理没有作业联锁的事情
                    QueryInFor_NOINput_IR_Cycle_Start_IO(strIP,strRFID_NO,i);

                 }
            else
                 {
                  //查询有作业联锁的内容

                    QueryInFor_HaveINput_IR_Cycle_Start_IO(strIP,strRFID_NO,i);

                 }//5end--------------------

           }
           else
           {
              //AddState("作业完成——");
           }
      }
      else
      {
        //Frm_IR_Main.AddState("作业完成");
      }
}

//防止站号重复，查询[3F_IR_INPUT]   QueryTypeGroupNO
QString MainWindow:: QueryTypeGroupNO_Cycle_Start(const QString StrEGtype)
{

    QString reTmpStr="00";
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;
    strSQL="select count(机型分组) from 3F_EGtype where  短型号='"+StrEGtype+"'";

    ADOQ.setQuery(strSQL);
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    //strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    //for(j=0;j<strCount;j++)
    //{
    //}
    if (strCount>0)
    {
        strSQL="select distinct(机型分组) from 3F_EGtype where  短型号='"+StrEGtype+"'";
        ADOQ.setQuery(strSQL);
        reTmpStr= ADOQ.data(ADOQ.index(0, 0)).toString();
    }

      return reTmpStr;

}


//根据最小ID值，查询最旧的机型流水号
QString MainWindow::QueryIR_Olddest(const QString StrMinID)
{
QString reTmpStr="0";
QSqlQueryModel ADOQ;
QString strSQL;
qint32 strCount;
strSQL="select ENG_TypeFlow from 3F_IR_RFID_READDATALIST Where ID='"+StrMinID+"'";

ADOQ.setQuery(strSQL);
//strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
reTmpStr = ADOQ.data(ADOQ.index(0, 0)).toString();
//strCount = (ADOQ.record(0).value("id")).toInt();
//取得一共有多少条件记录
//strCount = ADOQ.rowCount();
//循环将各条记录的各字段内容赋值给各数组变量
//for(j=0;j<strCount;j++)
//{
//}
if (strCount>0)
{

}

  return reTmpStr;
}


//根据工序号查询对应工序内的在数据库表3F_IR_RFID_READDATALIST中ID值最小的记录及机型流水号
//其实就是要实现先进先出
QString MainWindow::QueryIR_Olddest_ID(const QString Str_RFID_No,const QString StrIP)
{
  QString reTmpStr="0";
  QSqlQueryModel ADOQ;
  QString strSQL;
  qint32 strCount;
  strSQL="select count(ID) from 3F_IR_RFID_READDATALIST Where RFID_NO='"+Str_RFID_No+"' and IP_ADD='"+StrIP+"' and  ALL_FINISHED=0";

  ADOQ.setQuery(strSQL);
  strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
//strCount = (ADOQ.record(0).value("id")).toInt();
//取得一共有多少条件记录
//strCount = ADOQ.rowCount();
//循环将各条记录的各字段内容赋值给各数组变量
//for(j=0;j<strCount;j++)
//{
//}
  if (strCount>0)
  {
      strSQL="select MIN(ID) from 3F_IR_RFID_READDATALIST Where RFID_NO='"+Str_RFID_No+"' and IP_ADD='"+StrIP+"' and  ALL_FINISHED=0";

    //if (IR_Infor.ControlType=="01")
    //  {
    //    strSQL="select MIN(ID) from 3F_IR_RFID_READDATALIST Where RFID_NO='"+Str_RFID_No+"' and IP_ADD='"+StrIP+"' and  ALL_FINISHED=0";

    //}
   // else if (IR_Infor.ControlType=="00")
   //   {
   //     strSQL="select Max(ID) from 3F_IR_RFID_READDATALIST Where RFID_NO='"+Str_RFID_No+"' and IP_ADD='"+StrIP+"' and  ALL_FINISHED=0";
   //   }
    ADOQ.setQuery(strSQL);
    reTmpStr= ADOQ.data(ADOQ.index(0, 0)).toString();
  }
  return reTmpStr;

}


void MainWindow::QueryInFor_NOINput_IR_Cycle_Start_IO(const QString strIP,const QString strRFID_NO,const qint32 i)//strStation_NO为RFID绑定的站号
{
    QString strEGtypeFlow;
                 if (i==3)
                   {
                   AddState("串口3读取的机型无需作业，放行");
                   }
                 else if (i==4)
                   {
                   AddState2("串口4读取的机型无需作业，放行");
                   }
                   IR_CAN_RFID_Arr[i].IR_Belong_STOPGO.STOPGO_State="6";

                   strEGtypeFlow=IR_CAN_RFID_Arr[i].EGType_Flow;
                   //在[3F_IR_RUN]联锁数据表插入一条空记录,用于完成于有作业联锁相同的循环功能
                   //InsertRecord_IR_Partlist_IO_NOINput(IR_CAN_RFID_Arr[i].EGType_Flow,IR_CAN_RFID_Arr[i].MIN_ID,strRFID_NO,strIP);
                   InsertRecord_IR_Partlist_IO_NOINput(IR_CAN_RFID_Arr[i].ShortEGType,IR_CAN_RFID_Arr[i].MIN_ID,strRFID_NO,strIP);

                   if (IR_Infor.PC_DB_IO=="01")
                   {
                     //初直---------------------------------------
                        if  (IR_CAN_RFID_Arr[i].RFID_Station_NO=="03")
                           {
                              AddState("串口3读取的机型新循环中");
                              PC_ANDON_WriteFIN="00";
                           }
                        else if  (IR_CAN_RFID_Arr[i].RFID_Station_NO=="04")
                           {
                              AddState2("串口4读取的机型新循环中");
                              PC_ANDON_WriteFIN_Second="00";
                           }
                    //--------------------------------------------
                   }

                   //阻挡气缸
                   IR_CAN_RFID_Arr[i].IR_Belong_STOPGO.STOPGO_State="1";


                   //将机型短码strEGtypeFlow、RFID编号strRFID_NO存储在配置文件中
                   //如果为单机操作时需要保存
                   if (IR_Infor.ControlType=="01")
                      {
                         //将机型短码strEGtypeFlow、RFID编号strRFID_NO存储在配置文件中
                         SaveEGtypeFlow_RFID_NO_ToFile(strEGtypeFlow,strRFID_NO);

                         if (strRFID_NO==strStart_stationNO)  //前工序
                            {
                                 COUNT_NO=1;
                            }
                         else if (strRFID_NO==strEnd_stationNO) //后工序
                            {
                                 COUNT_NO2=1;
                            }
                      }

}
//单机联锁时，将当前机型和RFID编号保存，供重新启动系统时检索3F_IR_RUN中的信息
void MainWindow::SaveEGtypeFlow_RFID_NO_ToFile(const QString strEGtypeFlow,const QString strRFID_NO)
{
    if (IR_Infor.filepath_01win02linux=="01")
    {
        QSettings setting("D:/ubuntu14share/qtprojects/gac/gacfile/gacconfig.ini", QSettings::IniFormat);
        setting.beginGroup("ir");
        if (strRFID_NO=="03")
        {
          this->AddState("03写机型入文档"+strEGtypeFlow+"");
          setting.setValue("Process03_EGTypeFlow",strEGtypeFlow);
          IR_Infor.Current_EGtypeFlow_first=strEGtypeFlow;
        }
        else  if (strRFID_NO=="04")
        {
            this->AddState2("04写机型入文档"+strEGtypeFlow+"");
         setting.setValue("Process04_EGTypeFlow",strEGtypeFlow);
          IR_Infor.Current_EGtypeFlow_second=strEGtypeFlow;
        }
        setting.endGroup();
    }


    else if (IR_Infor.filepath_01win02linux=="02")
    {
        QSettings setting("gacfile/gacconfig.ini", QSettings::IniFormat);
        setting.beginGroup("ir");
        if (strRFID_NO=="03")
        {
          this->AddState("03写机型入文档"+strEGtypeFlow+"");
          setting.setValue("Process03_EGTypeFlow",strEGtypeFlow);
          IR_Infor.Current_EGtypeFlow_first=strEGtypeFlow;
        }
        else  if (strRFID_NO=="04")
        {
            this->AddState2("04写机型入文档"+strEGtypeFlow+"");
         setting.setValue("Process04_EGTypeFlow",strEGtypeFlow);
          IR_Infor.Current_EGtypeFlow_second=strEGtypeFlow;
        }
        setting.endGroup();
    }



}



//修改自动线、手动线时的不同参数值
//void SaveParameter_ToFile ()
//{
    /*
if (IR_Infor.filepath_01win02linux=="01")
{
    QSettings setting("D:/ubuntu14share/qtprojects/gac/gacfile/gacconfig.ini", QSettings::IniFormat);
    setting.beginGroup("ir");
    if (strRFID_NO=="03")
    {
      this->AddState("修改使用场合、控制模式参数，写机型入文档");
      setting.setValue("Process03_EGTypeFlow",strEGtypeFlow);
      IR_Infor.Current_EGtypeFlow_first=strEGtypeFlow;
    }
    setting.endGroup();
}


else if (IR_Infor.filepath_01win02linux=="02")
{
    QSettings setting("gacfile/gacconfig.ini", QSettings::IniFormat);
    setting.beginGroup("ir");
    if (strRFID_NO=="03")
    {
      this->AddState("修改使用场合、控制模式参数，写机型入文档");
      setting.setValue("RFIDtype",strEGtypeFlow);//使用的RFID类型
      setting.setValue("IR_IP",strEGtypeFlow);//联锁IP
      setting.setValue("ControlModel_Manu_Auto",strEGtypeFlow);//手动线，自动线
      setting.setValue("01DB02IO",strEGtypeFlow);//总线作业完成信号触发方式，手动线使用02IO，自动线01DB
      setting.setValue("01win02linux",strEGtypeFlow);//程序运行环境是WINDOWS还是LINUX
      IR_Infor.Current_EGtypeFlow_first=strEGtypeFlow;
    }
    setting.endGroup();
}
*/
//}
//对于没有作业的需要的机型短码,在读取后需要插入一条空记录,以保证正常
void MainWindow::InsertRecord_IR_Partlist_IO_NOINput(const QString Str_EGShortNo,const QString Str_MINID,const QString Str_RFID_No,const QString StrIP)
{
   QString strSQL;
   QString strtemp="00";

   QSqlQuery * query = new QSqlQuery();

   strSQL="delete  from 3F_IR_RUN Where  (IR_IP_ADD='"+StrIP+"') and (IR_RFID_No='"+Str_RFID_No+"')";

   query->exec(strSQL);


   strSQL="insert into 3F_IR_RUN(IR_RFID_No,IR_EGtype,IR_EGtype_NO,IR_PARTlist_Name,IR_PARTlist_NO,IR_GUNInfor_NO,";
   strSQL=strSQL+"IR_GUNInfor_Name,IR_Prg_NO,IR_Station_NO,IR_Part_Count,IR_Finished,IR_Valid,IR_IP_ADD,";
   strSQL=strSQL+"IR_PROCESS_No,IR_MIN_ID_OF_LIST,IR_PIN_InPut_Length,IR_PIN_OutPut_Length,IR_PLC_D,IR_Pin_No_InPut_Set,";
   strSQL=strSQL+"IR_Pin_No_OutPut_Set,IR_Model_No,IR_Model_Usefor) ";
   strSQL=strSQL+"values('"+Str_RFID_No+"','"+Str_EGShortNo+"','"+strtemp+"','"+strtemp+"',";
   strSQL=strSQL+"'"+strtemp+"','"+strtemp+"','"+strtemp+"','"+strtemp+"',";
   strSQL=strSQL+"'"+strtemp+"','"+strtemp+"','"+strtemp_OK+"',";
   strSQL=strSQL+"'"+bool_ok+"','"+StrIP+"','"+strtemp+"',";
   strSQL=strSQL+"'"+Str_MINID+"','"+strtemp+"','"+strtemp+"',";
   strSQL=strSQL+"'"+strtemp+"','"+strtemp+"','"+strtemp+"','"+strtemp+"','"+strtemp+"')";
   query->exec(strSQL);

        //绑定的RFID编号，在数据表[3F_CAN_RFID_STATIONNO_SET][3F_IR_INPUT]中
        //联锁对象，IO模块的站号，不能与绑定的RFID的站号重复，因为大家同在一个CAN网络
        //用于跟踪是由哪条记录相关的,此值应该是[3F_IR_RFID_READDATALIST]的ID
}



void MainWindow::QueryInFor_HaveINput_IR_Cycle_Start_IO(const QString strIP,const QString strRFID_NO,const qint32 i)//strStation_NO为RFID绑定的站号
{
  QString strtemp,strEGtypeFlow;


                   //根据读取的机型短码，查询[3F_IR_RUN]联锁数据设定表中，对应此机型、此绑定的RFID
                   //管控的相关联锁内容，而相关联锁的IO模块的站号就是[3F_IR_RUN]的IR_Station_NO字段
                   Query_IR_Partlist_IO(IR_CAN_RFID_Arr[i].EGType_Flow,IR_CAN_RFID_Arr[i].MIN_ID,strRFID_NO,strIP);//传递机型短码和当前工序号

                   strEGtypeFlow=IR_CAN_RFID_Arr[i].EGType_Flow;
                   if (IR_Infor.PC_DB_IO=="01")
                   {
                     //初直---------------------------------------
                        if  (IR_CAN_RFID_Arr[i].RFID_Station_NO=="03")
                           {
                              AddState("串口03新循环中");
                              PC_ANDON_WriteFIN="00";
                           }
                        else if  (IR_CAN_RFID_Arr[i].RFID_Station_NO=="04")
                           {
                              AddState2("串口04新循环中");
                              PC_ANDON_WriteFIN_Second="00";
                           }
                    //--------------------------------------------
                   }

                   //阻挡气缸
                   IR_CAN_RFID_Arr[i].IR_Belong_STOPGO.STOPGO_State="1";


                   //将机型短码strEGtypeFlow、RFID编号strRFID_NO存储在配置文件中
                   //如果为单机操作时需要保存
                   if (IR_Infor.ControlType=="01")
                      {
                         //将机型短码strEGtypeFlow、RFID编号strRFID_NO存储在配置文件中
                         SaveEGtypeFlow_RFID_NO_ToFile(strEGtypeFlow,strRFID_NO);

                         if (strRFID_NO==strStart_stationNO)    //前工序
                            {
                                 COUNT_NO=1;
                            }
                         else if (strRFID_NO==strEnd_stationNO)  //后工序
                            {
                                 COUNT_NO2=1;
                            }


                      }

                   //显示给操作看，但因为当前

                   //查询绑定的RFID受控的IO站数 [3F_IR_RUN]的字段[IR_Station_NO]中最大的值
                   if (strRFID_NO==strStart_stationNO)    //前工序
                      {
                         AddState("查询作业记录"+IR_CAN_RFID_Arr[i].EGType_Flow+"-"+strRFID_NO+"--"+strIP+"");
                      }
                   else if (strRFID_NO==strEnd_stationNO)  //后工序
                      {
                         AddState2("查询作业记录"+IR_CAN_RFID_Arr[i].EGType_Flow+"-"+strRFID_NO+"--"+strIP+"");
                      }

                   strtemp=Query_IO_CAN_MaxStationNO_Cycle(IR_CAN_RFID_Arr[i].EGType_Flow.mid(0,3),strRFID_NO,strIP);
                  //追加一个DataModule_3F.Query_IO_CAN_MaxStationNO变量为空时的处理，防止转换为整数时出错

                   if (strRFID_NO==strStart_stationNO)    //前工序
                      {
                         AddState("查询作业记录"+strtemp+"-");
                      }
                   else if (strRFID_NO==strEnd_stationNO)  //后工序
                      {
                         AddState2("查询作业记录"+strtemp+"-");
                      }
                   if (strtemp.length()!=0)
                     {//4{--------------------
                       if (strtemp.toInt()==0)
                        {

                        }
                        else
                        {   //3{--------------------
                           if (strRFID_NO==strStart_stationNO)    //前工序
                           {
                                AddState("查询联锁IO站数成功");      //将相关信息写到画面
                           }
                           else if (strRFID_NO==strEnd_stationNO)  //后工序
                           {
                                AddState2("查询联锁IO站数成功");      //将相关信息写到画面
                           }

                            IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO=strtemp.toInt();
                            IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr=new IR_IOTYPE[IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO+1];
                            //setlength(IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr,IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO+1);


                            //一、初始化对应RFID管辖的IO站点信息变量.IR_IO_Station_NO,.IR_IO_Userfor

                            InitIOStationInfor(strIP,i,strRFID_NO,IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO);

                            //发送给IO站点
                            //1、零件指示
                            //给数组IR_CAN_RFID_Arr[i]变量附值,包括各绑定RFID受控的IO 站点的设定值
                            //二、设定值包括零件指示

                            Query_RFID_CAN_SetInfor_IO_ByUsefor(IR_CAN_RFID_Arr[i].EGType_Flow.mid(0,3),strRFID_NO,strIP,"02");

                            //三、发送零件指示程序给各站点

                            SendPartshowPrgNO_to_IOStation(i,IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO);



                            //四、查询、发送拧紧程序给各站点

                            SendGunPrgNO_to_IOStation(strIP,i,strRFID_NO,IR_CAN_RFID_Arr[i].RFID_CAN_MaxMAPStationNO);
//return;//测试

                         } //3}--------------------


                  } //4}--------------------
                else
                  {
                    AddState("对应的RFID没有设定记录");      //将相关信息写到画面
                  }
//---------------------

}



void  MainWindow:: Query_IR_Partlist_IO(const QString Str_EGType_Flow,const QString Str_MINID,const QString Str_RFID_No,const QString StrIP)
{

    QSqlQuery * query = new QSqlQuery();
    QString Str_EGShortNo=Str_EGType_Flow.mid(0,3);
    QString strSQL;

    strSQL="delete  from 3F_IR_RUN Where  IR_IP_ADD='"+StrIP+"' and IR_RFID_No='"+Str_RFID_No+"' AND IR_Finished='"+strtemp_OK+"'"; //删除动态记录该工序已经完成的作业记录，需要追加该工序WHERE [IR_RFID_No]="""+Str_RFID_No+""" AND [IR_EGtype_Flow_No]="""+Str_EGShortNo+"""
    query->exec(strSQL);


    QSqlQueryModel ADOQ;
    QString strTemp;
    QString strTemp_value[24];
    qint32 strCount,j,strCount_query;
    //qint32 strRet;
    //strRet=0;
//首先查询有多少条记录
    strSQL="select count(ID) from 3F_IR_INPUT Where IR_EGtype_NO='"+Str_EGShortNo+"' and IR_SET_IPaddr='"+StrIP+"' and IR_RFID_No='"+Str_RFID_No+"'";

    ADOQ.setQuery(strSQL);
    strCount_query = ADOQ.data(ADOQ.index(0, 0)).toInt();
    if (strCount_query<1)
    {
        return;
    }


    strSQL="select * from 3F_IR_INPUT Where IR_EGtype_NO='"+Str_EGShortNo+"' and IR_SET_IPaddr='"+StrIP+"' and IR_RFID_No='"+Str_RFID_No+"'";

    ADOQ.setQuery(strSQL);

    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    //strCount = ADOQ.rowCount();
    //strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //循环将各条记录的各字段内容赋值给各数组变量
    for(j=0;j<strCount_query;j++)
    {
        //绑定的RFID编号，在数据表[3F_CAN_RFID_STATIONNO_SET][3F_IR_INPUT]中
      strTemp_value[1]=ADOQ.record(j).value("IR_RFID_No").toString();
      strTemp_value[23]=Str_EGType_Flow;
      strTemp_value[2]=ADOQ.record(j).value("IR_EGtype").toString();
      strTemp_value[3]=ADOQ.record(j).value("IR_EGtype_NO").toString();
      strTemp_value[4]=ADOQ.record(j).value("IR_PARTlist_Name").toString();
      strTemp_value[5]=ADOQ.record(j).value("IR_PARTlist_NO").toString();
      strTemp_value[6]=ADOQ.record(j).value("IR_GUNInfor_NO").toString();
      strTemp_value[7]=ADOQ.record(j).value("IR_GUNInfor_Name").toString();
      strTemp_value[8]=ADOQ.record(j).value("IR_Prg_NO").toString();
       if (strTemp_value[8].length()==0)
          {
            strTemp_value[8]="01";
          }

       ////联锁对象，IO模块的站号，不能与绑定的RFID的站号重复，因为大家同在一个CAN网络
       strTemp_value[9]=ADOQ.record(j).value("IR_Station_NO").toString();
       strTemp_value[10]=ADOQ.record(j).value("IR_Part_Count").toString();
       strTemp_value[11]=strtemp_NG;//ADOQ1.FieldByName("IR_Finished").AsString="否";//
       //strTemp_value[12]=bool_ok;//ADOQ1.FieldByName("IR_Valid").AsBoolean=TRUE;
       strTemp_value[13]=ADOQ.record(j).value("IR_SET_IPaddr").toString();//ADOQ1.FieldByName("IR_IP_ADD").AsString
       strTemp_value[14]=ADOQ.record(j).value("IR_PROCESS_No").toString();//Frm_IR_Main.ComB_RFID_No_first.Text;
       //用于跟踪是由哪条记录相关的,此值应该是[3F_IR_RFID_READDATALIST]的ID
       strTemp_value[15]=Str_MINID;//ADOQ1.FieldByName("IR_MIN_ID_OF_LIST").AsString

       //零件指示除了站号还必须知道对应站模块上的哪几个IO点
       strTemp_value[16]=ADOQ.record(j).value("IR_PIN_InPut_Length").toString();
       strTemp_value[17]=ADOQ.record(j).value("IR_PIN_OutPut_Length").toString();
       strTemp_value[18]=ADOQ.record(j).value("IR_PLC_D").toString();
       strTemp_value[19]=ADOQ.record(j).value("IR_Pin_No_InPut_Set").toString();


       //如果是零件取料完成模块时strTemp_value[19]是两位的针脚号,
       //如果是拧紧枪的模块,则为字段内容为[请选择],具体的针脚--合格完成输入信号,需要去数据表[3F_IR_STATION_PIN_INPUT]中寻找

       if (strTemp_value[19].length()!=2)  //其实为‘请选择’
          {
           if (strTemp_value[1]=="03")
           {
           this->AddState(""+strTemp_value[13]+"----"+strTemp_value[1]+"----"+strTemp_value[9]+"");
           }
           else if (strTemp_value[1]=="04")
           {
           this->AddState2(""+strTemp_value[13]+"----"+strTemp_value[1]+"----"+strTemp_value[9]+"");
           }

             //strTemp_value[19]=Query_FinishedOk_Signal(strTemp_value[13],strTemp_value[1],strTemp_value[9]);

          }
       strTemp_value[20]=ADOQ.record(j).value("IR_Pin_No_OutPut_Set").toString(); //零件指示联锁
       if (strTemp_value[20].length()!=2 )    //‘请选择’
          {
                        //作业指示联锁
             strTemp_value[20]="99";

          }

       strTemp_value[21]=ADOQ.record(j).value("IR_Model_No").toString();
       strTemp_value[22]=ADOQ.record(j).value("IR_Model_Usefor").toString();


        QSqlQuery * query = new QSqlQuery();
        strTemp="insert into 3F_IR_RUN(IR_RFID_No,IR_EGtype_Flow_No,IR_EGtype,IR_EGtype_NO,";
        strTemp=strTemp+"IR_PARTlist_Name,IR_PARTlist_NO,IR_GUNInfor_NO,IR_GUNInfor_Name,IR_Prg_NO,";
        strTemp=strTemp+"IR_Station_NO,IR_Part_Count,IR_Finished,IR_Valid,IR_IP_ADD,IR_PROCESS_No,IR_MIN_ID_OF_LIST,";
        strTemp=strTemp+"IR_PIN_InPut_Length,IR_PIN_OutPut_Length,IR_PLC_D,IR_Pin_No_InPut_Set,IR_Pin_No_OutPut_Set,";
        strTemp=strTemp+"IR_Model_No,IR_Model_Usefor";
        strTemp=strTemp+")";
        strTemp=strTemp+"values('"+strTemp_value[1]+"','"+strTemp_value[23]+"','"+strTemp_value[2]+"',";
        strTemp=strTemp+"'"+strTemp_value[3]+"','"+strTemp_value[4]+"','"+strTemp_value[5]+"','"+strTemp_value[6]+"',";
        strTemp=strTemp+"'"+strTemp_value[7]+"','"+strTemp_value[8]+"','"+strTemp_value[9]+"','"+strTemp_value[10]+"',";
        strTemp=strTemp+"'"+strTemp_value[11]+"','"+bool_ok+"','"+strTemp_value[13]+"','"+strTemp_value[14]+"',";
        strTemp=strTemp+"'"+strTemp_value[15]+"','"+strTemp_value[16]+"','"+strTemp_value[17]+"','"+strTemp_value[18]+"',";
        strTemp=strTemp+"'"+strTemp_value[19]+"','"+strTemp_value[20]+"','"+strTemp_value[21]+"','"+strTemp_value[22]+"'";

        strTemp=strTemp+")";
        query->exec(strTemp);



    }
}


QString MainWindow::Query_FinishedOk_Signal(const QString StrIP,const QString strRFID_NO,const QString strstation_NO)
{

    QSqlQueryModel ADOQ;
    QString strSQL;
    QString strFinished,restrtemp;

    strFinished ="全部合格完成";


    strSQL="select IR_Pin_No_PC_BIT from 3F_IR_STATION_PIN_INPUT Where (IR_SET_IPaddr='"+StrIP+"') and (IR_RFID_No='"+strRFID_NO+"')  AND (IR_Station_NO='"+strstation_NO+"') and  (IR_Function_Desc='"+strFinished+"')";

    ADOQ.setQuery(strSQL);

    restrtemp=(ADOQ.record(0).value("IR_Pin_No_PC_BIT")).toString();

    return restrtemp;
}


//发送拧紧程序给各站点
void MainWindow::SendGunPrgNO_to_IOStation(const QString strIP,qint32 intRFID_NO,const QString strRFID_NO,qint32 intMaxstationNO)//strStation_NO为RFID绑定的站号
{
    qint32 j,m;
    QString strtemp_stationNO,testtemp;
    j=0;

                            //给数组IR_CAN_RFID_Arr[i]变量初始化
           for(j=1;j<intMaxstationNO+1;j++)
                                {
                                      //2{--------------------
                                      strtemp_stationNO=IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Station_NO;
                                      if ((IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Userfor=="04") &&(strtemp_stationNO.length()!=0))
                                      {

                                            //------------因为每个模块即每个站点，也可以理解为每把枪都有可能使用了多个程序
                                            //------------（换套筒时），所以在发送程序时采用由小到大的顺序发送，也就是需要
                                            //------------取得最小的程序号（根据i、j、IP值）然后发送给站点
                                            //查询数据表[3F_IR_RUN]中当前机型、当前RFID、当前站点、当前IP对应的最小程序号
                                            IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].MINPrg=QueryIR_MIN_PrgNO(strtemp_stationNO,strRFID_NO,strIP);
                                            //1{--------------------
                                            //if IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Userfor="04"
                                               {
                                                  //给数组IR_CAN_RFID_Arr[i]变量附值,包括各绑定RFID受控的IO 站点的设定值
                                                  //设定值包括拧紧枪联锁
                                                  Query_RFID_CAN_SetInfor_IO_ByPrg(IR_CAN_RFID_Arr[intRFID_NO].EGType_Flow.mid(0,3),strRFID_NO,strIP,IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].MINPrg,strtemp_stationNO);


                                                   //首先单独发送一个程序号   作业联锁
                                                  if (strtemp_stationNO.length()!=0)
                                                  {

                                                  m= 0;

                                                  if (IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Reset.length()!=0)
                                                    {
                                                      m=IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Reset.toInt();//IO模块的CAN站号
                                                    }
                                                 /*
                                                  if (strRFID_NO==strStart_stationNO)    //前工序
                                                  {
                                                       AddState("查询拧紧程序3'"+strIP+"'-'"+testtemp.setNum(intMaxstationNO,10)+"'-'"+m+"'");       //将相关信息写到画面
                                                  }
                                                  else if (strRFID_NO==strEnd_stationNO)  //后工序
                                                  {
                                                       AddState2("查询拧紧程序3'"+strIP+"'-'"+testtemp.setNum(intMaxstationNO,10)+"'-'"+m+"'");      //将相关信息写到画面
                                                  }
                                                 */

                                                  //0{--------------------

                                                  if (m==0)
                                                     {
                                                      if (strRFID_NO==strStart_stationNO)    //前工序
                                                      {
                                                           AddState("IO模块的针脚未分配");       //将相关信息写到画面
                                                      }
                                                      else if (strRFID_NO==strEnd_stationNO)  //后工序
                                                      {
                                                           AddState2("IO模块的针脚未分配");      //将相关信息写到画面
                                                      }

                                                     }
                                                  else
                                                     {

                                                        IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State_Reset="0000000000000000";
                                                        //复位
                                                        IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State_Reset[16-m]='1';

                                                        Send_StationNO_PrgNO_TO_IO_GunPrgNO(intRFID_NO,j,2,strtemp_stationNO,IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State_Reset);


                                                        //发送程序号
                                                        Send_StationNO_PrgNO_TO_IO_GunPrgNO(intRFID_NO,j,2,strtemp_stationNO,IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);

                                                        //其次同时发送程序号和复位信号
                                                        IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State[16-m]='1';
                                                        Send_StationNO_PrgNO_TO_IO_GunPrgNO(intRFID_NO,j,2,strtemp_stationNO,IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);

                                                        //-------------
                                                        IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State[16-m]='0';
                                                        //发送程序号
                                                        Send_StationNO_PrgNO_TO_IO_GunPrgNO(intRFID_NO,j,2,strtemp_stationNO,IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);


                                                        if (intRFID_NO==3)    //前工序
                                                        {
                                                             AddState("发送程序IO站数成功");      //将相关信息写到画面
                                                        }
                                                        else if (intRFID_NO==4)  //后工序
                                                        {
                                                             AddState2("发送程序IO站数成功");      //将相关信息写到画面
                                                        }


                                                      }

                                                     }   //IR_CAN_RFID_Arr[intRFID_NO].IR_Belong_IO_Arr[j].IR_IO_Station_NO不为空判断结束
                                                    //0}--------------------
                                                }
                                                //1}--------------------
                                         }
                                         //2}--------------------
                                }
                                 //3}--------------------
}


//根据站号，发送程序号--CAN总线
void MainWindow::Send_StationNO_PrgNO_TO_IO_GunPrgNO(const qint32 i_rfid_NO,const qint32 int_j,const qint32 Order_NO,const QString str_IR_Station_NO,const QString str_CMD_Value)//根据站号，发送零件指示信息或拧紧程序号
{
  qint32 i,j,k;
  QString Frame_PrgAll_First;
  QString Frame_PrgAll_Second;
  QString strtemp;
  bool bok=false;
  i=i_rfid_NO;
  j=int_j;
  //CAN通信
   if (Order_NO==1 )      //发送拧紧程序
   {
        //如果某个IO站的联锁有效，则发送
                  if (IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Actived_Station_NO!="0")
                  {
                      for(j = (IR_RFID_First.Prg_NO_HaveSend[i].toInt()+1);j<Prg_Total.toInt()+1;j++)
                      {
                        if (IR_RFID_First.Prg_NO_HaveFinished[i][j])   //收到了作业完成合格信号，改成读数据库
                         {
                           if  (IR_RFID_First.Prg_NO[i][j]!="0")
                             {
                                Frame_PrgAll_First=IR_RFID_First.Prg_NO[i][j];//准备发送给CAN IO板的程序号
                                //frm_IR_Run.INIT_Operation(IR_RFID_First.Actived_Station_NO[i],Frame_PrgAll_First,Order_NO,CMD_COUMUNICATION.CMD_SELECTPRGNO);  //发送拧紧程序号给各站点
                                strtemp=strtemp.setNum(j,10);
                                IR_RFID_First.Prg_NO_HaveSend[i]=strtemp; //记录当前站刚发送完毕的程序号，用于当前机型在一个站内需要使用多个程序号
                                IR_RFID_First.Prg_NO_HaveFinished[i][j]=false;//当前程序未拧紧合格完成标识

                                break;//跳出当前FOR循环，进入下一个站
                             }
                         }
                      }
                     }
      }
     else if  (Order_NO==2)  //发送零件指示
      {
          //首先将2进制的16为“0000 0000 0000 0000” 转换为10进制,然后再转为16进制

            k=str_CMD_Value.toInt(&bok,2);
            strtemp=strtemp.setNum(k,16);

            SendCMD_TO_Station_By_CANCOM(str_IR_Station_NO,strtemp);

      }

}

 MainWindow::~MainWindow()
{
 /*
    if(myCom2 != NULL){
            if(myCom2->isOpen()){
                myCom2->close();
            }
            delete myCom2;
     }

    if(myCom3 != NULL){
            if(myCom3->isOpen()){
                myCom3->close();
            }
            delete myCom3;
     }
    if(myCom4 != NULL){
            if(myCom4->isOpen()){
                myCom4->close();
            }
            delete myCom4;
     }
    */
    delete ui;
}



//向CAN的IO模块发送输出指令 str_IR_Station_NO为IO模块的站号
void MainWindow::SendCMD_TO_Station_By_CANCOM(const QString str_IR_Station_NO,const QString str_CMD_Value)
{
 QString INC_value;
 QString strValue;
 QString CMDValue;

       CMDValue=Make_CAN_CMD(str_IR_Station_NO,str_CMD_Value);

       ReadWrite_BySpcom(CAN_COM,CMDValue);

}


QString MainWindow::Make_CAN_CMD(const QString str_IR_Station_NO,const QString str_CMD_Value)
{
  QString strValue,CMD_Value;
  qint32 i_stationno;

  //1将站号转换为两位的16进制,比如dec 12->hex c->hex 0c->大写hex 0C

        i_stationno=str_IR_Station_NO.toInt();//字符转换为10进制数值
       if (i_stationno<16  ){
           strValue="0"+strValue.setNum(i_stationno,16);//将10进制数值转换为16进制字符串
       }
       else{
           strValue=strValue.setNum(i_stationno,16);//将10进制数值转换为16进制字符串
       }

      if (str_CMD_Value.length()==1 ){
          CMD_Value="0"+str_CMD_Value;
      }
      else{
          CMD_Value=str_CMD_Value;
      }



        //strValue=CMD_COUMUNICATION.CMD_IDSend_CAN+strValue+CMD_COUMUNICATION.CMD_IDSend_CAN+str_CMD_Value+CMD_COUMUNICATION.CMD_IDRec_CAN;
        strValue=CMD_COUMUNICATION.CMD_IDSend_CAN+" "+strValue+" "+CMD_COUMUNICATION.CMD_IDSend_CAN+" "+CMD_Value+" "+CMD_COUMUNICATION.CMD_IDRec_CAN;

     // 2  转换为大写16进制格式
        strValue=strValue.toUpper();
        return strValue;
}



//str_RFID_NO串口号
void MainWindow::ReadWrite_BySpcom(const QString str_RFID_NO,const QString str_CMD_Value)
{
 QString strValue;
 qint32 i;
       if ((str_RFID_NO=="01" ) |(str_RFID_NO=="03" ) |(str_RFID_NO=="04" )|(str_RFID_NO=="05" )| (str_RFID_NO=="06" ))
       {
        i=str_RFID_NO.toInt();
        strValue=str_CMD_Value;
         if (str_RFID_NO=="03" ) {
          AddState(""+str_RFID_NO+"发送:"+strValue+"");
        }
        else if(str_RFID_NO=="04" ) {
          AddState2(""+str_RFID_NO+"发送:"+strValue+"");
                }
        else{
          AddState(""+str_RFID_NO+"发送:"+strValue+"");
          AddState2(""+str_RFID_NO+"发送:"+strValue+"");
          }
        orderLst_CAN_RFID_COM[i]=strValue;
        sendData_CAN_RFID_BySpcom(i);
       }
      else if(str_RFID_NO=="02" )
       {
           i=str_RFID_NO.toInt();
           strValue=str_CMD_Value;
           //AddState(""+str_RFID_NO+"发送:"+strValue+"");
           //AddState2(""+str_RFID_NO+"发送:"+strValue+"");
           orderLst_CAN_RFID_COM[i]=strValue;
           sendData_CAN_RFID_BySpcom_IO(i);

       }
}

//初始化连接到COM2,用于can模块的输入输出
void MainWindow::initComm2()
{

    QString portName=this->strCom2;
    myCom2 = new QextSerialPort(portName);
    //关联读串口Slot函数
   // connect(myCom2, SIGNAL(readyRead()), this, SLOT(readMyCom2()));//使用定时器轮询方式
    //设置波特率
     myCom2->setBaudRate((BaudRateType)this->strBaudRate2.toInt());
     //设置数据位
     myCom2->setDataBits((DataBitsType)8);
     //设置校验位
     myCom2->setParity(PAR_NONE);
     //设置停止位
     myCom2->setStopBits(STOP_1);
     //设置数据流控制
     myCom2->setFlowControl(FLOW_OFF);

     //设置延时
    // myCom2->setTimeout(TIME_OUT);
     myCom2->setTimeout(1);//定时读取数据到缓冲区，
     if(myCom2->open(QIODevice::ReadWrite)){
            this->AddState("com2<<=:已成功打开串口");
            this->AddState2("com2<<=:已成功打开串口");
            //QMessageBox::information(this, tr("打开成功"), tr("已成功打开串口") + portName, QMessageBox::Ok);

     }else{
            QMessageBox::critical(this, tr("打开失败"), tr("未能打开串口 ") + portName + tr("\n该串口设备不存在或已被占用"), QMessageBox::Ok);
            return;
     }

}


//----------------------------------------------------------
//读取数据
//初始化连接到COM2,用于can模块的输入输出
void MainWindow::readMyCom2()
{
        QString tmpStr_first,tmpStr_end,tmpcount;
    //读取串口内容
    //16进制形式显示
        QByteArray temp = myCom2->readAll();//读取串口缓冲区的所有数据给临时变量temp
        QString buf;

  /*
      // if(!temp.isEmpty())
       {
        QString RFID_NO="02";

        for(int i = 0; i < temp.count(); i++){
            QString s;
            s.sprintf("%02x", (unsigned char)temp.at(i));
            buf += s.toUpper();
        }

        this->AddState("com2<<=:"+buf+"");
        this->AddState2("com2<<=:"+buf+"");


        tmpStr_first=buf.mid(0, 2);
        tmpStr_end=buf.mid( buf.length()-2, 2);//获取帧尾，如果为55则处理，否则继续累加


        //if ((frame_first_com2.mid(0, 2)=="AA")&&(tmpStr_end=="55"))//如果接收到的是一帧的结束符号，则直接处理
        if ((tmpStr_first=="AA")&&(tmpStr_end=="55"))//如果接收到的是一帧的结束符号，则直接处理
        {
                      frame_content_com2=buf;//累加，一直等到55

                      recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]=frame_content_com2;//用于处理接收到的内容
                      this->AddState("com2完整777<<=:"+frame_content_com2+"");
                      CheckCMD_CAN_COM2(RFID_NO);//处理接收到的内容

                      frame_content_com2="";
                      frame_first_com2="";
                }
        else if((tmpStr_first=="AA")&&(frame_first_com2==""))
        {
           frame_content_com2=buf;
           frame_first_com2=buf;
           //this->AddState("com2<<888=:"+frame_first_com2+"");
        }

        else if ((frame_first_com2.mid(0, 2)=="AA")&&(tmpStr_end!="55"))
        {
           frame_content_com2=frame_content_com2+buf;
           //this->AddState("com2<<999=:"+frame_first_com2+"");
        }
        else if ((frame_first_com2.mid(0, 2)=="AA")&&(tmpStr_end=="55"))//如果接收到的是一帧的结束符号，则直接处理
        {
              frame_content_com2=frame_content_com2+buf;//累加，一直等到55

              recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]=frame_content_com2;//用于处理接收到的内容
              this->AddState("com2完整<<=:"+frame_content_com2+"");
              CheckCMD_CAN_COM2(RFID_NO);//处理接收到的内容

              frame_content_com2="";
              frame_first_com2="";
        }
        else
        {
              frame_content_com2="";
              temp ="";
        }


      }
*/
        ///*
         if(!temp.isEmpty())
         {

          for(int i = 0; i < temp.count(); i++)
          {
              QString s;
              s.sprintf("%02x", (unsigned char)temp.at(i));
              buf += s.toUpper();
          }

          this->AddState("com2<<=:"+buf+"");
          this->AddState2("com2<<=:"+buf+"");


          tmpStr_first=buf.mid(0, 2);
          tmpStr_end=buf.mid( buf.length()-2, 2);//获取帧尾，如果为55则处理，否则继续累加
          if ((tmpStr_first=="AA")&&(tmpStr_end=="55"))//如果接收到的是一帧的结束符号，则直接处理
                  {
                        frame_content_com2=buf;//
                        if (i_recv==i_recv_total)
                        {
                            i_recv=0;
                        }
                        recData_from_COM2[i_recv]=frame_content_com2;//用于处理接收到的内容

                        this->AddState("com2完整777--"+tmpcount.setNum(i_recv,10)+"<<=:"+frame_content_com2+"");
                        i_recv=i_recv+1;
                        frame_content_com2="";
                        frame_first_com2="";
                  }
          else if((tmpStr_first=="AA")&&(tmpStr_end!="55"))
          {
               if((tmpStr_first=="AA")&&(frame_first_com2==""))
              {
                 frame_content_com2=buf;
                 frame_first_com2=buf;
                 //this->AddState("com2<<888=:"+frame_first_com2+"");
              }
          }


          else if ((frame_first_com2.mid(0, 2)=="AA")&&(tmpStr_end!="55"))
          {
             frame_content_com2=frame_content_com2+buf;
             //this->AddState("com2<<999=:"+frame_first_com2+"");
          }
          else if ((frame_first_com2.mid(0, 2)=="AA")&&(tmpStr_end=="55"))//如果接收到的是一帧的结束符号，则直接处理
          {
                frame_content_com2=frame_content_com2+buf;//累加，一直等到55
                if (i_recv==i_recv_total)
                {
                    i_recv=0;
                }
                recData_from_COM2[i_recv]=frame_content_com2;//用于处理接收到的内容


                this->AddState("com2完整"+tmpcount.setNum(i_recv,10)+"<<=:"+frame_content_com2+"");
                i_recv=i_recv+1;
                frame_content_com2="";
                frame_first_com2="";
          }
          else
          {
                frame_content_com2="";
                temp ="";
          }


        }
//*/
}
//----------------------------------------------------------
void MainWindow::Timer_COM2_Progress()
{
    CheckCMD_CAN_COM2_FORTIMER();
}

void MainWindow::CheckCMD_CAN_COM2_FORTIMER()
{

       qint32 i ,j,i_count;
       QString tmpStr,Fram_Head,Fram_End,Fram_Content,testtemp;


      //首先截取接收的信息
          Fram_Head=CMD_COUMUNICATION.CMD_IDRec_CAN; //AA
          Fram_End=CMD_COUMUNICATION.CMD_IDSend_CAN;  //55



          for(i_count=0;i_count<i_recv_total;i_count++)
             {
               j=0;
               if (recData_from_COM2[i_count]!="**")
               {
                   tmpStr= recData_from_COM2[i_count];
                   //处理第1帧数据
                        for(i=0;i<tmpStr.length();i++)
                           {

                            if (tmpStr.mid(i,2)==Fram_End)  //帧尾55
                             {
                               Fram_Content=tmpStr.mid(j, i+2-j);//截取完整的一帧数据
                               this->AddState("==第"+testtemp.setNum(i_count,10)+"=j="+testtemp.setNum(j,10)+"=="+Fram_Content+"====");
                               Check_Station_Fram_IO("02",Fram_Content);  //根据取得的帧内容，进行相关事件处理
                               j= i+2;
                             }

                           }
                        recData_from_COM2[i_count]="**";

               }
             }



}

void MainWindow::CheckCMD_CAN_COM2(QString RFID_NO)
{

       qint32 i ,j;
       QString tmpStr,Fram_Head,Fram_End,Fram_Content;
       qint32 Fram_Head_Count ;
       QString content1,content4;

      //首先截取接收的信息
          Fram_Head=CMD_COUMUNICATION.CMD_IDRec_CAN; //AA
          Fram_End=CMD_COUMUNICATION.CMD_IDSend_CAN;  //55
          j=0;
          Fram_Head_Count=0;
          tmpStr= recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()];

          content1 = tmpStr.mid( 0, 2); //帧头AA
          content4 = tmpStr.mid( tmpStr.length()-2, 2); //帧尾55


          if ((content1==Fram_Head) && (content4==Fram_End))
             {
           //因为回来的信号可能是一起的，比如AA800055AA40000255AA070055,所以要截取，分别处理

         //处理第1帧数据
              for(i=0;i<tmpStr.length();i++)
                 {

                  if (tmpStr.mid(i,2)==Fram_End)  //帧尾55
                   {
                     Fram_Content=tmpStr.mid(j, i+2-j);//截取完整的一帧数据
                     this->AddState("==="+Fram_Content+"====");
                     Check_Station_Fram_IO(RFID_NO,Fram_Content);  //根据取得的帧内容，进行相关事件处理
                     j= i+2;
                   }

                 }

          }
}


void MainWindow::Check_Station_Fram_IO(QString RFID_NO,QString strFram_Content)
{
   // /*
   qint32 k;
   bool ok;
   QString tmpStr,Fram_Head,Fram_End,Strbit;
   QString content1,content2,content3,content4,strIP;

  //首先截取接收的信息
      Fram_Head=CMD_COUMUNICATION.CMD_IDRec_CAN; //AA
      Fram_End=CMD_COUMUNICATION.CMD_IDSend_CAN;  //55

      tmpStr= strFram_Content;
      content1 = tmpStr.mid(0, 2); //帧头AA
      //content3 = tmpStr.mid(2, 2); //帧头AA
      content4 = tmpStr.mid(tmpStr.length()-2, 2); //帧尾55
      if ((content1==Fram_Head) && (content4==Fram_End) && (tmpStr.length()==8))
         {
          this->AddState("+++++"+tmpStr+"-++++");
      //以下为处理从CAN站的IO输入信号反抗的数据
         content2 = tmpStr.mid( 2, 2);  //反馈站号

      //修改对应输出针脚的状态

         k = content2.toInt(&ok,16);
         if (k<10)
           {
              content2="0"+content2.setNum(k,10);

           }
         else
           {
              content2=content2.setNum(k,10);
           }

         //首先根据反馈的站号k，是否比数据表[3F_CAN_RFID_STATIONNO_SET]中是最大的RFID_CAN_MaxStationNO小，否则数组
         //IR_CAN_RFID_Arr[k]或报警出错
         if (!(k> RFID_CAN_MaxStationNO))
            {
                strIP=IR_CAN_RFID_Arr[k].RFID_IP_ADD;
            }
         else
            {
                strIP=IR_Infor.IP_ADD;
            }//反馈的站点信号，

         //{
         //根据反馈的站号，及配置的IP地址，查询在数据表[3F_CAN_RFID_STATIONNO_SET]中是否存在
         //1、如果不存在，则认为是IO模块，而不是RFID模块,其实只要设定的不是1-6就行,因为1-6

        // if (QueryIR_RFIDSET_LIST_BE(strIP,content2) )
            {

        //---------------- 20130223插入判断IO输入指令 开始----------

             //if  ((tmpStr.mid(0, 2)=="AA") && (tmpStr.mid( 6, 2)=="55"))      //因为IO反馈的指令AA+站号+输入状态+55，总长度为8
                 {

                    if  (tmpStr.mid(4, 2)!="00")       //检测开关状态值（上升沿）
                     {

                       //检查托盘到位信号
                       //Check_TuopanArraved(strStationNO:string;int_up_down:integer);
                       //--------------处理零件指示、拧紧联锁作业完成的信息开始---------------
                       //1、在接收到作业完成的开关信号后，更新数据表[3F_IR_RUN]中对应
                       //绑定的RFID受控的对应IO模块站号对应的机型代码对应的字段[IR_Finished]
                       //将16进制的数据转换为2进制的，然后判断哪个BIT，根据此BIT更新记录
                       Strbit="00";
                       Strbit=Check_Bit_Signal(tmpStr.mid(4, 2),16);
                       this->AddState(""+strIP+"-站号"+content2+"-针脚号"+Strbit+"");

                       if ((Strbit!="00")&(Strbit.length()==2))
                          {

                            //查询[3F_IR_RUN]中是否有当前模块对应的未完成的联锁作业，
                            //20140630--追加读取试漏数据的程序---开始
/*
                            //检查试漏启动、停止操作事件，操作的信号从7号站获取
                             CheckIR_Leak_Start_End(content2,Strbit);

                            //检查试漏结果为NG事件,NG信号从14号站获取
                             CheckIR_Leak_Result_NG(content2,Strbit);
                               //20140630--追加读取试漏数据的程序---结束
                             AddState('FROM:'+content2+'号站'+Strbit+'');
*/
                             if (Query_IR_RUN_FIN_IO_BystationNO(strIP,content2))
                                {

                                   //根据IP、站点号、Strbit值（输入针脚号）查找对应位输出针脚号，然后依据此针脚号改变输出状态
                                   RFID_NO=Query_RFID_NO(content2,strIP);
                                   if (RFID_NO=="03")
                                   {
                                   this->AddState("----rfid"+RFID_NO+"IO模块输入");
                                   }
                                   else if (RFID_NO=="04")
                                   {
                                   this->AddState2("----rfid"+RFID_NO+"IO模块输入");
                                   }
                                   //根据IP、站点号、更新记录表[3F_IR_RUN]中的字段[IR_Finished]
                                   //检查有无连接外围设备比如试漏机，等需要采集数据的
                                   //检查更新联锁作业数据表的记录
                                   CheckIR_SubDeviceOP_and_NODevice(RFID_NO,strIP,content2,Strbit);
                                }
                             else
                                {
                                 if (IR_Infor.ControlforLine_Manu_Auto=="01") //01手动线
                                    {
                                    //------------------------------------------------------
                                      //判断在阻挡器记录表中是否有设置记录开始
                                      //查询[3F_IR_STATION_PIN_INPUT_STOPGO]中是否有当前模块对应的阻挡气缸和复位开关信号，

                                      //if DataModule_3F.Query_IR_INPUT_STOPGO_BystationNO(strIP,content2,Strbit)
                                        {
                                            // 复位阻挡汽缸  取消查询阻挡设置记录20160809-lin
                                            //WorkThread.EnableSTOPGO_Reset_BYStation(strIP,content2,Strbit);


                                           //当系统接受到复位开关信号时的处理事件 ,实际操作时应该是将卡远离卡头，然后碰复位开关


                                             if (content2=="252")
                                               {
                                                 EnableSTOPGO_Reset(3);//复位阻挡器
                                                 IO_ResetCheck_3F_COM(Start_stationNO);

                                                 AddState("IO_IO:'"+content2+"'&&&03复位");

                                               }
                                             else if (content2=="254")
                                               {
                                                 EnableSTOPGO_Reset(4);
                                                 //此函数为专门针对当前特殊工序RFID04而设定
                                                  //写入ID卡 (COM1连接的RFID) --------缸盖分装线特殊要求
                                                 if (IR_Infor.Sub_Device_First !="00")
                                                 {
                                                        if (IR_Infor.Sub_Device_First=="01")
                                                           {
                                                              EnableSTOPGO_Reset_BYStation_temp(IR_Infor.ReadRFID_NoPosition_StationNO_First);
                                                           }
                                                        else
                                                           {

                                                           }
                                                  }
                                                 if (IR_Infor.Sub_Device_Second !="00" )
                                                     {
                                                        if (IR_Infor.Sub_Device_Second=="01")
                                                           {

                                                              EnableSTOPGO_Reset_BYStation_temp(IR_Infor.ReadRFID_NoPosition_StationNO_Second);
                                                           }
                                                        else
                                                           {
                                                               ;
                                                           }
                                                     }

                                                 IO_ResetCheck_3F_COM(End_stationNO);
                                                 AddState2("IO_IO:'"+content2+"'&&&04复位");
                                                 //
                                               }

                                          }

                                   //判断在阻挡器记录表中是否有设置记录结束
                                   //----------------------------------------------------------

                                       }
                                     else if (IR_Infor.ControlforLine_Manu_Auto=="02") //02自动线
                                       {
                                         //判断在阻挡器记录表中是否有设置记录结束
                                         //查询[3F_IR_STATION_PIN_INPUT_STOPGO]中是否有当前模块对应的阻挡气缸和复位开关信号，

                                        //if DataModule_3F.Query_IR_INPUT_STOPGO_BystationNO(strIP,content2,Strbit)
                                          {
                                          // 复位阻挡汽缸  取消查询阻挡设置记录20160809-lin
                                            // EnableSTOPGO_Reset_BYStation(strIP,content2,Strbit);


                                             //当系统接受到复位开关信号时的处理事件 ,实际操作时应该是将卡远离卡头，然后碰复位开关

                                         if (content2=="252")
                                            {
                                               EnableSTOPGO_Reset(3);
                                               reset3="1";
                                               IO_ResetCheck_3F_COM_ForAutoLine_New(Start_stationNO);
                                               AddState("IO_IO:"+content2+"&&&03复位");
                                               reset3="2";
                                             }
                                           else if (content2=="254")
                                             {
                                                EnableSTOPGO_Reset(4);
                                                reset4="1";
                                                IO_ResetCheck_3F_COM_ForAutoLine_New(End_stationNO);
                                                AddState2("IO_IO:"+content2+"&&&04复位");
                                                reset4="2";
                                              }

                                            }
                                       }
                                }
                          }
                      else
                          {
                           AddState("当前输入信号站点未设置");
                          }
                      //--------------处理零件指示、拧紧联锁作业完成的信息开始---------------
                     }
                   else if (tmpStr.mid( 4, 2)=="00")    //检测开关状态值（下降沿）
                     {
                        //this->AddState(""+strIP+"-"+content2+"");
                        //this->AddState(""+strIP+"-"+Strbit+"");
                        //return;
                                        if(tmpStr=="AA800055AA400055")
                                            {
                                                 content2="64";

                                                 IO_ResetCheck_3F_COM_ForAutoLine_New(Start_stationNO);
                                                 AddState("IO_IO:"+content2+"&&&03复位");
                                                 content2="128";
                                                 IO_ResetCheck_3F_COM_ForAutoLine_New(End_stationNO);
                                                 AddState2("IO_IO:"+content2+"&&&04复位");

                                            }
                                        else
                                         {
                                           if ((content2=="64" ) &&(reset3!="2" ))
                                             {
                                               IO_ResetCheck_3F_COM_ForAutoLine_New(Start_stationNO);
                                               AddState("IO_IO:"+content2+"&&&03复位");
                                               SendCMD_TO_Station_By_CANCOM("64","00");
                                               SendCMD_TO_Station_By_CANCOM("64","00");
                                             }
                                           else if ((content2=="128") && (reset4!="2" ))
                                             {

                                               IO_ResetCheck_3F_COM_ForAutoLine_New(End_stationNO);
                                               AddState2("IO_IO:"+content2+"&&&04复位");
                                               SendCMD_TO_Station_By_CANCOM("128","00");
                                               SendCMD_TO_Station_By_CANCOM("128","00");

                                             }
                                         }
                                         if (reset3=="2" )
                                         {
                                              reset3="3";
                                         }
                                         if (reset4=="2")
                                         {
                                              reset4="3";
                                         }
                   }

              }
          }//-----if (QueryIR_RFIDSET_LIST_BE(strIP,content2) )判断结束

        }
}

//检查试漏结果为OK事件,NG信号从14号站获取(把试漏机看成一把拧紧枪)
void MainWindow::CheckIR_SubDeviceOP_and_NODevice(const QString strRFID_NO,const QString strIP,const QString strStation_NO,const QString Strbit)
{
    // QString strWriteIDValue;
//00未连接，试漏机01或阿特拉斯02或技研的拧紧枪03且需要采集数据
  if (strRFID_NO==strStart_stationNO)
    {
 //-------------------------------------
   //如果有来自设定的外接辅助设备的模块信号
        if (strStation_NO==IR_Infor.Sub_Device_StationNO_First )
             {
            /*
               if (IR_Infor.Sub_Device_First!="00")
                {
                      if (IR_Infor.Sub_Device_First=="01" )
                           {
                               AddState("试漏程序1结果合格");
                               //更新试漏程序1为作业完成
                               DataModule_3F.Update_IR_RUN_RE_FIN_IO(strIP,strStation_NO,Strbit,WorkThread.ADOQ_First);
                               //检查试漏结果为OK事件,OK信号从14号站获取，试漏数据连接到 是串口1
                               CheckIR_Leak_Result_OK(strIP,strStation_NO,Strbit);
                           }
                      else   if (IR_Infor.Sub_Device_First=="02")
                           {

                           }
                      else   if (IR_Infor.Sub_Device_First=="03")
                           {

                           }
                      else   if (IR_Infor.Sub_Device_First=="04")
                           {

                           }
                      else   if (IR_Infor.Sub_Device_First=="05")
                           { //连接的是OMRON的RFID
                              if (strRFID_NO=="03" )
                                {
                                  AddState(""+strRFID_NO+"-"+strStation_NO+"RFID正在更新运行记录");

                                  DataModule_3F.Update_IR_RUN_RE_FIN_IO(strIP,strStation_NO,Strbit,WorkThread.ADOQ_First);
                                  AddState(""+strRFID_NO+"-"+strStation_NO+"RFID完成更新运行记录");

                                  //AddState("COM1准备写入机型流水");
                                }

                              //写入ID卡 (COM1连接的RFID) --------缸体分装线特殊要求
                              AddState("机型流水:"+IR_Infor.Current_EGtypeFlow_first+"");
                              strWriteIDValue:=ICFunction.WriteMDS_OMRON_BySpcom("0010",Trimright(IR_Infor.Current_EGtypeFlow_first));
                              ReadWrite_BySpcom(BarCode_COM,strWriteIDValue);

                              //重新检索未作业完成的信息
                              WorkThread.IR_Cycle_ReStart_IO(strIP,strStation_NO,Strbit);
                              WorkThread.Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_first,strStart_stationNO,IR_Infor.IP_ADD);

                           }
                      else   if (IR_Infor.Sub_Device_First=="06")
                           {

                           }
              }
               */
         }
        else
          {
              AddState(""+strRFID_NO+"-"+strStation_NO+"RFID正在更新运行记录");
              AddState(""+strRFID_NO+"更新记录"+strIP+"-"+strStation_NO+"-"+Strbit+"");
              Update_IR_RUN_RE_FIN_IO(strIP,strStation_NO,Strbit);
              AddState(""+strRFID_NO+"-"+strStation_NO+"RFID完成更新运行记录");

              IR_Cycle_ReStart_IO(strIP,strStation_NO,Strbit);
              Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_first,strStart_stationNO,IR_Infor.IP_ADD);

          }
    }
  else if (strRFID_NO==strEnd_stationNO)
    {
 //-------------------------------------
   //如果有来自设定的外接辅助设备的模块信号
        if (strStation_NO==IR_Infor.Sub_Device_StationNO_Second )
             {
            /*
               if (IR_Infor.Sub_Device_Second !="00" )
                {
                      if (IR_Infor.Sub_Device_Second=="01")
                           {
                               AddState("试漏程序1结果合格");
                               //更新试漏程序1为作业完成
                               DataModule_3F.Update_IR_RUN_RE_FIN_IO(strIP,strStation_NO,Strbit,WorkThread.ADOQ_Second);
                               //检查试漏结果为OK事件,OK信号从14号站获取，试漏数据连接到 是串口1
                               CheckIR_Leak_Result_OK(strIP,strStation_NO,Strbit);
                           }
                      else   if (IR_Infor.Sub_Device_Second=="02")
                           {

                           }
                      else   if (IR_Infor.Sub_Device_Second=="03")
                           {

                           }
                      else   if (IR_Infor.Sub_Device_Second=="04" )
                           {

                           }
                      else   if (IR_Infor.Sub_Device_Second=="05" )
                           {
                             if (strRFID_NO=="03")
                                {
                                  AddState2(""+strRFID_NO+"RFID正在更新运行记录");
                                  DataModule_3F.Update_IR_RUN_RE_FIN_IO(strIP,strStation_NO,Strbit,WorkThread.ADOQ_First);
                                  AddState2(""+strRFID_NO+"RFID完成更新运行记录");

                                   AddState2("COM1准备写入机型流水");
                                }
                              //写入ID卡 (COM1连接的RFID) --------缸体分装线特殊要求
                              AddState2("机型流水:"+IR_Infor.Current_EGtypeFlow_Second+"");
                              strWriteIDValue:=ICFunction.WriteMDS_OMRON_BySpcom("0010",Trimright(IR_Infor.Current_EGtypeFlow_Second));
                              ReadWrite_BySpcom(BarCode_COM,strWriteIDValue);

                              WorkThread.IR_Cycle_ReStart_IO(strIP,strStation_NO,Strbit);
                              Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_Second,strEnd_stationNO,IR_Infor.IP_ADD);

                           }
                      else   if (IR_Infor.Sub_Device_Second=="06")
                           {

                           }
              }
               */
         }
        else
          {
              AddState2(""+strRFID_NO+"RFID正在更新运行记录");
              Update_IR_RUN_RE_FIN_IO(strIP,strStation_NO,Strbit);
              AddState2(""+strRFID_NO+"RFID完成更新运行记录");
              IR_Cycle_ReStart_IO(strIP,strStation_NO,Strbit);
              Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_second,strRFID_NO,IR_Infor.IP_ADD);
          }
    }
}

//CAN串口使用
//更新[3F_IR_RUN]数据表(单机控制)
void MainWindow::Update_IR_RUN_RE_FIN_IO(const QString StrIP,const QString strstationNO,const QString strbit)
{

 QSqlQueryModel ADOQ;
 QString strSQL,strtemp;
 qint32 strCount;

 strSQL="select count(ID) from 3F_IR_RUN where IR_IP_ADD='"+StrIP+"' and IR_Station_NO='"+strstationNO+"'  and IR_Finished='"+strtemp_NG+"'";

 ADOQ.setQuery(strSQL);
 strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();


 //strCount = (ADOQ.record(0).value("id")).toInt();
 //取得一共有多少条件记录
 //strCount = ADOQ.rowCount();
 //循环将各条记录的各字段内容赋值给各数组变量
 if (strCount>0)
 {


      //如果有记录，则查询对应枪的最小程序记录，因为需要考虑换套筒，制定了从小到大选程序顺序的规则
     strSQL="select Min(IR_Prg_NO) from 3F_IR_RUN Where IR_IP_ADD='"+StrIP+"' and IR_Station_NO='"+strstationNO+"'  and IR_Finished='"+strtemp_NG+"'";

     ADOQ.setQuery(strSQL);
     strtemp = ADOQ.data(ADOQ.index(0, 0)).toString();

     //this->AddState("test-"+StrIP+"-"+strstationNO+"-"+strtemp+"");
     //QString strSQL;
     QSqlQuery * query = new QSqlQuery();
     strSQL="Update 3F_IR_RUN set IR_Finished='"+strtemp_OK+"' where IR_IP_ADD='"+StrIP+"' and IR_Station_NO='"+strstationNO+"'  and IR_Finished='"+strtemp_NG+"' and  IR_Prg_NO ='"+strtemp+"'";
     query->exec(strSQL);

  }
}



//联锁循环开始 后，当串口接收到有来自IO站的零件指示的反馈信息（取料完成）时，
//需要更新输出值，并输出到IO站，以熄灭指示灯
void  MainWindow::IR_Cycle_ReStart_IO(const QString strIP,const QString strStation_NO,const QString Str_InputPINNo)
{

      //QString strOutputPINNO;
      QString strIR_RFID_No;
      qint32 i,j,k;
      //QString strCmdValue;

      QSqlQueryModel ADOQ;
      QString strSQL;
      //qint32 strCount;

      k=0;
      strSQL="select  IR_Pin_No_OutPut_Set , IR_RFID_No  from  3F_IR_RUN  where  IR_IP_ADD='"+strIP+"' AND IR_Station_NO='"+strStation_NO+"'";

      ADOQ.setQuery(strSQL);
      //strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
      k=(ADOQ.record(0).value("IR_Pin_No_OutPut_Set")).toInt();//输出针脚号
      i=(ADOQ.record(0).value("IR_RFID_No")).toInt();//绑定的RFID编号
      strIR_RFID_No=(ADOQ.record(0).value("IR_RFID_No")).toString();//绑定的RFID编号
      j=strStation_NO.toInt();//IO模块的CAN站号


     //更新该IO站点对应的输出状态值

     //首先确认当前IO模块站对应的strStation_NO，作业完成输入针脚号

     if (k!=0)      //常规操作开关
     {
      if (k!=99)     //零件指示作业完成输入信号
        {
          //j=StrToInt(strStation_NO);//IO模块的CAN站号
          /*//当把[3F_IR_INPUT]设定的 字段[IR_Pin_No_OutPut_Set]作为针脚号时
                    //修改对应输出针脚的状态
                    IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State[17-k]:='0';
                    //根据站号，发送程序号---CAN总线通信
                    Send_StationNO_PrgNO_TO_IO_ReStart(2,strStation_NO,IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);
          */
          //当把 3F_IR_INPUT 设定的 字段 IR_Pin_No_OutPut_Set 作为取料数量时
          IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State="0000000000000000";
          Send_StationNO_PrgNO_TO_IO_GunPrgNO_Continue(i,j,2,strStation_NO,IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);

        }
      else  if (k==99)   //拧紧指示作业完成输入信号
        {

         //接收到该把枪当前程序作业完成的信号后，需要继续判断当前枪对应的站号在 3F_IR_RUN 数据表中是否还有未完成的任务
              if ((strStation_NO.length() !=0 ) & (strIR_RFID_No.length() !=0 ))
                 {
                  IR_Cycle_Cotinuce_IO_NOFinished(strIP,strIR_RFID_No,strStation_NO);
                 }
              else
                 {
                  AddState2("NGN"+strIR_RFID_No+"---"+strStation_NO+"");
                 }
        }
    }
}

//如果启动系统时检测到[3F_IR_RUN] 表中还有未完成的联锁作业，则
//联锁循环继续
void MainWindow::IR_Cycle_Cotinuce_IO_NOFinished(const QString strIP,const QString strRFID_NO,const QString strStationNO)//strStation_NO为RFID绑定的站号
{
qint32 i,j,m;
QString strtemp,strtemp_stationNO;

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$   can
    i=strRFID_NO.toInt(); //RFID的站号编号
    j=strStationNO.toInt(); //RFID的站号编号
    //发送给IO站点   2、拧紧枪程序选择、联锁指示
    //给数组IR_CAN_RFID_Arr[i]变量初始化
    strtemp_stationNO=IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Station_NO;

     if( (IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Userfor=="04") & (strtemp_stationNO.length()!=0) )
         {
                //------------因为每个模块即每个站点，也可以理解为每把枪都有可能使用了多个程序
                //------------（换套筒时），所以在发送程序时采用由小到大的顺序发送，也就是需要
                //------------取得最小的程序号（根据i、j、IP值）然后发送给站点
                 //查询数据表[3F_IR_RUN]中当前机型、当前RFID、当前站点、当前IP对应的最小程序号

                //IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].MINPrg=QueryIR_MIN_PrgNO_ReStart(IR_CAN_RFID_Arr[i].EGType_Flow,strtemp_stationNO,strRFID_NO,strIP);
                IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].MINPrg=QueryIR_MIN_PrgNO(strtemp_stationNO,strRFID_NO,strIP);
                strtemp=IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].MINPrg;


                //只有程序大于0才表示还有第二个或更多程序
                if (strtemp.toInt()>0 )
                 {
                    if(strRFID_NO=="03")
                   {
                    this->AddState("rfid--"+strRFID_NO+"-站点"+strStationNO+"的第二个或第三或第四个程序号----"+strtemp+"");
                   }
                    else if(strRFID_NO=="04")
                    {
                     this->AddState2("rfid--"+strRFID_NO+"-站点"+strStationNO+"的第二个或第三或第四个程序号----"+strtemp+"");
                    }
                    //给数组IR_CAN_RFID_Arr[i]变量附值,包括各绑定RFID受控的IO 站点的设定值
                        //设定值包括零件指示、拧紧枪联锁

                        Query_RFID_CAN_SetInfor_IO_ByPrg(IR_CAN_RFID_Arr[i].EGType_Flow,strRFID_NO,strIP,IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].MINPrg,strtemp_stationNO);

                        //首先单独发送一个程序号   作业联锁
                       if (strtemp_stationNO.length()!=0 )
                          {
                           m= 0;
                           m=IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Reset.toInt();//IO模块的CAN站号
                           if (m==0 )
                             {
                               if(strRFID_NO=="03")
                              {
                               this->AddState("IO模块的针脚未分配");
                              }
                               else if(strRFID_NO=="04")
                               {
                                this->AddState2("IO模块的针脚未分配");
                               }

                             }
                           else
                             {

                                   Send_StationNO_PrgNO_TO_IO_GunPrgNO_Continue(i,j,2,strtemp_stationNO,IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);

                                   IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State[16-m]='1';

                                   //其次同时发送程序号和复位信号

                                   Send_StationNO_PrgNO_TO_IO_GunPrgNO_Continue(i,j,2,strtemp_stationNO,IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);
                                   IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State[16-m]='0';


                                   //发送程序号

                                   Send_StationNO_PrgNO_TO_IO_GunPrgNO_Continue(i,j,2,strtemp_stationNO,IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Pin_Output_State);
                             }

                         }//IR_CAN_RFID_Arr[i].IR_Belong_IO_Arr[j].IR_IO_Station_NO不为空判断结束

                 }
                else{
                    if (strRFID_NO=="03")
                    {

                       this->AddState("rfid--"+strRFID_NO+"-站点"+strStationNO+"无第二个或更多程序");
                    }
                    else if (strRFID_NO=="04")
                    {

                       this->AddState2("rfid--"+strRFID_NO+"-站点"+strStationNO+"无第二个或更多程序");
                    }


                }


      }

}

//Strlength=16表示转换为16位长度的2进制
QString MainWindow::Check_Bit_Signal(const QString Str_Hex,const qint32 Strlength)
{
  QString StrTemp,ResultTemp,temptest;
  qint32 i,num_bit;

     bool ok;
     qint32 hex_test=0;
     hex_test=Str_Hex.toInt(&ok,16);//将16进制字符转换为10进制数值

     StrTemp=Str_IntToBin(hex_test,Strlength);//将数据转换为Strlength位长度的二进制字符串

     ResultTemp="00";
     for(i=0;i<Strlength;i++)
       {
           if (StrTemp.mid(i,1)=="1")
              {

                  num_bit=Strlength-i;
                  if (num_bit<10)
                  {//4444aa----4444aa----00000 00000 000100

                      ResultTemp="0"+ResultTemp.setNum(num_bit,10);
                  }
                  else
                  {
                      ResultTemp=ResultTemp.setNum(num_bit,10);
                  }
                  break;
              }
       }
     //this->AddState("4444kk----"+ResultTemp+"");
     return ResultTemp;
}



//将一个十进制整型转换成二进制值 参数说明：Int:被转换的整型值
//Size:转换后的宽度：4位 8位 或更大
QString MainWindow::Str_IntToBin(const qint32 Int_HEX,const qint32 Strlength)
{
  QString str_bin="";
  qint32 length,i;
  str_bin=str_bin.setNum(Int_HEX,2);//将数值转换为2进制的字符串
  length=str_bin.length();

       for(i=0;i<Strlength-length;i++)
       {
         str_bin="0"+str_bin;
       }
  return str_bin;
}




//复位放行气缸
void MainWindow::EnableSTOPGO_Reset(qint32 Rfid_NO)
{
  qint32 i=0;
  bool ok=false;
  QString str_CMD_Value="",str_IR_Station_NO="";

          //1、根据RFID编号找到对应阻挡器、复位检测开关连接用的IO模块的站号

          str_IR_Station_NO=IR_CAN_RFID_Arr[Rfid_NO].IR_Belong_STOPGO.STOPGO_Station_NO ;

          str_CMD_Value="0000000000000000";
          IR_CAN_RFID_Arr[Rfid_NO].IR_Belong_STOPGO.STOPGO_Eanbled=false;

          //2、使能阻挡气缸的引脚为1，强制输出

          i=(IR_CAN_RFID_Arr[Rfid_NO].IR_Belong_STOPGO.STOPGO_Pin_Output_PinNo).toInt();
          str_CMD_Value[17-i]='0';
          //先将2进制字符串转为10进制数值,然后再转为16进制字符串
          str_CMD_Value=str_CMD_Value.setNum(str_CMD_Value.toInt(&ok,2),16);

          if (Rfid_NO==3)
            {
               str_IR_Station_NO="252";
               str_CMD_Value="0";
            }
          else if(Rfid_NO==4)
            {
               str_IR_Station_NO="254";
               str_CMD_Value="0";
            }
          SendCMD_TO_Station_By_CANCOM(str_IR_Station_NO,str_CMD_Value);
          IR_CAN_RFID_Arr[Rfid_NO].IR_Belong_STOPGO.STOPGO_State="2";
          AddState("IO_IO:&&&03复位阻挡器");
}

void MainWindow::EnableSTOPGO_Reset_BYStation_temp(const QString strStation_NO)
{
    bool ok=false;
   QString  str_CMD_Value ;


          //1、根据RFID编号找到对应阻挡器、复位检测开关连接用的IO模块的站号

          str_CMD_Value="0000000000000000";
          //2、使能阻挡气缸的引脚为1，强制输出

          str_CMD_Value=str_CMD_Value.setNum(str_CMD_Value.toInt(&ok,2),16);
          SendCMD_TO_Station_By_CANCOM(strStation_NO,str_CMD_Value);
}


//当系统接受到复位开关信号时的处理事件 ,实际操作时应该是将卡远离卡头，然后碰复位开关
void MainWindow::IO_ResetCheck_3F_COM(const qint32 StrRFID_NO)
{
  QString  strRFID_NO_temp="";


         if (StrRFID_NO<10)
             {
                 strRFID_NO_temp="0"+strRFID_NO_temp.setNum(StrRFID_NO,10);
             }
         else
             {
                 strRFID_NO_temp=strRFID_NO_temp.setNum(StrRFID_NO,10);
             }

        //更新
        UPdate_IR_RFID_READDATALIST_ByRFIDNO_IP_Reset(strRFID_NO_temp,IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD);
       /*
        //复位拧紧枪
        if  StrRFID_NO<10 then
          begin
             SendGunPrgNO_to_IOStation_Reset(IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD,StrRFID_NO,'0'+inttostr(StrRFID_NO),IR_CAN_RFID_Arr[StrRFID_NO].RFID_CAN_MaxMAPStationNO);
          end
        else
          begin
             SendGunPrgNO_to_IOStation_Reset(IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD,StrRFID_NO,inttostr(StrRFID_NO),IR_CAN_RFID_Arr[StrRFID_NO].RFID_CAN_MaxMAPStationNO);
          end;

        */
        //删除 20140717 屏蔽
        //DataModule_3F.DeleteIR_RUN_Record_ByRFIDNO_IP_Reset(strRFID_NO_temp,IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD);

        //更新[3F_IR_RUN]表中的记录为完成  20140717 起用
        //前工序
        if (strRFID_NO_temp==strStart_stationNO)
           {
              //Update_IR_RUN_RE_FIN_IO_ByMAN_or_ByResetInput(IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD,strRFID_NO_temp,ADOQ_First);
              Update_IR_RUN_RE_FIN_IO_ByMAN_or_ByResetInput(IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD,strRFID_NO_temp);
              Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_first,strStart_stationNO,IR_Infor.IP_ADD);


              AddState("该工序作业数据更新完成");

              //如果连接的RFID是OMRON，比如缸体分装线
              if (IR_RFID.IR_RFID_Type=="2")
                {
                    //允许重新读ID
                     IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadData_Only="00";
                     AddState("该工序重读ID许可");
                     //解除读卡状态
                     if (IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadSta=="01")
                     {
                          IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadSta="00";

                      }
                             AddState("该工序解除读卡");

               }
                /*
                if (IR_RFID.IR_RFID_Type=="1")
                begin
                    IR_CAN_RFID_Arr[StrRFID_NO].IR_Belong_STOPGO.STOPGO_State:='1';
                    frm_IR_Run.AddState(IR_CAN_RFID_Arr[StrRFID_NO].IR_Belong_STOPGO.STOPGO_State);
                end;
                */
          }
        //后工序
        else if (strRFID_NO_temp==strEnd_stationNO)
        {
              Update_IR_RUN_RE_FIN_IO_ByMAN_or_ByResetInput(IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD,strRFID_NO_temp);
              //Update_IR_RUN_RE_FIN_IO_ByMAN_or_ByResetInput(IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD,strRFID_NO_temp,ADOQ_Second);
              Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_second,strEnd_stationNO,IR_Infor.IP_ADD);
              AddState2("该工序作业数据更新完成");


             //如果连接的RFID是OMRON，比如缸体分装线
              //if IR_RFID.IR_RFID_Type="2"
                {
                    //允许重新读ID
                     IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadData_Only="00";
                     AddState2("该工序重读ID许可");
                    //解除读卡状态
                     if (IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadSta=="01")
                     {
                           IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadSta="00";
                      }
                     AddState2("该工序解除读卡");
                }
                /*
              if IR_RFID.IR_RFID_Type='1' then
                begin

                    IR_CAN_RFID_Arr[StrRFID_NO].IR_Belong_STOPGO.STOPGO_State:='1';
                    frm_IR_Run.AddState2(IR_CAN_RFID_Arr[StrRFID_NO].IR_Belong_STOPGO.STOPGO_State);
                end;
                */
          }
}



//更新数据表[3F_IR_RFID_READDATALIST] 的字段ALL_FINISHED
void MainWindow::UPdate_IR_RFID_READDATALIST_ByRFIDNO_IP_Reset(const QString StrRFID_NO,const QString StrIP)
{
  //QSqlQueryModel * model =  new QSqlQueryModel();
  //model->setQuery("select * from student ");
  //model->setHeaderData(0,Qt::Horizontal,tr("id"));

   QSqlQuery * query = new QSqlQuery();
   QString strSQL;
   strSQL="Update 3F_IR_RFID_READDATALIST SET  ALL_FINISHED=1 where RFID_NO='"+StrRFID_NO+"' AND IP_ADD='"+StrIP+"'";
   query->exec(strSQL);

   //query->exec("insert into student(name) values('sg3f01') ");
}


//人工强制放行或复位信号输入时更新[3F_IR_RUN]数据表
//void MainWindow::Update_IR_RUN_RE_FIN_IO_ByMAN_or_ByResetInput(const QString StrIP,const QString strRFID_NO,const QString ADOQ1:TADOQuery);
void MainWindow::Update_IR_RUN_RE_FIN_IO_ByMAN_or_ByResetInput(const QString StrIP,const QString strRFID_NO)
{

    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount,j;

    strSQL="select count(ID) from 3F_IR_RUN where IR_IP_ADD='"+StrIP+"' and IR_RFID_No='"+strRFID_NO+"' and IR_Finished='"+strtemp_NG+"'";

    ADOQ.setQuery(strSQL);
    //strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    //strCount = ADOQ.rowCount();
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //循环将各条记录的各字段内容赋值给各数组变量
    //for(j=0;j<strCount;j++)
    if (strCount>0)
    {
        if (IR_Infor.ControlforLine_Manu_Auto=="02") //02自动线
         {
           strSQL="Update 3F_IR_RUN set IR_Finished='"+strtemp_OK+"' where IR_IP_ADD='"+StrIP+"'  and IR_RFID_No='"+strRFID_NO+"'   and IR_Finished='"+strtemp_NG+"'";
         }
        else if (IR_Infor.ControlforLine_Manu_Auto=="01") //01手动线
         {
           strSQL="Update 3F_IR_RUN set IR_Finished='"+strtemp_OK+"' where IR_IP_ADD='"+StrIP+"'  and IR_RFID_No='"+strRFID_NO+"'   and IR_Finished='"+strtemp_NG+"'";
          }
//-----------------------------刷新屏幕的记录表状态------------------
        QSqlQueryModel * model =  new QSqlQueryModel();
        model->setQuery(strSQL);
        model->setHeaderData(0,Qt::Horizontal,tr("id"));
        model->setHeaderData(1,Qt::Horizontal,tr("name"));

        ui->com3_tableView->setModel(model); //view使用界面定义的控件
        ui->com3_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->com3_tableView->setBackgroundRole(QPalette::Dark);
    }
}

//显示内容
void MainWindow::Display_IR_Partlist_DOUBLE(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP)
{
  QString strSQL;
  QString strtempEGShortNo;


 strtempEGShortNo= Str_EGShortNo.mid(0,3);



 if (Str_RFID_No==strStart_stationNO)
 {

     ui->eqmid3_lineEdit->setText(Str_EGShortNo);//显示在机型文本框
     //QSqlQueryModel * todolist_model =  new QSqlQueryModel();
     QSg3fQueryModel  * todolist_model =  new QSg3fQueryModel();
     //strSQL="select * from 3F_IR_RUN Where (IR_IP_ADD='"+StrIP+"') and (IR_RFID_No='"+Str_RFID_No+"') order by IR_Prg_NO asc";
     strSQL="select ir_ip_add,"
                          " ir_rfid_no,"
                          " ir_egtype_no,"
                          " ir_station_no,"
                          " ir_partlist_name,"
                          " ir_finished "
                         " from 3f_ir_run "
                          " Where (IR_IP_ADD='"+StrIP+"') and (IR_RFID_No='"+Str_RFID_No+"') order by IR_Prg_NO asc";

     todolist_model->setQuery(strSQL);
     todolist_model->setHeaderData(0,Qt::Horizontal,tr("ir_ip_add"));
     todolist_model->setHeaderData(1,Qt::Horizontal,tr("ir_rfid_no"));
     todolist_model->setHeaderData(2,Qt::Horizontal,tr("ir_egtype_no"));
     todolist_model->setHeaderData(3,Qt::Horizontal,tr("ir_station_no"));
     todolist_model->setHeaderData(4,Qt::Horizontal,tr("ir_partlist_name"));
     todolist_model->setHeaderData(5,Qt::Horizontal,tr("ir_finished"));

     ui->com3_tableView->setModel(todolist_model); //view使用界面定义的控件
     //ui->com3_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
     //ui->com3_tableView->setBackgroundRole(QPalette::Dark);


  }
   else if (Str_RFID_No==strEnd_stationNO)
  {
    ui->eqmid4_lineEdit->setText(Str_EGShortNo);//显示在机型文本框
   // QSqlQueryModel * todolist_model =  new QSqlQueryModel();
    QSg3fQueryModel  * todolist_model =  new QSg3fQueryModel();
    strSQL="select ir_ip_add,"
                         " ir_rfid_no,"
                         " ir_egtype_no,"
                         " ir_station_no,"
                         " ir_partlist_name,"
                         " ir_finished "
                         " from 3f_ir_run "
                         " Where (IR_IP_ADD='"+StrIP+"') and (IR_RFID_No='"+Str_RFID_No+"') order by IR_Prg_NO asc";

    todolist_model->setQuery(strSQL);
    todolist_model->setHeaderData(0,Qt::Horizontal,tr("ir_ip_add"));
    todolist_model->setHeaderData(1,Qt::Horizontal,tr("ir_rfid_no"));
    todolist_model->setHeaderData(2,Qt::Horizontal,tr("ir_egtype_no"));
    todolist_model->setHeaderData(3,Qt::Horizontal,tr("ir_station_no"));
    todolist_model->setHeaderData(4,Qt::Horizontal,tr("ir_partlist_name"));
    todolist_model->setHeaderData(5,Qt::Horizontal,tr("ir_finished"));

     ui->com4_tableView->setModel(todolist_model); //view使用界面定义的控件
     //ui->com4_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
     //ui->com4_tableView->setBackgroundRole(QPalette::Dark);

  }
}

//当系统接受到复位开关信号时的处理事件 ,实际操作时应该是将卡远离卡头，然后碰复位开关
void MainWindow::IO_ResetCheck_3F_COM_ForAutoLine_New(qint32 StrRFID_NO)
{
  QString strRFID_NO_temp;
  if (StrRFID_NO<10)
    {
       strRFID_NO_temp="0"+strRFID_NO_temp.setNum(StrRFID_NO,10);//数字转换为字符串
    }
  else
    {
       strRFID_NO_temp=strRFID_NO_temp.setNum(StrRFID_NO,10);
    }


                   //更新
                   //DataModule_3F.UPdate_IR_RFID_READDATALIST_ByRFIDNO_IP_Reset_ForAutoLine(strRFID_NO_temp,IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD);
                  /*
                   {
                   //复位拧紧枪
                   if  (StrRFID_NO<10) then
                     begin
                        SendGunPrgNO_to_IOStation_Reset(IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD,StrRFID_NO,'0'+inttostr(StrRFID_NO),IR_CAN_RFID_Arr[StrRFID_NO].RFID_CAN_MaxMAPStationNO);
                     end
                   else
                     begin
                        SendGunPrgNO_to_IOStation_Reset(IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD,StrRFID_NO,inttostr(StrRFID_NO),IR_CAN_RFID_Arr[StrRFID_NO].RFID_CAN_MaxMAPStationNO);
                     end;

                   }
                   */
                   //删除 20140717 屏蔽
                   //DataModule_3F.DeleteIR_RUN_Record_ByRFIDNO_IP_Reset(strRFID_NO_temp,IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD);

                   //更新[3F_IR_RUN]表中的记录为完成  20140717 起用
                   //前工序
                   if (strRFID_NO_temp==strStart_stationNO)
                   {
                         Update_IR_RUN_RE_FIN_IO_ByMAN_or_ByResetInput(IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD,strRFID_NO_temp);
                         Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_first,strStart_stationNO,IR_Infor.IP_ADD);

                         AddState("该工序作业数据更新完成");
                         //允许重新读ID
                         IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadData_Only="00";
                         AddState("该工序重读ID许可");
                        //解除读卡状态
                         if (IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadSta=="01")
                          {
                            IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadSta="00";
                          }
                         AddState("该工序解除读卡");
                      }
                   //后工序
                   else if (strRFID_NO_temp==strEnd_stationNO)
                      {
                         Update_IR_RUN_RE_FIN_IO_ByMAN_or_ByResetInput(IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD,strRFID_NO_temp);

                         Display_IR_Partlist_DOUBLE(IR_Infor.Current_EGtypeFlow_second,strEnd_stationNO,IR_Infor.IP_ADD);
                         AddState2("该工序作业数据更新完成");
                        //允许重新读ID
                         IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadData_Only="00";
                         AddState2("该工序重读ID许可");
                        //解除读卡状态
                         if (IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadSta=="01")
                           {
                             IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadSta="00";
                           }
                         AddState("该工序解除读卡");
                      }

}



//CAN串口使用
//更新[3F_IR_RUN]数据表
bool MainWindow::Query_IR_RUN_FIN_IO_BystationNO(const QString StrIP,const QString strstationNO)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;
    strSQL="select count(ID) from 3F_IR_RUN where IR_IP_ADD='"+StrIP+"' and IR_Station_NO='"+strstationNO+"'  and IR_Finished='"+strtemp_NG+"'";

    ADOQ.setQuery(strSQL);
    //取得一共有多少条件记录
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = ADOQ.rowCount();
    if (strCount>0)
    {
      return true;
    }else
    {
      return false;
    }


}


//CAN串口使用
//根据本系统配置的IP地址、绑定RFID的站号，查询对应的作业3F_CAN_RFID_STATIONNO_SET是否有相关记录
bool MainWindow::QueryIR_RFIDSET_LIST_BE(const QString StrIP,const QString strstationNO)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;

    strSQL="select count(ID) from 3F_CAN_RFID_STATIONNO_SET Where RFID_Station_NO='"+strstationNO+"' and RFID_IP_ADD='"+StrIP+"'";

    ADOQ.setQuery(strSQL);
    //strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = ADOQ.rowCount();
    if (strCount>0)
    {
      return true;
    }else
    {
      return false;
    }


}


//读取数据库3F_IR_INPUT中配置绑定的RFID（CAN总线）受控的站点及对应的设备信息
QString MainWindow::Query_RFID_NO(const QString Str_StationNO,const QString StrIP)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;
    QString Result="00";
    strSQL="select * from 3F_IR_RUN where  IR_IP_ADD='"+StrIP+"' AND IR_Station_NO='"+Str_StationNO+"'";

    ADOQ.setQuery(strSQL);
    //strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("IR_RFID_No")).toInt();
    //取得一共有多少条件记录
    strCount = ADOQ.rowCount();
    if (strCount>0)
    {
      Result=(ADOQ.record(0).value("IR_RFID_No")).toString();
    }

    return Result;
}

//初始化连接到COM3,前工序
void MainWindow::initComm3()
{

    QString portName=this->strCom3;
    myCom3 = new QextSerialPort(portName);
    //关联读串口Slot函数
    connect(myCom3, SIGNAL(readyRead()), this, SLOT(readMyCom3()));

    if(IR_RFID.IR_RFID_Type=="2")
      {

    //设置波特率
     myCom3->setBaudRate((BaudRateType)this->strBaudRate3.toInt());
     //设置数据位
     //myCom3->setDataBits((DataBitsType)8);
     myCom3->setDataBits((DataBitsType)7);
     //设置校验位
     //myCom3->setParity(PAR_NONE);
     myCom3->setParity(PAR_EVEN);

     //设置停止位
     myCom3->setStopBits(STOP_1);
     //设置数据流控制
     myCom3->setFlowControl(FLOW_OFF);

     //设置延时
     myCom3->setTimeout(TIME_OUT);
     myCom3->open(QIODevice::ReadWrite);
      }
    else if(IR_RFID.IR_RFID_Type=="1")
    {
        //设置波特率
         myCom3->setBaudRate((BaudRateType)this->strBaudRate3.toInt());
         //设置数据位
         myCom3->setDataBits((DataBitsType)8);
         //设置校验位
         myCom3->setParity(PAR_NONE);

         //设置停止位
         myCom3->setStopBits(STOP_1);
         //设置数据流控制
         myCom3->setFlowControl(FLOW_OFF);

         //设置延时
         myCom3->setTimeout(TIME_OUT);
         myCom3->open(QIODevice::ReadWrite);

    }







     if(myCom3->open(QIODevice::ReadWrite)){
            this->AddState("com3<<=:已成功打开串口");
            this->AddState2("com3<<=:已成功打开串口");
            //QMessageBox::information(this, tr("打开成功"), tr("已成功打开串口") + portName, QMessageBox::Ok);

     }else{
            QMessageBox::critical(this, tr("打开失败"), tr("未能打开串口 ") + portName + tr("\n该串口设备不存在或已被占用"), QMessageBox::Ok);
            return;
     }

}



//读取连接到COM3的RFID,前工序
void MainWindow::readMyCom3()
{
    QString RFID_NO="03";
    QString tmpStr_first,tmpStr_end;
//读取串口内容
//16进制形式显示
    QByteArray temp = myCom3->readAll();
    QString buf;
  // if(!temp.isEmpty())
   {


    for(int i = 0; i < temp.count(); i++){
        QString s;
        s.sprintf("%02x", (unsigned char)temp.at(i));
        buf += s.toUpper();
    }



    if  (IR_RFID.IR_RFID_Type=="2")//rfid为OMRON
    {

//this->AddState("com3<<=:"+buf+"");
    tmpStr_first=buf.mid(0, 2);
    tmpStr_end=buf.mid( buf.length()-2, 2);//获取帧尾，如果为55则处理，否则继续累加


    if ((tmpStr_first=="52")&&(tmpStr_end=="0D"))//如果接收到的是一帧的结束符号，则直接处理
    {
          frame_content_com3=frame_content_com3+buf;//累加，一直等到55

          recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]=frame_content_com3;//用于处理接收到的内容
         // this->AddState("com3完整<<=:"+ChangeAreaHEXToStr(frame_content_com3)+"");
          CheckCMD_CAN_RFID_3F(RFID_NO,recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]);

          frame_content_com3="";
          frame_first_com3="";
    }
    else if((tmpStr_first=="52")&&(frame_first_com3==""))
    {
       frame_content_com3=buf;
       frame_first_com3=buf;
    }

    else if ((frame_first_com3.mid(0, 2)=="52")&&(tmpStr_end!="0D"))
    {
       frame_content_com3=frame_content_com3+buf;
    }
    else if ((frame_first_com3.mid(0, 2)=="52")&&(tmpStr_end=="0D"))//如果接收到的是一帧的结束符号，则直接处理
    {
          frame_content_com3=frame_content_com3+buf;//累加，一直等到55

          recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]=frame_content_com3;//用于处理接收到的内容
         // this->AddState("com3完整<<=:"+ChangeAreaHEXToStr(frame_content_com3)+"");
          CheckCMD_CAN_RFID_3F(RFID_NO,recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]);

          frame_content_com3="";
          frame_first_com3="";
    }
    else
    {
          frame_content_com3="";
          temp ="";
    }

   }//rfid为omron
   else if  (IR_RFID.IR_RFID_Type=="1")//rfid为sigo
    {

       // this->AddState("com3<<=:"+buf+"");
        tmpStr_first=buf.mid(0, 2);
        tmpStr_end=buf.mid( buf.length()-2, 2);//获取帧尾，如果为55则处理，否则继续累加


        if ((tmpStr_first=="43")&&(tmpStr_end=="0D"))//如果接收到的是一帧的结束符号，则直接处理
        {
              frame_content_com3=frame_content_com3+buf;//累加，一直等到55

              recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]=frame_content_com3;//用于处理接收到的内容
              //this->AddState("com3完整<<=:"+ChangeAreaHEXToStr(frame_content_com3)+"");
              //this->AddState("com3完整<<=:"+frame_content_com3+"");
              CheckCMD_CAN_RFID_3F(RFID_NO,recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]);

              frame_content_com3="";
              frame_first_com3="";
        }
        else if((tmpStr_first=="43")&&(frame_first_com3==""))
        {
           frame_content_com3=buf;
           frame_first_com3=buf;
        }

        else if ((frame_first_com3.mid(0, 2)=="43")&&(tmpStr_end!="0D"))
        {
           frame_content_com3=frame_content_com3+buf;
        }
        else if ((frame_first_com3.mid(0, 2)=="43")&&(tmpStr_end=="0D"))//如果接收到的是一帧的结束符号，则直接处理
        {
              frame_content_com3=frame_content_com3+buf;//累加，一直等到55

              recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]=frame_content_com3;//用于处理接收到的内容
              //this->AddState("com3完整<<=:"+ChangeAreaHEXToStr(frame_content_com3)+"");
              //this->AddState("com3完整<<=:"+frame_content_com3+"");
              CheckCMD_CAN_RFID_3F(RFID_NO,recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]);

              frame_content_com3="";
              frame_first_com3="";
        }
        else
        {
              frame_content_com3="";
              temp ="";
        }
    }
  }
}


//初始化连接到COM4,后工序
void MainWindow::initComm4()
{
    QString portName=this->strCom4;
    myCom4 = new QextSerialPort(portName);
    //关联读串口Slot函数
    connect(myCom4, SIGNAL(readyRead()), this, SLOT(readMyCom4()));


    if(IR_RFID.IR_RFID_Type=="2")
        {
    //设置波特率
     myCom4->setBaudRate((BaudRateType)this->strBaudRate4.toInt());
     //设置数据位
    // myCom4->setDataBits((DataBitsType)8);
     myCom4->setDataBits((DataBitsType)7);
     //设置校验位
     //myCom4->setParity(PAR_NONE);
          myCom4->setParity(PAR_EVEN);
     //设置停止位
     myCom4->setStopBits(STOP_1);
     //设置数据流控制
     myCom4->setFlowControl(FLOW_OFF);

     //设置延时
     myCom4->setTimeout(TIME_OUT);
     myCom4->open(QIODevice::ReadWrite);
   }
   else if(IR_RFID.IR_RFID_Type=="1")
    {
        //设置波特率
         myCom4->setBaudRate((BaudRateType)this->strBaudRate4.toInt());
         //设置数据位
         myCom4->setDataBits((DataBitsType)8);
         //设置校验位
         myCom4->setParity(PAR_NONE);
         //设置停止位
         myCom4->setStopBits(STOP_1);
         //设置数据流控制
         myCom4->setFlowControl(FLOW_OFF);

         //设置延时
         myCom4->setTimeout(TIME_OUT);
         myCom4->open(QIODevice::ReadWrite);
    }



     if(myCom4->open(QIODevice::ReadWrite)){
            this->AddState("com4<<=:已成功打开串口");
            this->AddState2("com4<<=:已成功打开串口");
            //QMessageBox::information(this, tr("打开成功"), tr("已成功打开串口") + portName, QMessageBox::Ok);

     }else{
            QMessageBox::critical(this, tr("打开失败"), tr("未能打开串口 ") + portName + tr("\n该串口设备不存在或已被占用"), QMessageBox::Ok);
            return;
     }

}



//读取连接到COM4的RFID,后工序
void MainWindow::readMyCom4()
{
    QString RFID_NO="04";
    QString tmpStr_first,tmpStr_end;
//读取串口内容
//16进制形式显示
    QByteArray temp = myCom4->readAll();
    QString buf;
  // if(!temp.isEmpty())
   {


    for(int i = 0; i < temp.count(); i++){
        QString s;
        s.sprintf("%02x", (unsigned char)temp.at(i));
        buf += s.toUpper();
    }

  //this->AddState2("com4<<=:"+buf+"");

    if  (IR_RFID.IR_RFID_Type=="2")//rfid为OMRON
    {
   //this->AddState2("com4<<=:"+buf+"");
    tmpStr_first=buf.mid(0, 2);
    tmpStr_end=buf.mid( buf.length()-2, 2);//获取帧尾，如果为55则处理，否则继续累加


    if ((tmpStr_first=="52")&&(tmpStr_end=="0D"))//如果接收到的是一帧的结束符号，则直接处理
    {
          frame_content_com4=frame_content_com4+buf;//累加，一直等到55

          recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]=frame_content_com4;//用于处理接收到的内容
        //  this->AddState2("com4完整<<=:"+ChangeAreaHEXToStr(frame_content_com4)+"");
          CheckCMD_CAN_RFID_3F(RFID_NO,recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]);

          frame_content_com4="";
          frame_first_com4="";
    }

    else if((tmpStr_first=="52")&&(frame_first_com4==""))
    {
       frame_content_com4=buf;
       frame_first_com4=buf;
    }

    else if ((frame_first_com4.mid(0, 2)=="52")&&(tmpStr_end!="0D"))
    {
       frame_content_com4=frame_content_com4+buf;
    }
    else if ((frame_first_com4.mid(0, 2)=="52")&&(tmpStr_end=="0D"))//如果接收到的是一帧的结束符号，则直接处理
    {
          frame_content_com4=frame_content_com4+buf;//累加，一直等到55

          recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]=frame_content_com4;//用于处理接收到的内容
       //   this->AddState2("com4完整<<=:"+ChangeAreaHEXToStr(frame_content_com4)+"");
          CheckCMD_CAN_RFID_3F(RFID_NO,recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]);

          frame_content_com4="";
          frame_first_com4="";
    }
    else
    {
          frame_content_com4="";
          temp ="";
    }

    }//rfid为omron
    else if  (IR_RFID.IR_RFID_Type=="1")//rfid为sigo
     {

      //this->AddState2("com4<<=:"+buf+"");
        tmpStr_first=buf.mid(0, 2);
        tmpStr_end=buf.mid( buf.length()-2, 2);//获取帧尾，如果为55则处理，否则继续累加


        if ((tmpStr_first=="43")&&(tmpStr_end=="0D"))//如果接收到的是一帧的结束符号，则直接处理
        {
              frame_content_com4=frame_content_com4+buf;//累加，一直等到55

              recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]=frame_content_com4;//用于处理接收到的内容
              //this->AddState2("com4完整<<=:"+ChangeAreaHEXToStr(frame_content_com4)+"");
             // this->AddState2("com4完整<<=:"+frame_content_com4+"");
              CheckCMD_CAN_RFID_3F(RFID_NO,recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]);

              frame_content_com4="";
              frame_first_com4="";
        }

        else if((tmpStr_first=="43")&&(frame_first_com4==""))
        {
           frame_content_com4=buf;
           frame_first_com4=buf;
        }

        else if ((frame_first_com4.mid(0, 2)=="43")&&(tmpStr_end!="0D"))
        {
           frame_content_com4=frame_content_com4+buf;
        }
        else if ((frame_first_com4.mid(0, 2)=="43")&&(tmpStr_end=="0D"))//如果接收到的是一帧的结束符号，则直接处理
        {
              frame_content_com4=frame_content_com4+buf;//累加，一直等到55

              recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]=frame_content_com4;//用于处理接收到的内容
              //this->AddState2("com4完整<<=:"+ChangeAreaHEXToStr(frame_content_com4)+"");
             // this->AddState2("com4完整<<=:"+frame_content_com4+"");
              CheckCMD_CAN_RFID_3F(RFID_NO,recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]);

              frame_content_com4="";
              frame_first_com4="";
        }
        else
        {
              frame_content_com4="";
              temp ="";
        }
     }
  }
}


//处理从串口3和串口4读取的信息
void MainWindow::CheckCMD_CAN_RFID_3F(const QString StrstationNO,const QString StrtempINput)
{
    qint32 k;
  //  qint32 Fram_Head_Count;
    QString tmpStr,Fram_Head;

    QString content1,content2;

  //1-首先截取接收的信息
   tmpStr= StrtempINput;
   k=StrstationNO.toInt();

   //2-判断是否已经成功读取一次
             if (StrstationNO=="03")
             {

                //frm_IR_Run.Panel15.color=clyellow;

                ReadIDValue_03_new=tmpStr;
                if (ReadIDValue_03_old=="00" )//软件启动时
                   {
                      ReadIDValue_03_old=tmpStr;
                      this->AddState("com3完整<<=:"+tmpStr+"");
                   }
                else
                   {
                     if (ReadIDValue_03_new!=ReadIDValue_03_old)
                         {
                             this->AddState("com3完整<<=:"+tmpStr+"");
                             ReadIDValue_03_old= ReadIDValue_03_new;
                             PC_ANDON_WriteFIN="00";
                             //frm_IR_Run.Edit2.Text="新循环开始";
                             tmpStr=ReadIDValue_03_old;

                         }
                     else
                         {
                             IR_CAN_RFID_Arr[k].IR_RFID_ReadSta="00";
                             return;

                         }
                   }
             }
           else if (StrstationNO=="04")
             {

               //frm_IR_Run.Panel3.color=clyellow;

                ReadIDValue_04_new=tmpStr;
                if (ReadIDValue_04_old=="00" )//软件启动时
                   {
                      ReadIDValue_04_old=tmpStr;
                      this->AddState2("com4完整<<=:"+tmpStr+"");
                   }
                else
                   {
                     if (ReadIDValue_04_new!=ReadIDValue_04_old )
                         {
                            this->AddState2("com4完整<<=:"+tmpStr+"");
                            // frm_IR_Run.Edit3.Text="新循环开始";
                             PC_ANDON_WriteFIN_Second="00";
                             ReadIDValue_04_old= ReadIDValue_04_new;
                             tmpStr=ReadIDValue_04_old;
                         }
                     else
                         {
                             IR_CAN_RFID_Arr[k].IR_RFID_ReadSta="00";
                             return;
                         }
                   }
             }

             //3-处理数据相关事件
            if  (IR_RFID.IR_RFID_Type=="3")  //条码枪
              {
              //4132312043313030303031
              tmpStr=tmpStr.mid(1,6)+tmpStr.mid(9,tmpStr.length()-8);
              //41323143313030303031
              }
            else if  (IR_RFID.IR_RFID_Type=="1")  //3F的RFID
              {
                 Fram_Head="43"; //AA
                 content1 = tmpStr.mid(0, 2); //帧头AA
                 if (content1==Fram_Head)
                   {
                     if  ((tmpStr.mid(10, 2)=="41") ||  (tmpStr.mid(10, 2)=="43")||  (tmpStr.mid(10, 2)=="42"))
                       {
                         //在线程中处理
                         //tmpStr=StrtempINput.mid(12,6) +StrtempINput.mid(20,14);
                       }
                     else
                       {
                         if (k==3){
                         this->AddState("com3读取机型错误-0x41,0x42,0x43");
                         }
                         else if (k==4){
                         this->AddState2("com4读取机型错误-0x41,0x42,0x43");
                         }
                         return;
                       }
                   }//反馈指令判断结束
                 else
                   {
                       return;
                   }

              }
             else if (IR_RFID.IR_RFID_Type=="2")  //OMRON的RFID
              {
                  content2 = StrstationNO;  //反馈RFID编号

                  tmpStr= StrtempINput;
                  if (tmpStr.mid(tmpStr.length()-2, 2) =="0D")
                    {
                       //1、判断读卡成功RD00  ，对应16进制52443030
                       if ((tmpStr.mid(0, 2) == "52") &&(tmpStr.mid(2, 2) == "44"))
                       {
                            if ((tmpStr.mid(4, 2) == "30")&&(tmpStr.mid(6, 2) == "30"))  //1-读ID操作成功
                               {
                                  //在线程中处理
                                  tmpStr=StrtempINput.mid(12,6) +StrtempINput.mid(20,14);

                                 // exit;
                               }
                            else
                               {
                                 IR_CAN_RFID_Arr[k].IR_RFID_ReadSta="00";
                                 if (k==3){
                                 this->AddState("com3读取机型错误-0x52");
                                 }
                                 else if (k==4){
                                 this->AddState2("com4读取机型错误-0x52");
                                 }
                                 return;
                               }
                         }
                     //2、判断写卡成功，写成功的返回指令WT0010*
                      else if (tmpStr.mid(1, 2) =="WT")
                         {

                         }
                      //3、判断清卡、消报警成功，写成功的返回指令CF0010*
                      else if (tmpStr.mid(1, 2) =="CF")
                         {
                           if (tmpStr.mid(4, 1) =="0")
                               {
                                   ;
                               }
                            else if (tmpStr.mid(4, 1) != "0")
                               {
                                  // ICFunction.ClearRFIDError(MSCOmron_First); //------------------
                               }
                            else
                               {
                                  return;
                               }
                         }
                      else
                         {
                           IR_CAN_RFID_Arr[k].IR_RFID_ReadSta="00";
                           if (k==3){
                           this->AddState("com3读取机型错误-非0x52");
                           }
                           else if (k==4){
                           this->AddState2("com4读取机型错误-非0x52");
                           }

                           return;
                         }
                    }
                  else
                    {
                      IR_CAN_RFID_Arr[k].IR_RFID_ReadSta="00";
                      if (k==3){
                      this->AddState("com3读取机型错误");
                      }
                      else if (k==4){
                      this->AddState2("com4读取机型错误");
                      }
                      return;
                    }
               } //判断为OMRON的RFID结束


       //--------------------------------------------------------------

                       content2 = StrstationNO;  //反馈站号
                     //首先，将16进制站号转换为10进制
                       //k=content2.toInt();

                      //根据反馈的站号，及配置的IP地址，查询在数据表[3F_CAN_RFID_STATIONNO_SET]中是否存在
                      //1、如果不存在，则认为是IO模块，而不是RFID模块
                      if ((content2==strStart_stationNO)||(content2==strEnd_stationNO))
                       {
                          if (k==3){
                          this->AddState("com3读取机型成功-0x52");
                          }
                          else if (k==4){
                          this->AddState2("com4读取机型成功-0x52");
                          }
                           IR_CAN_RFID_Arr[k].Fram_Rfid_Data_Rev=tmpStr;
                           //AddState(tmpStr);
                          // Edit5.Text= IR_CAN_RFID_Arr[k].Fram_Rfid_Data_Rev;
                           IR_CAN_RFID_Arr[k].Fram_Rfid_Reset="03";
                           //调用函数RFID_Reset_3F_COM(i:integer) ,进行保存操作
                           RFID_Reset_3F_COM34(k);

                        }
                               //------------------------读卡处理结束--------

}



void MainWindow::RFID_Reset_3F_COM34(const qint32 i)
{
   QString strStation_NO;

               if (IR_CAN_RFID_Arr[i].RFID_Station_NO!="")
                  {


                          if (IR_CAN_RFID_Arr[i].Fram_Rfid_Reset=="03")
                           {
                              if (i==3){
                              this->AddState("com3数据保存处理");
                              }
                              else if (i==4){
                              this->AddState2("com4数据保存处理");
                              }
                               RFID_ResetCheck_3F_COM34(i);
                               IR_CAN_RFID_Arr[i].Fram_Rfid_Reset="00";
                           }

                          else if  (IR_CAN_RFID_Arr[i].Fram_Rfid_Reset=="04")
                           {
                                IR_CAN_RFID_Arr[i].Fram_Rfid_Reset="00";

                           }
                           else if  (IR_CAN_RFID_Arr[i].Fram_Rfid_Reset=="05")
                           {
                              IR_CAN_RFID_Arr[i].Fram_Rfid_Reset="00";

                           }
                  }

}


//判断是否需要保存读卡记录及是否需要更新记录的状态
void MainWindow::RFID_ResetCheck_3F_COM34(qint32 StrRFID_NO)
{
   QString strRFID_NO_temp;
   QString strData_Rev;
   strData_Rev=IR_CAN_RFID_Arr[StrRFID_NO].Fram_Rfid_Data_Rev;

                              //1、首先判断是否启用了复位信号，如果启用了则判断是否启用了复位清记录

                             if (IR_Infor.StrReset_Valid=="01")
                                {
                                 /*
                                 if (StrRFID_NO==3){
                                 this->AddState("com3数据");
                                 }
                                 else if (StrRFID_NO==4){
                                 this->AddState2("com4数据");
                                 }
                                 */
                                 // 1-1、未启用复位清记录=‘00’，此时必须等到作业完毕，且已经执行了放行阻挡气缸指令后
                                 //才能读取下一个托盘信息
                                 if (IR_Infor.StrReset_DelRecord=="00")
                                    {
                                     if (StrRFID_NO==3){
                                        this->AddState("ooooooo'"+IR_CAN_RFID_Arr[StrRFID_NO].IR_Belong_STOPGO.STOPGO_State+"'");
                                     }
                                     else if (StrRFID_NO==4){
                                        this->AddState2("ooooooo'"+IR_CAN_RFID_Arr[StrRFID_NO].IR_Belong_STOPGO.STOPGO_State+"'");
                                     }

                                    //1表示已经作业完成，3表示阻挡气缸已经执行放行动作，但还没有复位，复位后为2
                                     if (!(IR_CAN_RFID_Arr[StrRFID_NO].IR_Belong_STOPGO.STOPGO_State=="3"))
                                         {
                                              AddState("rrrrrrrrrr");
                                          //01表示已经有同组的作业未完成，00表示完成
                                             if (!(IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadData_Only=="01"))
                                                 {
                                                   AddState("rr33333333333");
                                                   Check_Station_Fram_COM34(StrRFID_NO,"03",strData_Rev.mid(0,36)); //确认属于哪一个站的哪一帧信息

                                                 }
                                          }
                                     }

                                 //1-2、如果启用了清零，则只要一有复位信号或重新读卡就删除[3F_IR_RUN]中对应的RFID的记录，并更新[3F_IR_RFID_READDATALIST]
                                 //中对应的RFID的记录为完成 ，而无需执行了放行阻挡气缸指令，就可以读取新的机型
                                  else if  (IR_Infor.StrReset_DelRecord=="01")
                                     {
                                            if (StrRFID_NO<10)
                                              {
                                                  strRFID_NO_temp="0"+strRFID_NO_temp.setNum(StrRFID_NO,10);
                                              }
                                            else
                                              {
                                                  strRFID_NO_temp=strRFID_NO_temp.setNum(StrRFID_NO,10);
                                              }

                                            if (IR_Infor.ControlforLine_Manu_Auto=="01") //01手动线
                                              {
                                                //更新
                                                  UPdate_IR_RFID_READDATALIST_ByRFIDNO_IP_COM34(strRFID_NO_temp,IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD);

                                                //删除
                                                  DeleteIR_RUN_Record_ByRFIDNO_IP_COM34(strRFID_NO_temp,IR_CAN_RFID_Arr[StrRFID_NO].RFID_IP_ADD);
                                             }
                                            //保存
                                            /*
                                            if (StrRFID_NO==3){
                                               this->AddState("aaaa'"+IR_CAN_RFID_Arr[StrRFID_NO].IR_Belong_STOPGO.STOPGO_State+"'");
                                            }
                                            else if (StrRFID_NO==4){
                                               this->AddState2("aaaa'"+IR_CAN_RFID_Arr[StrRFID_NO].IR_Belong_STOPGO.STOPGO_State+"'");
                                            }
                                            */
                                             Check_Station_Fram_COM34(StrRFID_NO,"03",strData_Rev.mid(0,36)); //确认属于哪一个站的哪一帧信息

                                     }
                                }
                              //2、如果没有启用复位信号,则不用等待任何信号，只要全部作业完成，就可自动读取新的记录
                              else if (IR_Infor.StrReset_Valid=="00")
                                {
                                     //01表示已经有同组的作业未完成，00表示完成
                                             if (!(IR_CAN_RFID_Arr[StrRFID_NO].IR_RFID_ReadData_Only=="01"))
                                                 {
                                                  AddState("rr555555555");
                                                   Check_Station_Fram_COM34(StrRFID_NO,"03",strData_Rev.mid(0,36)); //确认属于哪一个站的哪一帧信息

                                                 }


                                }

}



//以下为全部使用串口读取3F的RFID-------开始
//更新数据表[3F_IR_RFID_READDATALIST] 的字段ALL_FINISHED
void MainWindow::UPdate_IR_RFID_READDATALIST_ByRFIDNO_IP_COM34(const QString StrRFID_NO,const QString StrIP)
{

    QString strSQL;
    QSqlQuery * query = new QSqlQuery();
    strSQL="Update 3F_IR_RFID_READDATALIST SET  ALL_FINISHED=1 where RFID_NO='"+StrRFID_NO+"' AND IP_ADD='"+StrIP+"'";
    query->exec(strSQL);

}

void MainWindow::DeleteIR_RUN_Record_ByRFIDNO_IP_COM34(const QString StrRFID_NO,const QString StrIP)
{

                 QString strSQL;
                 QSqlQuery * query = new QSqlQuery();
                 strSQL="delete  from 3F_IR_RUN where IR_RFID_No='"+StrRFID_NO+"' AND IR_IP_ADD='"+StrIP+"'";
                 query->exec(strSQL);
}

void MainWindow::DeleteFin_IR_RUN_Record_ByRFIDNO_IP_COM34(const QString StrRFID_NO,const QString StrIP)
{

    QString strSQL;
    QSqlQuery * query = new QSqlQuery();
    strSQL="delete from 3F_IR_RUN where IR_RFID_No='"+StrRFID_NO+"' AND IR_IP_ADD='"+StrIP+"' AND IR_Finished='"+strtemp_OK+"'";

    query->exec(strSQL);


    if (StrRFID_NO.toInt()==3) {
      this->AddState("RFID03作业完成,更新R"+StrRFID_NO+"-"+StrIP+"");
    }
    else if (StrRFID_NO.toInt()==4) {
      this->AddState2("RFID04作业完成,更新R"+StrRFID_NO+"-"+StrIP+"");
    }

}



//确认属于哪一个站的哪一帧信息
void MainWindow::Check_Station_Fram_COM34(const qint32 Station_NO,const QString Fram_NO,const QString Fram_Content)
{
qint32 k;
QString Rfid_Data_Head, strValue;
QString strStation_NO;
QString strReadDATA;
//QByteArray arr;



      //因为从CAN总线反馈回来的CAN站号是16进制的，需要转换为10进制jj=strToint("$"+tmpStr);
      Rfid_Data_Head=CMD_COUMUNICATION.CMD_READ;//CMD_COUMUNICATION.CMD_READ="43"
      if(Fram_NO=="03") //第4帧
         {
                    k=Station_NO;
                    if (k<10)
                     {
                        strStation_NO="0"+strStation_NO.setNum(k,10);
                     }
                    else
                     {
                        strStation_NO=strStation_NO.setNum(k,10);
                     }

                      // Frm_IR_Main.AddState(""+INTTOSTR(LENGTH(IR_CAN_RFID_Arr))+"号站保存完毕记录");
                      if (IR_RFID.IR_RFID_Type=="3")  //条码枪
                         {
                           strReadDATA=Fram_Content;//截取机型和流水号
                           IR_CAN_RFID_Arr[k].IR_RFID_ReadDATA=Fram_Content;//截取机型和流水号
                            //判断读取的字符是否为A或C，如果是则进行转换和保存，如果不是则不保存、不转换
                            if ((strReadDATA.mid(0,2)=="41") ||(strReadDATA.mid(0,2)=="43"))
                                {
                                     //将串口获得的16进制机型和流水数据转换为ASII码机型和流水
                                     strValue=ChangeAreaHEXToStr(strReadDATA);

                                     //保存读取的记录，包括机型、流水号、时间、站号（与设备名称捆绑）
                                     //保存机型和流水给各设备,并发送接收数据的复位指令给各CAN站
                                     //CAN总线时
                                     // Frm_IR_Main.SaveRecord(strStation_NO,strValue,IR_CAN_RFID_Arr[Station_NO].RFID_IP_ADD,Fram_Content) ;
                                     //3FC_COM时 A195522222

                                     if (k==3){
                                        this->AddState("com3准备保存数据'"+strValue+"'");
                                     }
                                     else if (k==4){
                                        this->AddState2("com4准备保存数据'"+strValue+"'");

                                     }
                                      //SaveRecord_3F_COM(strStation_NO,IR_CAN_RFID_Arr[k].RFID_IP_ADD,strReadDATA) ;
                                      SaveRecord_3F_COM(strStation_NO,IR_CAN_RFID_Arr[k].RFID_IP_ADD,strValue) ;

                                }
                            else
                                {
                                if (k==3){
                                   this->AddState("com3扫描失败'"+strReadDATA.mid(2,2)+"'，请重读取");
                                }
                                else if (k==4){
                                   this->AddState2("com4扫描失败'"+strReadDATA.mid(2,2)+"'，请重读取");

                                }

                                     IR_CAN_RFID_Arr[k].IR_RFID_ReadSta="00";

                                }
                         }
                      else if (IR_RFID.IR_RFID_Type=="2")  //OMRON的RFID
                         {

                           strReadDATA=Fram_Content;//截取机型和流水号
                           IR_CAN_RFID_Arr[k].IR_RFID_ReadDATA=Fram_Content;//截取机型和流水号
                            //判断读取的字符是否为A或C，如果是则进行转换和保存，如果不是则不保存、不转换
                            if ((strReadDATA.mid(0,2)=="41")|| (strReadDATA.mid(0,2)=="43"))
                                {
                                     //将串口获得的16进制机型和流水数据转换为ASII码机型和流水
                                     strValue=ChangeAreaHEXToStr(strReadDATA);
                                     if (k==3){
                                        this->AddState("com3准备保存数据'"+strValue+"'");
                                     }
                                     else if (k==4){
                                        this->AddState2("com4准备保存数据'"+strValue+"'");

                                     }
                                     //保存读取的记录，包括机型、流水号、时间、站号（与设备名称捆绑）
                                     //保存机型和流水给各设备,并发送接收数据的复位指令给各CAN站
                                     //CAN总线时
                                     // SaveRecord(strStation_NO,strValue,IR_CAN_RFID_Arr[Station_NO].RFID_IP_ADD,strReadDATA) ;
                                     //3FC_COM时 A195522222
                                      //SaveRecord_3F_COM(strStation_NO,IR_CAN_RFID_Arr[k].RFID_IP_ADD,strReadDATA) ;
                                      SaveRecord_3F_COM(strStation_NO,IR_CAN_RFID_Arr[k].RFID_IP_ADD,strValue) ;


                                }
                            else
                                {
                                if (k==3){
                                   this->AddState("com3读取失败'"+strReadDATA.mid(2,2)+"'，请重读取");
                                }
                                else if (k==4){
                                   this->AddState2("com4读取失败'"+strReadDATA.mid(2,2)+"'，请重读取");

                                }
                                     IR_CAN_RFID_Arr[k].IR_RFID_ReadSta="00";

                                }
                         }
                      else  if (IR_RFID.IR_RFID_Type=="1")  //3f的RFID
                         {
                          if (Fram_Content.mid(0,2)==Rfid_Data_Head)  //"43"
                           {
                            //IR_CAN_RFID_Arr[Station_NO].IR_RFID_ReadDATA=Copy(Fram_Content,11,20);//截取机型和流水号
                            IR_CAN_RFID_Arr[k].IR_RFID_ReadDATA=Fram_Content.mid(10,6)+Fram_Content.mid(18,14);//截取机型和流水号
                            strReadDATA=IR_CAN_RFID_Arr[k].IR_RFID_ReadDATA;

                            //判断读取的字符是否为A或C，如果是则进行转换和保存，如果不是则不保存、不转换
                            if ((strReadDATA.mid(0,2)=="41") || (strReadDATA.mid(0,2)=="43")|| (strReadDATA.mid(0,2)=="42"))
                                {
                                     //将串口获得的16进制机型和流水数据转换为ASII码机型和流水
                                     strValue=ChangeAreaHEXToStr(strReadDATA);
                                     if (k==3){
                                        this->AddState("com3准备保存数据'"+strValue+"'");
                                     }
                                     else if (k==4){
                                        this->AddState2("com4准备保存数据'"+strValue+"'");

                                     }
                                     //保存读取的记录，包括机型、流水号、时间、站号（与设备名称捆绑）
                                     //保存机型和流水给各设备,并发送接收数据的复位指令给各CAN站
                                     //CAN总线时
                                     // Frm_IR_Main.SaveRecord(strStation_NO,strValue,IR_CAN_RFID_Arr[Station_NO].RFID_IP_ADD,strReadDATA) ;
                                     //3FC_COM时 A195522222
                                      //SaveRecord_3F_COM(strStation_NO,IR_CAN_RFID_Arr[k].RFID_IP_ADD,strReadDATA) ;

                                      SaveRecord_3F_COM(strStation_NO,IR_CAN_RFID_Arr[k].RFID_IP_ADD,strValue) ;


                                }
                            else
                                {
                                if (k==3){
                                   this->AddState("com3读取失败'"+strReadDATA.mid(2,2)+"'，请重读取");
                                }
                                else if (k==4){
                                   this->AddState2("com4读取失败'"+strReadDATA.mid(2,2)+"'，请重读取");

                                }
                                }
                           }
                       }//3f的RFID判断结束

         }
}


//保存读取的记录，包括机型、流水号、时间、站号（与设备名称捆绑）、
void MainWindow::SaveRecord_3F_COM(const QString Station_NO,const QString StrIP,const QString strcontent)
{
  QString EGtype_Flow;
  QString strSQL;
  QString strinputdatetime,LINEID_2,LINEID;
  QDateTime strtime;
  QString str_Check;
  bool strFlow_RE_all;
  bool all_fin=false;
  bool write_fin=false;
  QString str_NOFINISHED;
  qint32 i,k;
  QString Station_NO_temp,EGtype_Flow_temp;



  str_NOFINISHED="0";

   k=Station_NO.toInt();
/*
 * k=Station_NO.toInt();
    if (k<10)
      {
         Station_NO="0"+Station_NO.setNum(k,10);//数字转换为字符串
      }
    else
      {
         Station_NO=Station_NO.setNum(k,10);
      }
*/
   //需要增加检测库里是否有相同流水号的发动机记录
    LINEID_2="1";//生产线分装线代码（只有装配有1总装、2缸体分装、3缸盖分装、4连杆分装）
    LINEID="4";//生产线代码（4装配1线）
    strFlow_RE_all=false; //不重复


    EGtype_Flow=strcontent;
    Station_NO_temp=Station_NO;
    EGtype_Flow_temp=EGtype_Flow;

  //查询从当前ID读取的机型记录在[3F_IR_RFID_READDATALIST ]中是否有未完成的记录
    //如果有则不保存，如果没有则保存
    if (Check_EGTypeFlow_Only_Read_3F_COM34(EGtype_Flow,Station_NO,StrIP))
       {
        if ((IR_RFID.IR_RFID_Type=="2")||(IR_RFID.IR_RFID_Type=="3")) //使用的是OMRON的RFID或条码枪
         {
            //--------------------->20150412{

            //------------------------------------------------------------------
            strFlow_RE_all=false;
             //有相同流水而且未作业完成时,则只是复位RFID,并不执行保存
             if   (Check_EGTypeFlow_Only_Read_3F_COM34_NOFinished(EGtype_Flow,Station_NO,StrIP))
               {
                 str_NOFINISHED="1";

                 EventforHavesamerecordANDnofin(Station_NO,EGtype_Flow);
               }
            //有相同流水且已作业完成时,先删除已经作业的记录,然后再保存
             else
               {
                  if (IR_Infor.ControlforLine_Manu_Auto=="02") //02自动线
                    {
                       EventforHavesamerecordANDfin(Station_NO,EGtype_Flow);
                    }
                }  //------------------------有相同但已完成
          }//---------------->20150412end
        else if (IR_RFID.IR_RFID_Type=="1")
          {
              //有相同流水而且未作业完成时,则只是复位RFID,并不执行保存
              //Edit7.Text=Check_EGTypeFlow_Only_Read_3F_COM34_NOFinished(EGtype_Flow,Station_NO,StrIP) ;
             //exit;
             if (Check_EGTypeFlow_Only_Read_3F_COM34_NOFinished(EGtype_Flow,Station_NO,StrIP))
               {

                 AddState("03号RFID有相同记录，且未完成");
                 return;
               }
            //有相同流水且已作业完成 时,先删除已经作业的记录,然后再保存
             else
               {
                  strFlow_RE_all=false;
                  AddState("03号RFID有相同记录，且已完成");
                  //return;
               }  //------------------------有相同但已完成

          }
       }
    else
       {
           strFlow_RE_all=false;//无相同的流水号
            if (Station_NO=="03")
                  {
                     AddState("03号RFID流水号唯一");

                 }
            else if (Station_NO=="04")
                 {
                     AddState2("04号RFID流水号唯一");
                 }
       }


    //开始没有重号许可执行的是事件
    if (!strFlow_RE_all)
    {
      if(str_NOFINISHED=="0" ) //而且不存在作业未完成--------------
      {
             for (i=1;i<EGtype_Flow.length();i++)
             {
                 if (EGtype_Flow.mid(i,1)=="A")
                    {
                     str_Check=EGtype_Flow.mid(i,EGtype_Flow.length()+1-i);
                      EGtype_Flow=str_Check;

                      break;
                    }
             }
      //------------------------->>>>>>>>>>>>>>>>>>开始
       {
       if( !(Check_Interlock_Set_COM34(EGtype_Flow.mid(0,3),Station_NO,StrIP)))
       {
           if (Station_NO=="03")
                 {
                  AddState("无需作业，放行--"+Station_NO+"号站不保存记录");

                }
           else if (Station_NO=="04")
                {
                  AddState2("无需作业，放行--"+Station_NO+"号站不保存记录");
                }

        IR_CAN_RFID_Arr[k].IR_Belong_STOPGO.STOPGO_State="6";
       }
       else
       {
           strtime=QDateTime::currentDateTime();
           //strinputdatetime =FormatDateTime("yyyy-MM-dd HH:mm:ss",now);  //录入时间，读取系统时间
           strinputdatetime =strtime.toString("yyyy-MM-dd HH:mm:ss ddd");
             if (Station_NO=="03")
                  {

                     AddState("03号RFID正在保存记录:"+strinputdatetime+"");
                  }
             else if (Station_NO=="04")
                 {

                     AddState2("04号RFID正在保存记录:"+strinputdatetime+"");
                 }

           //插入记录
           {

           QSqlQuery * query = new QSqlQuery();

           if (IR_CAN_RFID_Arr[k].IR_IO_Type=="01")  //
             {
               all_fin=true;
               write_fin=false;
             }
           else if (IR_CAN_RFID_Arr[k].IR_IO_Type=="02")  //
             {
               all_fin=false;
               write_fin=true;
             }

           strSQL="insert into 3F_IR_RFID_READDATALIST (RFID_NO,ENG_Type,ENG_TypeFlow,ENG_Flow,";
           strSQL=strSQL+"LINENAME,LINEID,READ_Time,LINEID_2,MCName,";
           strSQL=strSQL+"IP_ADD,LISTTEST,ForPick,ALL_FINISHED,WriteTOMachine_Finished) values(";

           strSQL=strSQL+"'"+Station_NO+"','"+EGtype_Flow.mid(0,3)+"','"+EGtype_Flow+"',";
           strSQL=strSQL+"'"+EGtype_Flow.mid(3,EGtype_Flow.length()-3)+"','"+IR_CAN_RFID_Arr[k].RFID_LINE_NAME+"',";
           strSQL=strSQL+"'"+IR_CAN_RFID_Arr[k].RFID_LINE_ID+"','"+strinputdatetime+"',";
           strSQL=strSQL+"'"+IR_CAN_RFID_Arr[k].RFID_LINE_ID2+"','"+IR_CAN_RFID_Arr[k].RFID_MACHINE_NAME+"',";
           strSQL=strSQL+"'"+StrIP+"','"+strcontent+"','00','"+all_fin+"','"+write_fin+"')";

           query->exec(strSQL);

           if (Station_NO=="03")
                {
                     AddState("03号RFID保存完毕记录");
                 }
           else if (Station_NO=="04")
                 {
                     AddState2("04号RFID保存完毕记录");
                 }

         }

        //保存记录完毕后许可RFID重新读取
        IR_CAN_RFID_Arr[k].IR_RFID_ReadSta="00";

        }
        }//------------------------->>>>>>>>>>>>>>>>>>结束


        }
       else if (str_NOFINISHED=="1")
        {
              if  (Station_NO=="03")
                  {
                     AddState("03号RFID相同记录,不保存记录");
                  }
              else if  (Station_NO=="04")
                 {
                     AddState2("04号RFID相同记录,不保存记录");
                 }

        }


     DisplayForRun(Station_NO_temp,EGtype_Flow_temp);


    } //if not strFlow_RE 判断结束


}

void MainWindow::DisplayForRun(const QString Station_NO,const QString EGtype_Flow)
{
    if (Station_NO=="03")
    {
      AddState("'"+Station_NO+"'--'"+EGtype_Flow+"'");
    }
    else if  (Station_NO=="04" )
    {
      AddState2("'"+Station_NO+"'--'"+EGtype_Flow+"'");
    }

    if (Station_NO=="03")
      {
               IR_Infor.Current_EGtypeFlow_first=EGtype_Flow;
               //frm_IR_Run.Edit_EGType_Flow1.Text=EGtype_Flow;
               //ICFunction.RFID_CurrentEGTypeFlow_WriteTOiniFile("03",EGtype_Flow);

      }
    else if (Station_NO=="04" )
      {

              IR_Infor.Current_EGtypeFlow_second=EGtype_Flow;
              //frm_IR_Run.Edit_EGType_Flow2.Text=EGtype_Flow;
              //ICFunction.RFID_CurrentEGTypeFlow_WriteTOiniFile("04",EGtype_Flow);

      }
}


//检索各CAN站点RFID读取的数据在数据表中是否已经存在，如果存在则加个后缀@
bool MainWindow::Check_Interlock_Set_COM34(const QString strFlow,const QString strStation,const QString strIP)
{

    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;

    strSQL="select count(ID) from 3F_IR_INPUT where  IR_EGtype_NO='"+strFlow+"' and IR_RFID_No='"+strStation+"'and IR_SET_IPaddr='"+strIP+"'";

    ADOQ.setQuery(strSQL);
    //strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    if (strCount>0)
   {
     return true;
   }
   else
   {
    return false;
   }


}
                    //检索各CAN站点RFID读取的数据在数据表中是否已经存在，如果存在则加个后缀@
bool MainWindow::Check_EGTypeFlow_Only_Read_3F_COM34(const QString strFlow,const QString strStation,const QString strIP)
{

    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;
    strSQL="select count(ID) from 3F_IR_RFID_READDATALIST where  ENG_TypeFlow="""+strFlow+""" and RFID_NO="""+strStation+"""and IP_ADD="""+strIP+"""";

    ADOQ.setQuery(strSQL);
    //strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    if (strCount>0)
   {
     return true;
   }
   else
   {
    return false;
   }
}

//检索各CAN站点RFID读取的数据在数据表中是否已经存在，如果存在则加个后缀@
bool MainWindow::Check_EGTypeFlow_Only_Read_3F_COM34_NOFinished(const QString strFlow,const QString strStation,const QString strIP)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount;

    strSQL="select count(ID) from 3F_IR_RFID_READDATALIST where  ENG_TypeFlow="""+strFlow+""" and RFID_NO="""+strStation+"""and IP_ADD="""+strIP+""" and ALL_FINISHED=0";

    ADOQ.setQuery(strSQL);
    //strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    if (strCount>0)
   {
     return true;
   }
   else
   {
    return false;
   }

}


void MainWindow::EventforHavesamerecordANDfin(const QString Station_NO,const QString EGtype_Flow)
{
                   if (Station_NO=="03")
                      {
                         AddState("当前机型流水号---'"+EGtype_Flow+"'");
                         if (IR_RFID.IR_RFID_Type=="2")
                           {
                              delete_RECORD_SAME_COM34(EGtype_Flow,"03",IR_Infor.IP_ADD);
                           }
                     }
                    else if (Station_NO=="04")
                     {
                         AddState2("当前机型流水号---'"+EGtype_Flow+"'");
                        if (IR_RFID.IR_RFID_Type=="2")   //rfid的类型为OMRON
                          {
                              delete_RECORD_SAME_COM34(EGtype_Flow,"04",IR_Infor.IP_ADD);
                          }
                     }

                    if (Station_NO=="03")
                      {
                         AddState("当前机型流水号---'"+EGtype_Flow+"'");
                         AddState("03号RFID有重号并完成,删除旧记录");

                      }
                    else if (Station_NO=="04")
                     {
                         AddState2("当前机型流水号---'"+EGtype_Flow+"'");
                         AddState2("04号RFID有重号并完成,删除旧记录");
                     }
}


void MainWindow::EventforHavesamerecordANDnofin(const QString Station_NO,const QString EGtype_Flow)
{
              if (IR_Infor.ControlforLine_Manu_Auto=="02") //02自动线
                 {
                    if (IR_RFID.IR_RFID_Type=="2") //rfid的类型为OMRON
                       {
                          IR_CAN_RFID_Arr[Station_NO.toInt()].IR_RFID_ReadSta="00";
                          //strFlow_RE_all=true;//有相同的流水号
                       }
                 }

               if (Station_NO=="03")
                  {
                     AddState("当前机型流水号---'"+EGtype_Flow+"'");
                     AddState("03号RFID有重号且未完成,不保存");

                  }
               else if (Station_NO=="04")
                 {
                     AddState2("当前机型流水号---'"+EGtype_Flow+"'");
                     AddState2("04号RFID有重号且未完成,不保存");
                 }
}


//因为重号，需要更新记录将活塞信息写入记录
void MainWindow::delete_RECORD_SAME_COM34(const QString strEGType_Flow,const QString strRFID_NO,const QString strIP)
{
    QString strSQL;
    QSqlQuery * query = new QSqlQuery();
    strSQL="delete  from  3F_IR_RFID_READDATALIST where ENG_TypeFlow='"+strEGType_Flow+"'  and RFID_NO='"+strRFID_NO+"' and IP_ADD='"+strIP+"'";
    query->exec();




    strSQL="delete  from 3F_IR_RUN where IR_RFID_No='"+strRFID_NO+"'and IR_IP_ADD='"+strIP+"'";
    query->exec();

}


void MainWindow::showError(const QSqlError &err)
{
    QMessageBox::critical(this, "Unable to initialize Database",
                "Error initializing database: " + err.text());
}

//数据库测试
void MainWindow::testQSqlQueryModel()
{
    QSqlQueryModel * model =  new QSqlQueryModel();
    QSqlQuery * query = new QSqlQuery();

    //query->exec("create table student(id int primary key auto_increment , name varchar(20)) ");
    query->exec("insert into student(name) values('sg3f01') ");
    query->exec("insert into student(name) values('sg3f02') ");
    query->exec("insert into student(name) values('sg3f03') ");

    model->setQuery("select * from student ");
    model->setHeaderData(0,Qt::Horizontal,tr("id"));
    model->setHeaderData(1,Qt::Horizontal,tr("name"));

   // QTableView * view = new QTableView();
     //ui->todolist_tableView->setModel(model); //view使用界面定义的控件
   //  ui->todolist_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
   //  ui->todolist_tableView->setBackgroundRole(QPalette::Dark);
     //ui->todolist_tableView->show();
}

void MainWindow::readJobList(const QString strIP, const QString strCom, const QString strEGFlowNo)
{
    QSqlQuery query;
    QString strEGtypeno = strEGFlowNo.left(3);
    QString strSQL = " insert into t_ir_rfid_readdatalist(RFID_NO,ENG_Type,ENG_TypeFlow,IP_ADD ) "
                     " values('"
                        + strCom + "','"
                        + strEGtypeno + "','"
                        +  strEGFlowNo + "','"
                        +  strIP+ "')";

    qDebug()<<"StrSQL = " +strSQL ;
    query.exec(strSQL);
    this->AddState("0000000000000");


    /*
    QString strSQL2 = " insert into t_ir_run(ir_ip_add,ir_rfid_no,eng_type,mcname,read_time,eng_typeflow ) "
                     " select ir_set_ipaddr,ir_rfid_no,ir_egtype_no,ir_partlist_name,now(),'" + strEGFlowNo + "' "
                     " from t_ir_input "
                     "  where ir_set_IPaddr = '"
                        + strIP + "' and ir_rfid_no = '"
                        + strCom  + "' and ir_egtype_no = '"
                        + strEGtypeno + "'";

    qDebug()<<"strSQL2 = " +strSQL2 ;
    query.exec(strSQL2);
   */

}
void MainWindow::QueryTodoListByID(QString equipid)
{
    QSg3fQueryModel * todolist_model =  new QSg3fQueryModel(); //model需要new一个对象,不能直接使用
    QString strSql ="select * from student where id = " + equipid;
    qDebug()<<strSql;
    todolist_model->setQuery(strSql);
    todolist_model->setHeaderData(0,Qt::Horizontal,tr("id"));
    todolist_model->setHeaderData(1,Qt::Horizontal,tr("name"));

    //ui->todolist_tableView->setModel(todolist_model); //view使用界面定义的控件
   // ui->todolist_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

}


void MainWindow::QueryTodoListByID_Com3(QString equipid)
{

    QSqlQueryModel *com3model =  new QSqlQueryModel(); //model需要new一个对象,不能直接使用
    QString strSql ="select * from student where id = " + equipid;
    qDebug()<<strSql;
    com3model->setQuery(strSql);
    com3model->setHeaderData(0,Qt::Horizontal,tr("id"));
    com3model->setHeaderData(1,Qt::Horizontal,tr("name"));
    //QTableView *com3view = new QTableView;
    ui->com3_tableView->setModel(com3model); //view使用界面定义的控件
    //com3view->show();
}


void MainWindow::QueryTodoListByID_Com4(QString equipid)
{

    QSqlQueryModel *com4model =  new QSqlQueryModel(); //model需要new一个对象,不能直接使用
    QString strSql ="select * from student where id = " + equipid;
    qDebug()<<strSql;
    com4model->setQuery(strSql);
    com4model->setHeaderData(0,Qt::Horizontal,tr("id"));
    com4model->setHeaderData(1,Qt::Horizontal,tr("name"));
    //QTableView *com3view = new QTableView;
    ui->com4_tableView->setModel(com4model); //view使用界面定义的控件
    //com4view->show();

}

void MainWindow::on_eqmid3_lineEdit_textChanged(const QString &arg1)
{
    qDebug()<<"receive from RFID "<<arg1;
    QueryTodoListByID(arg1);
}

void MainWindow::on_eqmid4_lineEdit_textChanged(const QString &arg1)
{
    qDebug()<<"receive from RFID "<<arg1;
    QueryTodoListByID(arg1);
}

void MainWindow::on_reset_pushButton_clicked()
{
    //scanJobProgress();
    //readJobList(strLocalIP,strCom,"C28C000001");
    //readJobList("192.168.100.2","03","C28C000001");
/*
    QString RFID_NO="02";

    QString temp="AA800355AA40000255";//AA800055AA40000255AA070055
    this->AddState("com2<<=:'"+temp+"'");
    recData_fromICLst_CAN_RFID_COM[RFID_NO.toInt()]=temp;//用于处理接收到的内容

    CheckCMD_CAN_COM2(RFID_NO);//处理接收到的内容

    */
    //SendCMD_TO_Station_By_CANCOM("64","00");
    //SendCMD_TO_Station_By_CANCOM("64","FF");

    /*2进制转16进制
    qint32 i,j,k;
    QString str_CMD_Value;
    QString strtemp;
    bool bok=false;
    str_CMD_Value="0010";
    k=str_CMD_Value.toInt(&bok,2);
    this->AddState(strtemp);
    strtemp=strtemp.setNum(k,16);
    this->AddState(strtemp);
    */

    /*
QString strtemp;
    //取得程序号 16进制
    strtemp=strtemp.setNum(5,16);
    this->AddState("程序22---"+strtemp+"");
    //将取得程序号转换为2进制的输出文本，以实现与设定端子针脚扯上关系
    strtemp=strtemp.setNum(5,2);
    this->AddState("程序33---"+strtemp+"");

*/
//QString strtemp;
    //strtemp=Check_Bit_Signal("01",16);
    //Check_Bit_Signal("04",16);



//    Check_Station_Fram_IO("03","AA1F0155");
/*
    QSqlQuery * query = new QSqlQuery();
    QString strSQL,StrIP,strstationNO,strtemp;
    StrIP="192.168.100.1";
    strstationNO="31";
    strtemp="5";
    strSQL="Update 3F_IR_RUN set IR_Finished='"+strtemp_OK+"' where IR_IP_ADD='"+StrIP+"' and IR_Station_NO='"+strstationNO+"'  and IR_Finished='"+strtemp_NG+"' and  IR_Prg_NO ='"+strtemp+"'";
    query->exec(strSQL);
*/


    DeleteFin_IR_RUN_Record_ByRFIDNO_IP_COM34("03","192.168.100.1");
}

void MainWindow::on_testpushButton_clicked()
{
    readJobList(strLocalIP,strCom3,"C28C000001");
}



qint32 MainWindow::StrToInt(const char *str)
{
    return atoi(str);
}


//获得最大的RFID站号 ,整线控制
//qint32 MainWindow::Query_RFID_CAN_MaxStationNO(const QString Line_ID,const QString Line_ID_2,const QString strIP, const QString InitType)
qint32 MainWindow::Query_RFID_CAN_MaxStationNO(const QString strIP, const QString InitType)
{
QSqlQueryModel ADOQ;
QString strSQL;
qint32 strCount;
qint32 strRet;
strRet=0;



 if(InitType=="00")//整线
 {
     strSQL="select Count(RFID_Station_NO) from 3F_CAN_RFID_STATIONNO_SET where   RFID_IP_ADD='"+strIP+"'";
 }
 else if (InitType=="01")//单工序
 {
     strSQL="select Count(RFID_Station_NO) from 3F_CAN_RFID_STATIONNO_SET where   RFID_IP_ADD='"+strIP+"'";

 }
 ADOQ.setQuery(strSQL);
 strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();


 if (strCount>0 )
 {
   //this->AddState(""+Line_ID_2+"-"+strIP+"-"+InitType+"");
   if (InitType=="00")
    {
        strSQL="select max(RFID_Station_NO) from 3F_CAN_RFID_STATIONNO_SET where   RFID_IP_ADD='"+strIP+"'";
    }
   else if (InitType=="01")
   {
        strSQL="select max(RFID_Station_NO) from 3F_CAN_RFID_STATIONNO_SET where   RFID_IP_ADD='"+strIP+"'";

   }

   ADOQ.setQuery(strSQL);
   strRet = ADOQ.data(ADOQ.index(0, 0)).toInt();

 }
 return strRet;
}

void MainWindow::InitIR_RFID_ComArray(const QString InitType)
{
   qint32 i;
        //故只要查询3F_CAN_RFID_STATIONNO_SET数据表中最大的RFID_Station_NO
    //InitType为区分单机控制还是整线控制，01表示单机，00表示整线
    //ui->eqmid4_lineEdit->setText(IR_RFID.IR_RFID_Type);

    IR_Infor.Control_MaxRFIDNO=Query_RFID_CAN_MaxStationNO(IR_Infor.IP_ADD,InitType);


    RFID_CAN_MaxStationNO=IR_Infor.Control_MaxRFIDNO;

    if (RFID_CAN_MaxStationNO<3) //如果没有设置联锁，则只能串口1和串口2有效
    {
           RFID_CAN_MaxStationNO=4;
    }


    // setlength(IR_CAN_RFID_Arr,RFID_CAN_MaxStationNO+1);
    IR_CAN_RFID_Arr=new IR_CAN_RFIDTYPE[RFID_CAN_MaxStationNO+1];

    //给数组IR_CAN_RFID_Arr[i]变量初始化
    for(i=1;i<RFID_CAN_MaxStationNO+1;i++)
    {
       IR_CAN_RFID_Arr[i].RFID_Station_NO=" ";
       IR_CAN_RFID_Arr[i].IR_RFID_ReadSta="00"; //读卡状态
       IR_CAN_RFID_Arr[i].IR_CycleTimeOver="00"; //00节拍未到达，01节拍到达
       IR_CAN_RFID_Arr[i].IR_CycleTimeOver_alramsta="00";

    }

    //this->AddState(""+tempstr.setNum(RFID_CAN_MaxStationNO,10)+"");

    //给数组IR_CAN_RFID_Arr[i]变量附值
    Query_RFID_CAN_SetInfor(IR_Infor.IP_ADD,InitType);//'4'装配1线，'3'缸盖分装1线//获取IP和IP站号



    //连接所有已经设定了的设备PLC ，如果是整线控制则要考虑连接对应IP号的PLC，但单机为PC就不要连接了
    /*
   {
    if InitType='00' then
      begin
        for i:=1 to RFID_CAN_MaxStationNO do
         begin
           if IR_CAN_RFID_Arr[i].RFID_Station_NO<>'' then
             begin
             //此处的i值与CAN站的站号一致，而CAN站的站号（RFID_Station_NO）设定存储于数据表3F_CAN_RFID_STATIONNO_SET
             //根据站号，即i值，初始化与PLC相关的数据连接，通讯连接 ，而i值也就与PLC的连接是数组编号一致
               ICPLCCommunication.InitPLC_netport_arr(TrimRight(IR_CAN_RFID_Arr[i].RFID_IP_ADD),IR_CAN_RFID_Arr[i].RFID_IP_Station_NO,i);
               Application.ProcessMessages;
             end;
         end;
      end;
   }
   */
}



//读取数据库3F_CAN_RFID_STATIONNO_SET中配置的RFID（CAN总线）的站点及对应的设备信息（主要是对应的IP地址）
//整线控制
void MainWindow::Query_RFID_CAN_SetInfor(const QString strIP,const QString InitType)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount,j,i;

    //整线控制
    if (InitType=="00")
    {
     strSQL="select * from 3F_CAN_RFID_STATIONNO_SET where RFID_IP_ADD='"+strIP+"'";
    }
    //单机控制
    else   if(InitType=="01")
    {
     strSQL="select * from 3F_CAN_RFID_STATIONNO_SET where RFID_IP_ADD='"+strIP+"'";
    }
    ADOQ.setQuery(strSQL);
    //取得一共有多少条件记录
    //strCount = ADOQ.rowCount();
    strCount = ADOQ.rowCount();
    //this->AddState("---"+strtemp.setNum(strCount,10)+"");
    //循环将各条记录的各字段内容赋值给各数组变量
    for(j=0;j<strCount;j++)
    {

      i=ADOQ.record(j).value("RFID_Station_NO").toInt();//CAN站的站号，用于绑定PLC的IP地址数组编号
      IR_CAN_RFID_Arr[i].RFID_Station_NO=ADOQ.record(j).value("RFID_Station_NO").toString();
      IR_CAN_RFID_Arr[i].RFID_LINE_NAME=ADOQ.record(j).value("RFID_LINE_NAME").toString();
      IR_CAN_RFID_Arr[i].RFID_MACHINE_NAME=ADOQ.record(j).value("RFID_MACHINE_NAME").toString();

      IR_CAN_RFID_Arr[i].RFID_IP_ADD=ADOQ.record(j).value("RFID_IP_ADD").toString();

      IR_CAN_RFID_Arr[i].RFID_CANLine_NO=ADOQ.record(j).value("RFID_CANLine_NO").toString();
      IR_CAN_RFID_Arr[i].RFID_LINE_ID=ADOQ.record(j).value("RFID_LINE_ID").toString();
      IR_CAN_RFID_Arr[i].RFID_LINE_ID2=ADOQ.record(j).value("RFID_LINE_ID2").toString();

      IR_CAN_RFID_Arr[i].RFID_SendToPLC_IP=ADOQ.record(j).value("RFID_SendToPLC_IP").toString();;
      //IR_CAN_RFID_Arr[i].RFID_LINE_ID2=ADOQ.record(j).value("RFID_LINE_ID2").toString();;
      IR_CAN_RFID_Arr[i].RFID_EGType_Daddr=ADOQ.record(j).value("RFID_EGType_Daddr").toString();;
      IR_CAN_RFID_Arr[i].RFID_EGType_Daddr_Length=ADOQ.record(j).value("RFID_EGType_Daddr_Length").toString();;
      IR_CAN_RFID_Arr[i].RFID_FDJTypeState=0;
      IR_CAN_RFID_Arr[i].RFID_EGFlow_Daddr=ADOQ.record(j).value("RFID_EGFlow_Daddr").toString();;
      IR_CAN_RFID_Arr[i].RFID_EGFlow_Daddr_Length=ADOQ.record(j).value("RFID_EGFlow_Daddr_Length").toString();;
      IR_CAN_RFID_Arr[i].RFID_PLCRequestEGType_Daddr=ADOQ.record(j).value("RFID_PLCRequestEGType_Daddr").toString();;
      //(01-读写机型给PLC的RFID,02-绑定作业联锁的RFID)
      IR_CAN_RFID_Arr[i].IR_IO_Type=ADOQ.record(j).value("RFID_Station_Usefor_Type").toString();;
      IR_CAN_RFID_Arr[i].RFID_IO_Init='1';
      IR_CAN_RFID_Arr[i].IR_Belong_STOPGO.STOPGO_Eanbled=false;
      IR_CAN_RFID_Arr[i].IR_Belong_STOPGO.STOPGO_Station_NO="00";
      IR_CAN_RFID_Arr[i].IR_Belong_STOPGO.STOPGO_Pin_Input_PinNo="00";
      IR_CAN_RFID_Arr[i].IR_Belong_STOPGO.STOPGO_Pin_Output_PinNo="00";


      IR_CAN_RFID_Arr[i].IR_Belong_PROCESSNO=ADOQ.record(j).value("RFID_Process_Station_NO").toString();;
      //IR_CAN_RFID_Arr[i].IR_IO_Pin_Input_length:=ADOQ.record(j).value("RFID_PIN_InPut_Length").toString();;
      //IR_CAN_RFID_Arr[i].IR_IO_Pin_Output_length:=ADOQ.record(j).value("RFID_PIN_OutPut_Length").toString();;
      //IR_CAN_RFID_Arr[i].IR_IO_Pin:=ADOQ.record(j).value("RFID_Station_Usefor_Type").toString();;


      IR_CAN_RFID_Arr[i].IR_MAP_PLC_D_Arrival=ADOQ.record(j).value("IR_PLC_D_Arrival").toString();; //PLC对应的托盘到位D地址
      IR_CAN_RFID_Arr[i].IR_MAP_PLC_D_Reset=ADOQ.record(j).value("IR_PLC_D_Reset").toString();; //PLC对应的托盘复位D地址
      IR_CAN_RFID_Arr[i].IR_MAP_PLC_D_Fin=ADOQ.record(j).value("IR_PLC_D_Fin").toString();; //PLC对应的作业完成D地址

      Query_RFID_CAN_MC_NetStationInfor(i,IR_CAN_RFID_Arr[i].RFID_IP_ADD); //绑定PLC的IP地址数组编号
      Query_RFID_CAN_MC_NetStationInfor_STOPGO(i,IR_CAN_RFID_Arr[i].RFID_IP_ADD); //绑定PLC的IP地址数组编号

    }
}


//读取数据库3F_IP_ADD中配置的IP站点及对应的设备信息（主要是对应的站点，网络号）
void MainWindow::Query_RFID_CAN_MC_NetStationInfor(qint32 TCP_OrderNO,const QString IP)
{
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount,j;

    ui->eqmid4_lineEdit->setText("IP""");
    strSQL="select * from 3F_IP_ADD where  IP_Addr='"+IP+"'";
    ADOQ.setQuery(strSQL);
    //strCount = ADOQ.data(ADOQ.index(0, 0)).toInt();
    //strCount = (ADOQ.record(0).value("id")).toInt();
    //取得一共有多少条件记录
    strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    for(j=0;j<strCount;j++)
    {
        //-------------------------------------------------------------1
        //if  (length(ADOQ.record(j).value("Station_NO").toString())!=0) {//注意trimright的判断
        if  (ADOQ.record(j).value("Station_NO").toString()!=0) {//注意trimright的判断
        IR_CAN_RFID_Arr[TCP_OrderNO].RFID_IP_Station_NO =ADOQ.record(j).value("Station_NO").toInt();
        }
       else
        {
         IR_CAN_RFID_Arr[TCP_OrderNO].RFID_IP_Station_NO =1;
        }

       IR_CAN_RFID_Arr[TCP_OrderNO].RFID_IP_NET_NO=ADOQ.record(j).value("NET_NO").toString();
       IR_CAN_RFID_Arr[TCP_OrderNO].RFID_IP_Group_NO=ADOQ.record(j).value("Group_NO").toString();
    }
}


//查询阻挡器相关的输入，输出引脚设定值
 //读取数据库[3F_IR_STATION_PIN_INPUT_STOPGO]中配置的IP站点及对应的设备信息（主要是对应的站点，网络号）
void MainWindow::Query_RFID_CAN_MC_NetStationInfor_STOPGO(qint32 TCP_OrderNO,const QString IP)
{
    QString constvalue;
    QString strStationNo;
    QSqlQueryModel ADOQ;
    QString strSQL;
    qint32 strCount,j;


  //查询输入引脚
    constvalue="01";
    strSQL="select * from 3F_IR_STATION_PIN_INPUT_STOPGO where  IR_SET_IPaddr='"+IP+"' and IR_RFID_No='"+strStationNo+"' and IR_PIN_INPUT_OUTPUT='"+constvalue+"'";
    ADOQ.setQuery(strSQL);


    if (TCP_OrderNO<10)
      {
         strStationNo="0"+strStationNo.setNum(TCP_OrderNO,10);
      }
    else
      {
         strStationNo=strStationNo.setNum(TCP_OrderNO,10);
      }

    //取得一共有多少条件记录
    strCount = ADOQ.rowCount();
    //循环将各条记录的各字段内容赋值给各数组变量
    for(j=0;j<strCount;j++)
    {
    IR_CAN_RFID_Arr[TCP_OrderNO].IR_Belong_STOPGO.STOPGO_Station_NO=ADOQ.record(j).value("IR_Station_NO").toString();
    IR_CAN_RFID_Arr[TCP_OrderNO].IR_Belong_STOPGO.STOPGO_Pin_Input_PinNo=ADOQ.record(j).value("IR_Pin_No_Set").toString();
    }
    //--------------------------------------

    //查询输出引脚
      constvalue="02";
      strSQL="select * from 3F_IR_STATION_PIN_INPUT_STOPGO where  IR_SET_IPaddr='"+IP+"' and IR_RFID_No='"+strStationNo+"' and IR_PIN_INPUT_OUTPUT='"+constvalue+"'";
      ADOQ.setQuery(strSQL);
   //取得一共有多少条件记录
     strCount = ADOQ.rowCount();
   //循环将各条记录的各字段内容赋值给各数组变量
     for(j=0;j<strCount;j++)
     {
       IR_CAN_RFID_Arr[TCP_OrderNO].IR_Belong_STOPGO.STOPGO_Pin_Output_PinNo=ADOQ.record(j).value("IR_Pin_No_Set").toString();
     }

}

void MainWindow::AddState(const QString state)
{

   if (Row_AddState<60)
   {
    ui->listWidget_start->addItem(state);
    Row_AddState++;
   }
   else
   {
     ui->listWidget_start->clear();
     Row_AddState=0;
     ui->listWidget_start->addItem(state);
   }

   /*
    ui->textBrowser_start->setText(ui->textBrowser_start->document()->toPlainText() + state);
   // ui->textBrowser_start->setText(state);
    QTextCursor cursor = ui->textBrowser_start->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser_start->setTextCursor(cursor);
  */

}

//显示状态内容
void MainWindow::AddState2(const QString state)
{

    if (Row_AddState2<30)
    {
     ui->listWidget_end->addItem(state);
     Row_AddState2++;
    }
    else
    {
      ui->listWidget_end->clear();
      Row_AddState2=0;
      ui->listWidget_end->addItem(state);
    }
/*
    ui->listWidget_start->addItem(state);

    ui->textBrowser_end->setText(ui->textBrowser_end->document()->toPlainText() + state);
    //ui->textBrowser_end->setText(state);
    QTextCursor cursor = ui->textBrowser_end->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser_end->setTextCursor(cursor);
    //ui->statusBar->showMessage(tr("成功读取%1字节数据").arg(temp.size()));//temp.size()接收到的字节数
*/
}


void MainWindow::startOrStopThreadA()
{
    if(threadA.isRunning())
    {
        ui->pushButton_A->setText(tr("Stop A"));
        threadA.stop();
        ui->pushButton_A->setText(tr("Start A"));
    }
    else
    {
        ui->pushButton_A->setText(tr("Start A"));
        threadA.start();
        ui->pushButton_A->setText(tr("Stop A"));
   }
}

void MainWindow::startOrStopThreadB()
{
    if(threadB.isRunning())
    {
        ui->pushButton_B->setText(tr("Stop B"));
        threadB.stop();
        ui->pushButton_B->setText(tr("Strat B"));
    }
    else
    {
        ui->pushButton_B->setText(tr("Start B"));
        threadB.start();
        ui->pushButton_B->setText(tr("Stop B"));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    threadA.stop();
    threadB.stop();
    threadA.wait();
    threadB.wait();
    event->accept();
}

void MainWindow::close()
{
    exit(0);
}

void MainWindow::on_pushButton_quit_clicked()
{
    close();
}

void MainWindow::on_pushButton_A_clicked()
{
  startOrStopThreadA();
}

void MainWindow::on_pushButton_B_clicked()
{
  startOrStopThreadB();
}

void MainWindow::on_forceacc_pushButton_clicked()
{

    //ui->ReadNewEGTypeFlow_First.Enabled =true;

        //记录操作事件
    //strinputdatetime =FormatDateTime('yyyy-MM-dd HH:mm:ss',now);
    //保存强制放行记录
  // SaveUSER_SysOP_Record('强制放行作业联锁', strinputdatetime, 'admin','04');
   this->AddState("该工序复位操作");

    if (IR_Infor.ControlforLine_Manu_Auto=="01")//01手动线
    {
            IO_ResetCheck_3F_COM(Start_stationNO);//WorkThread.
    }
    else if (IR_Infor.ControlforLine_Manu_Auto=="02" )//02自动线
    {
          if (IR_Infor.PC_DB_IO=="02" )//IO
            {
               //IO_ResetCheck_3F_COM_ForAutoLine_New(Start_stationNO);
               IO_ResetCheck_3F_COM(Start_stationNO);//WorkThread.
            }
          else if (IR_Infor.PC_DB_IO=="01" )//DB
            {
               //IO_ResetCheck_3F_COM_ForAutoLine_New_PC_ENABLED(Start_stationNO);
               IO_ResetCheck_3F_COM(Start_stationNO);//WorkThread.
            }
     }


}



void MainWindow::on_pushButton_2_clicked()
{
    //ui->ReadNewEGTypeFlow_First.Enabled =true;

        //记录操作事件
    //strinputdatetime =FormatDateTime('yyyy-MM-dd HH:mm:ss',now);
    //保存强制放行记录
    //SaveUSER_SysOP_Record('强制放行作业联锁', strinputdatetime, 'admin','04');
    this->AddState2("该工序复位操作");
 if (IR_Infor.ControlforLine_Manu_Auto=="01")//01手动线
 {
         IO_ResetCheck_3F_COM(End_stationNO);//WorkThread.
 }
 else if (IR_Infor.ControlforLine_Manu_Auto=="02" )//02自动线
 {
       if (IR_Infor.PC_DB_IO=="02" )//IO
         {
            //IO_ResetCheck_3F_COM_ForAutoLine_New(End_stationNO);
            IO_ResetCheck_3F_COM(End_stationNO);//WorkThread.
         }
       else if (IR_Infor.PC_DB_IO=="01" )//DB
         {
            //IO_ResetCheck_3F_COM_ForAutoLine_New_PC_ENABLED(End_stationNO);
            IO_ResetCheck_3F_COM(End_stationNO);//WorkThread.
         }
  }
}

void MainWindow::on_ReadNewEGTypeFlow_First_clicked()
{
    ReadIDValue_03_old="";
}

void MainWindow::on_ReadNewEGTypeFlow_Second_clicked()
{
    ReadIDValue_04_old="";
}

void MainWindow::on_pushButton_clicked()
{
    Parasetting *w=new Parasetting(this);//生成mainwindow界面窗口
    //loginDialog login;//生成loginDialog界面窗口
   // this->hide();
    w->show();
}
