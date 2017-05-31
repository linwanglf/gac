#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "thread.h"
#include <QMainWindow>
#include <QtSql>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "qextserial/qextserialport.h"
#include <QCloseEvent>

//延时，TIME_OUT是串口读写的延时
#define TIME_OUT 10

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private:
    Ui::MainWindow *ui;
    QTimer *obotimer;
    QextSerialPort *myCom2;
    QextSerialPort *myCom3;
    QextSerialPort *myCom4;
    QSqlQueryModel * todolist_model;
    QSqlQuery  * todolist_query;

    Thread threadA;//创建线程A
    Thread threadB;//创建线程B
    QString strCom1;
    QString strCom2;
    QString strCom3;
    QString strCom4;
    QString strCom5;
    QString strCom6;

    QString frame_content_com2="";//接收到的帧内容
    QString frame_first_com2="";//接收到的帧头
    QString frame_content_com3="";//接收到的帧内容
    QString frame_first_com3="";//接收到的帧头
    QString frame_content_com4="";//接收到的帧内容
    QString frame_first_com4="";//接收到的帧头
    QString frame_content_com1="";//接收到的帧内容
    QString frame_first_com1="";//接收到的帧头

    QString strBaudRate1;
    QString strBaudRate2;
    QString strBaudRate3;
    QString strBaudRate4;
    QString strBaudRate5;
    QString strBaudRate6;

    QString strLocalIP;
    QString strHost;
    QString strUser;
    QString strPassword;
    QString strScanjobprogresstimer;

    qint32 Row_AddState=0;
    qint32 Row_AddState2=0;



    qint32 Start_stationNO;
    qint32 End_stationNO;
    QString strtemp_NG,strtemp_OK;
    bool bool_ok,bool_ng;
    QString Prg_Total;
    //用于记忆读卡的 内容,防止不断读卡-----20160314
    QString ReadIDValue_03_old;
    QString ReadIDValue_03_new;

    QString ReadIDValue_04_old;
    QString ReadIDValue_04_new;

    QString PC_ANDON_WriteFIN;
    QString PC_ANDON_WriteFIN_Second;

    QString orderLst_CAN_RFID_COM[6];//当前RFID控制的有效站
    QString recData_fromICLst_CAN_RFID_COM[6];//当前RFID控制的有效站
    qint32 i_recv_total=50;
    qint32 i_recv=0;
    QString recData_from_COM2[50];//当前RFID控制的有效站
    QString timeinitInterval;//初始化的定时器的扫描周期
    QString Timer_RFID_ReadStartCycle_StartInterval;//前工序读卡的定时器的扫描周期
    QString Timer_RFID_ReadStartCycle_EndInterval;//后工序读卡的定时器的扫描周期
    QString timeOSInterval;//程序循环的定时器的扫描周期

    QTimer *Timer_init;
    QTimer *Timer_RFID_ReadStartCycle_Start; //前工序读卡触发定时器
    QTimer *Timer_RFID_ReadStartCycle_End;  //后工序读卡触发定时器
    QTimer *timeOS; //程序循环触发定时器
    QTimer *time_com2; //循环处理COM2接收到的数据触发定时器


    QString reset3;
    QString reset4;
    QString TextSelEnabled;
    QString strTuopan_ON;
    QString strTuopan_OFF;
    bool CAN_TuoPan_ON;//CAN总线检测的托盘到位信号
    qint32 TuopanState_Head;
    QString strEnd_stationNO;
    QString strStart_stationNO;
    QString CAN_COM;
    QString TEST1;
    QString TEST2;
    qint32 COUNT_NO;
    qint32 COUNT_NO2;
    QString PC_TC_BarCodeCheck;

private slots:

    //初始化
    void initialTimer();//为实现与delphi平台结构类同
    void initialConfiguration();
    void initialCom_Var();

    void initComm2();
    void readMyCom2();
    void QueryTodoListByID(QString equipid);
    void CheckCMD_CAN_COM2(QString RFID_NO);
    void Check_Station_Fram_IO(QString RFID_NO,QString strFram_Content);

    void initComm3();
    void readMyCom3();
    void QueryTodoListByID_Com3(QString equipid);

    void initComm4();
    void readMyCom4();
    void QueryTodoListByID_Com4(QString equipid);


    void CheckIR_SubDeviceOP_and_NODevice(const QString strRFID_NO,const QString strIP,const QString strStation_NO,const QString Strbit);
    //根据输入的完成信号更新数据表
    void Update_IR_RUN_RE_FIN_IO(const QString StrIP,const QString strstationNO,const QString strbit);

    //联锁循环开始 后，当串口接收到有来自IO站的零件指示的反馈信息（取料完成）时，
    //需要更新输出值，并输出到IO站，以熄灭指示灯
    void  IR_Cycle_ReStart_IO(const QString strIP,const QString strStation_NO,const QString Str_InputPINNo);
    //如果启动系统时检测到[3F_IR_RUN] 表中还有未完成的联锁作业，则
    //联锁循环继续
    void IR_Cycle_Cotinuce_IO_NOFinished(const QString strIP,const QString strRFID_NO,const QString strStationNO);//strStation_NO为RFID绑定的站号



    //处理从串口3和串口4读取的信息
    void CheckCMD_CAN_RFID_3F(const QString StrstationNO,const QString StrtempINput);
    void RFID_Reset_3F_COM34(const qint32 i);
    //判断是否需要保存读卡记录及是否需要更新记录的状态
    void RFID_ResetCheck_3F_COM34(qint32 StrRFID_NO);
    void Check_Station_Fram_COM34(const qint32 Station_NO,const QString Fram_NO,const QString Fram_Content);
    //保存读取的记录，包括机型、流水号、时间、站号（与设备名称捆绑）
    void  SaveRecord_3F_COM(const QString Station_NO,const QString StrIP,const QString strcontent);
    bool  Check_EGTypeFlow_Only_Read_3F_COM34(const QString strFlow,const QString strStation,const QString strIP);
    bool  Check_EGTypeFlow_Only_Read_3F_COM34_NOFinished(const QString strFlow,const QString strStation,const QString strIP);
    void  EventforHavesamerecordANDfin(const QString Station_NO,const QString EGtype_Flow);
    void  EventforHavesamerecordANDnofin(const QString Station_NO,const QString EGtype_Flow);
    //因为重号，需要更新记录将活塞信息写入记录
    void  delete_RECORD_SAME_COM34(const QString strEGType_Flow,const QString strRFID_NO,const QString strIP);
    void  DisplayForRun(const QString Station_NO,const QString EGtype_Flow);
    //检索各CAN站点RFID读取的数据在数据表中是否已经存在，如果存在则加个后缀@
    bool Check_Interlock_Set_COM34(const QString strFlow,const QString strStation,const QString strIP);
    //更新数据表[3F_IR_RFID_READDATALIST] 的字段ALL_FINISHED
    void UPdate_IR_RFID_READDATALIST_ByRFIDNO_IP_COM34(const QString StrRFID_NO,const QString StrIP);
    void DeleteFin_IR_RUN_Record_ByRFIDNO_IP_COM34(const QString StrRFID_NO,const QString StrIP);
    void DeleteIR_RUN_Record_ByRFIDNO_IP_COM34(const QString StrRFID_NO,const QString StrIP);


    //str_RFID_NO串口号
    void ReadWrite_BySpcom(const QString str_RFID_NO,const QString str_CMD_Value);
    //串口发送数据
    void sendData_CAN_RFID_BySpcom(const qint32 IntRFID_NO);
    //串口2发送数据给IO
    void sendData_CAN_RFID_BySpcom_IO(const qint32 IntRFID_NO);
   //发送数据
    void sendMsg(const qint32 IntRFID_NO,const QString str);
    //发送数据给IO
     void sendMsg_IO(const qint32 IntRFID_NO,const QString str);
    //各定时器的相关事件
    void Timer_initProgress();
    void Timer_RFIDStart_Progress();
    void Timer_RFIDEnd_Progress();
    void Timer_OS_Progress();

    void CheckCMD_CAN_COM2_FORTIMER();
    void Timer_COM2_Progress();

    //线体上的ID是统一管理的情况时，读取各工位“读卡完毕信号”
    void RequestReadIDFinished_CAN_Total();
    //根据本系统配置的IP地址、绑定RFID的站号，查询对应的作业3F_IR_INPUT是否有相关记录
    bool QueryIR_INPUT_LIST_BE(const QString StrIP,const QString strstationNO);
    //检查是否全部作业完成
    QString Query_IR_Finished_IO(const QString Str_RFID_NO,const QString StrIP);
    QString QueryIR_Olddest_ID_FromLISTTable_IO(const QString Str_RFID_No,const QString StrIP);
    //更新数据表[3F_IR_RFID_READDATALIST] 的字段ALL_FINISHED
    void UPdate_IR_RFID_READDATALIST(const QString StrMIN_ID);
    void Delete_IR_RFID_READDATALIST_COM34(const QString StrID);
    //更新数据表[3F_IR_ANDON] 的字段 FINISH_STATE
    void  UPdate_IR_ANDON_Fin(const QString StrRFID_NO,const QString StrIP,const QString StrState);
    //更新数据表[3F_IR_ANDON] 的字段 FINISH_STATE
    void ADD_IR_ANDON_Fin(const QString StrRFID_NO,const QString StrIP,const QString StrState);
    //节拍到达，检测是否作业完成，如果没有则驱动蜂鸣器报警
    void  Alram_for_CycleArraved();
    //向绑定的RFID强制放行阻挡气缸
    void  EnableSTOPGO_GO_PC(qint32 Rfid_NO);
    bool QueryIR_RUN_NO_FIN(const QString StrIP,const QString strstationNO);

    //联锁循环继续
    void  IR_Cycle_Continue_IO(const QString strIP,const QString strRFID_NO);//strStation_NO为RFID绑定的站号
    //根据RFID站号，从[3F_IR_RUN] 查询对应的机型短码
    QString  QueryEGType_IR_RUN_NO_FIN(const QString Str_RFID_No,const QString StrIP);
   //防止站号重复，查询[3F_IR_INPUT]   QueryTypeGroupNO
    QString  QueryTypeGroupNO_Cycle_Continue(const QString StrEGtype);
    //检索各CAN站点RFID读取的数据在数据表中是否已经存在，如果存在则加个后缀@
    bool Check_Interlock_Set(const QString strFlow,const QString strStation,const QString strIP);
    //查询绑定的RFID受控的IO站数 [3F_IR_RUN]
    QString Query_IO_CAN_MaxStationNO_Cycle(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP);

    //初始化对应RFID管辖的IO站点信息
    void  InitIOStationInfor(const QString strIP,qint32 intRFID_NO,const QString strRFID_NO,qint32 intMaxstationNO);
    //发程序给指示灯
    void  Query_RFID_CAN_SetInfor_IO_ByUsefor(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP,const QString StrModel_Usefor);
    //发送零件程序给各站点
    void  SendPartshowPrgNO_to_IOStation(const qint32 intRFID_NO,const qint32 intMaxstationNO);//strStation_NO为RFID绑定的站号
    //发送拧紧程序给各站点     SendGunPrgNO_to_IOStation_Continue   SendGunPrgNO_to_IOStation_Continue
    void  SendGunPrgNO_to_IOStation_Continue(const QString strIP,const qint32 intRFID_NO,const QString strRFID_NO,const qint32 intMaxstationNO);
    //根据站号，发送程序号--CAN总线
    void Send_StationNO_PrgNO_TO_IO_PartshowPrgNO(const qint32 i_RFID_NO,const qint32 int_j,const qint32 Order_NO,const QString str_IR_Station_NO,const QString str_CMD_Value);

    //读取数据库3F_IR_INPUT中配置绑定的RFID（CAN总线）受控的站点及对应的设备信息
    void Query_RFID_CAN_SetInfor_IO_ByPrg(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP,const QString StrMinPrgNo,const QString StrstationNo);


    //首先查询数据表[3F_IR_STATION_PIN_INPUT]设定的端子针脚分配
    //查询各IO站点对应的IO设定值，也就是拧紧程序号或者零件指示

    void Query_RFID_CAN_SetPIN_IO_Cycle_Start(const QString Str_RFID_No,const QString StrIP,const QString strtemp_stationNO,const QString PIN_Type);
    void Query_RFID_CAN_SetPrg_IO_ByPrg_Cycle_Start(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP,const QString strtemp_stationNO,const QString StrMinPrgNo);

    //匹配输出针脚与程序的关系，产生一个输出模块的程序
    //需要注意复位脚的输出
    void Make_RFID_CAN_Pin_Prg_IO(const QString Str_RFID_No,const QString StrIP,const QString strtemp_stationNO);
    //查找作业完成输入信号脚，复位程序信号脚
    void Make_RFID_CAN_Pin_Finished_Reset_IO(const QString Str_RFID_No,const QString strtemp_stationNO);
    QString Query_Reset_Signal(const QString StrIP,const QString strRFID_NO,const QString strstation_NO,const QString functionNO);

    //联锁循环开始
    void IR_Cycle_Start_IO(const QString strIP,const QString strRFID_NO);
    //根据工序号查询对应工序内的在数据库表3F_IR_RFID_READDATALIST中ID值最小的记录及机型流水号

    QString QueryIR_Olddest_ID(const QString Str_RFID_No,const QString StrIP);
    //读取数据库3F_IR_INPUT中配置绑定的RFID（CAN总线）受控的站点及对应的设备信息
    QString QueryIR_StationNO_From_IR_RUN(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP,const QString Str_StationNO);
    //根据最小ID值，查询最旧的机型流水号
    QString QueryIR_Olddest(const QString StrMinID);//3F_IR_RUN中的模块类型
    void QueryInFor_NOINput_IR_Cycle_Start_IO(const QString strIP,const QString strRFID_NO,const qint32 i);
    //对于没有作业的需要的机型短码,在读取后需要插入一条空记录,以保证正常
    void InsertRecord_IR_Partlist_IO_NOINput(const QString Str_EGShortNo,const QString Str_MINID,const QString Str_RFID_No,const QString StrIP);


    //根据RFID号、站点号、IP值查询对应的在数据库表
    //其实就是要实现先进先出
    QString QueryIR_IO_Userfor(const QString Str_Station_No,const QString Str_RFID_No,const QString StrIP);
    //全部作业完成后初始化站数组和程序数组为0  ,也就是清除站点数据、程序选择数

    void  Clear_Array_PIN_PRG_CAN(const QString IO_Module_Type,qint32 i,qint32 j);
    void Query_RFID_CAN_SetValue_IO(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP,const QString strtemp_stationNO,const QString PIN_Type);

    //根据RFID号、站点号、IP值查询对应的在数据库表3F_IR_RUN中最小的程序号
    //其实就是要实现先进先出
    QString QueryIR_MIN_PrgNO(const QString Str_Station_No,const QString Str_RFID_No,const QString StrIP);
    //单机联锁时，将当前机型和RFID编号保存，供重新启动系统时检索3F_IR_RUN中的信息
    void SaveEGtypeFlow_RFID_NO_ToFile(const QString strEGtypeFlow,const QString strRFID_NO);
    //修改自动线、手动线时的不同参数值
   // void SaveParameter_ToFile();

    QString QueryTypeGroupNO_Cycle_Start(const QString StrEGtype);

    void QueryInFor_HaveINput_IR_Cycle_Start_IO(const QString strIP,const QString strRFID_NO,const qint32 i);

    void Query_IR_Partlist_IO(const QString Str_EGType_Flow,const QString Str_MINID,const QString Str_RFID_No,const QString StrIP);
    QString Query_FinishedOk_Signal(const QString StrIP,const QString strRFID_NO,const QString strstation_NO);

    //发送拧紧程序给各站点
    void SendGunPrgNO_to_IOStation(const QString strIP,qint32 intRFID_NO,const QString strRFID_NO,qint32 intMaxstationNO);
    void Send_StationNO_PrgNO_TO_IO_GunPrgNO_Continue(const qint32 i_rfid_NO,const qint32 int_j,const qint32 Order_NO,const QString str_IR_Station_NO,const QString str_CMD_Value);//根据站号，发送零件指示信息或拧紧程序号
    void Send_StationNO_PrgNO_TO_IO_GunPrgNO(const qint32 i_rfid_NO,const qint32 int_j,const qint32 Order_NO,const QString str_IR_Station_NO,const QString str_CMD_Value);//根据站号，发送零件指示信息或拧紧程序号



    //读写ID
    //向RFID发送指令，以读取ID中相关的信息
    void ReadWrite_RFID_BySpcom(const QString str_RFID_NO,const QString str_CMD_Value);
    //写ID，使用
    QString WriteMDS_OMRON_BySpcom(const QString strAddr,const QString strv);
    //读ID，轴瓦选配用
    QString ReadMDS_OMRON_BySpcom(const QString mdsAddr,const QString MDSLength);
    void  OMRON_RFID_ReadAutoCycle(const qint32 IntRFID_NO);

    //函 数 名：HexToAsc()
    //功能描述：把16进制转换为ASCII
    unsigned char  HexToAsc(unsigned char aChar);
    //函 数 名：AscToHex()
     //功能描述：把ASCII转换为16进制
    unsigned char  AscToHex(unsigned char aHex);

    //16进制\ASII相互转换
    QString  ChangeAreaHEXToStr_QByteArray(QByteArray arr);
    QString  ChangeAreaHEXToStr(const QString arr);
    QString  ChangeAreaStrToHEX(const QString arr);

    //十进制 to 16位的二进制
    QString IntToBin_ok(const qint32 Value,const qint32 Size);

    //扫描正在做的作业进度，结果显示在View上
    void scanJobProgress();
    void refreshJobProgress();

    void readJobList(const QString strIP, const QString strCom, const QString strEGFlowNo);



     //控件SLOT
     void on_eqmid3_lineEdit_textChanged(const QString &arg1);
     void on_eqmid4_lineEdit_textChanged(const QString &arg1);
     void on_reset_pushButton_clicked();


     //公共
     void showError(const QSqlError &err);


     //测试用
     void testQSqlQueryModel();
     void on_testpushButton_clicked();
     void on_forceacc_pushButton_clicked();



     //联锁子函数
     //void InitIR_RFID_PLC(const QString strLINE_ID,const QString strLINE_ID2,const QString InitType);
     void InitIR_RFID_ComArray(const QString InitType);

     void AddState2(const QString state);
     void AddState(const QString state);
     //获得最大的RFID站号 ,整线控制
     qint32 Query_RFID_CAN_MaxStationNO(const QString strIP,const QString InitType);
     qint32 StrToInt(const char *str);
     void Query_RFID_CAN_SetInfor(const QString strIP,const QString InitType);
     void Query_RFID_CAN_MC_NetStationInfor(qint32 TCP_OrderNO,const QString IP);
     void Query_RFID_CAN_MC_NetStationInfor_STOPGO(qint32 TCP_OrderNO,const QString IP);

     bool QueryIR_RFIDSET_LIST_BE(const QString StrIP,const QString strstationNO);
     bool Query_IR_RUN_FIN_IO_BystationNO(const QString StrIP,const QString strstationNO);
     QString Query_RFID_NO(const QString Str_StationNO,const QString StrIP);
     QString Check_Bit_Signal(const QString Str_Hex,const qint32 Strlength);
     QString Str_IntToBin(const qint32 Int_HEX,const qint32 Strlength);


     //复位放行气缸
     void EnableSTOPGO_Reset(qint32 Rfid_NO);
     void EnableSTOPGO_Reset_BYStation_temp(const QString strStation_NO);

     //当系统接受到复位开关信号时的处理事件 ,实际操作时应该是将卡远离卡头，然后碰复位开关
     void IO_ResetCheck_3F_COM(const qint32 StrRFID_NO);
     //当系统接受到复位开关信号时的处理事件 ,实际操作时应该是将卡远离卡头，然后碰复位开关
     void IO_ResetCheck_3F_COM_ForAutoLine_New(qint32 StrRFID_NO);

     QString Make_CAN_CMD(const QString str_IR_Station_NO,const QString str_CMD_Value);
     //向CAN的IO模块发送输出指令 str_IR_Station_NO为IO模块的站号
     void SendCMD_TO_Station_By_CANCOM(const QString str_IR_Station_NO,const QString str_CMD_Value);
     //更新数据表[3F_IR_RFID_READDATALIST] 的字段ALL_FINISHED
     void UPdate_IR_RFID_READDATALIST_ByRFIDNO_IP_Reset(const QString StrRFID_NO,const QString StrIP);

     // void Update_IR_RUN_RE_FIN_IO_ByMAN_or_ByResetInput(const QString StrIP,const QString strRFID_NO, ADOQ1:TADOQuery);
     void Update_IR_RUN_RE_FIN_IO_ByMAN_or_ByResetInput(const QString StrIP,const QString strRFID_NO);

     //显示内容
     void Display_IR_Partlist_DOUBLE(const QString Str_EGShortNo,const QString Str_RFID_No,const QString StrIP);


     void  startOrStopThreadA();
     void  startOrStopThreadB();
     void close();
     void closeEvent(QCloseEvent *event);
     void on_pushButton_quit_clicked();
     void on_pushButton_A_clicked();
     void on_pushButton_B_clicked();

     void on_pushButton_2_clicked();
     void on_ReadNewEGTypeFlow_First_clicked();
     void on_ReadNewEGTypeFlow_Second_clicked();
     void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
