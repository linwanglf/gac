#ifndef COMVAR
#define COMVAR

typedef struct stu{
    char name[20];
    int age;
    char sex;
} STU;

STU stu_test1;

typedef struct ir_type{
        QString Barcode ;//条形码字符串


        QString strYear ; //年变量
        QString strMonth ;  //月变量
        QString strDay ;    //天变量
        QString strBarcode ;// 条形码
        QString strHour ;   //日期变量
        QString strMinute ; //流水号
        QString strSecond ;//临时条形码
        QString strUsername ;//用户名变量
        QString Strpassword ;//密码变量
        QString strTime ;//登录时间变量
        QString strQuanxian ;//权限变量
        QString WorkStation ;//服务器名称
        QString Chuankou ;//串口变量
        QString ZhanHao ;//Sony模块站号
        QString WriteAddr ;//RFID写地址

        QString RFID_FirstPort;    //RFID端口
        QString RFID_FirstBlt ;//RFID波特率
        QString RFID_SecondPort;    //RFID端口
        QString RFID_SecondBlt ;//RFID波特率

        QString Com1_Blt ;//RFID波特率
        QString Com2_Blt ;//RFID波特率
        QString Com3_Blt ;//RFID波特率
        QString Com4_Blt ;//RFID波特率
        QString Com5_Blt ;//RFID波特率
        QString Com6_Blt ;//RFID波特率

        QString Com1_ByteSize ;//RFID波特率
        QString Com2_ByteSize ;//RFID波特率
        QString Com3_ByteSize ;//RFID波特率
        QString Com4_ByteSize ;//RFID波特率
        QString Com5_ByteSize ;//RFID波特率
        QString Com6_ByteSize ;//RFID波特率

        QString Com1_StopBits ;//RFID波特率
        QString Com2_StopBits ;//RFID波特率
        QString Com3_StopBits ;//RFID波特率
        QString Com4_StopBits ;//RFID波特率
        QString Com5_StopBits ;//RFID波特率
        QString Com6_StopBits ;//RFID波特率

        QString Com1_UseFor ;//联锁00物流01
        QString Com2_UseFor ;//联锁00物流01
        QString Com3_UseFor ;//联锁00物流01
        QString Com4_UseFor ;//联锁00物流01
        QString Com5_UseFor ;//联锁00物流01
        QString Com6_UseFor ;//联锁00物流01

        double Cishu ;    //启动次数

        QString JXHaddr ;//机型号MDS地址
        QString JXHMDSLength ;//机型号MDS长度

        QString CSTypeGroupstart ;//曲轴机型分组起始地址
        QString CSTypeGroupLength ;//曲轴机型分组读取长度

        QString Control_APLCIP ;//总控A PLC IP地址
        qint32 Control_APLCStation ;//总控A PLC 站号
        QString Control_APLCStartAddr ;//总控A PLC 开始地址

        QString Control_BPLCIP ;//总控B PLC IP地址
        qint32 Control_BPLCStation ;//总控B PLC 站号
        QString Control_BPLCStartAddr ;//总控B PLC 开始地址

        QString IR_APLCIP ;//总锁A PLC IP地址
        qint32 IR_APLCStation ;//总锁A PLC 站号
        QString IR_APLCStartAddr ;//总锁A PLC 开始地址

        QString IR_BPLCIP ;//总锁B PLC IP地址
        qint32 IR_BPLCStation ;//总锁B PLC 站号
        QString IR_BPLCStartAddr ;//总锁B PLC 开始地址


        QString TuoPan ;//托盘状态PLC地址
        QString LGMGCAddr ;//连杆分组人工确认地址

        QString strTimeAddr ; //缸体上线时间地址
        QString strTimeLength ;//缸体上线时间长度

        QString BarCodeValue ;//读条码的值
        QString RFID_ReadValue_First ;//读条码的值
        QString RFID_ReadValue_Second ;//读条码的值
        QString confirm ;//重号选择确认
        QString IP_ADD ;//当前工序设定的IP地址
        QString IP_ADD_ANDON ;//设定的ANDON相关的PLC的IP地址
        QString Real_Count_D_ANDON ;//设定的ANDON相关的实绩产量地址
        QString Plan_Count_D_ANDON ;//设定的ANDON相关的计划产量地址
        QString StrCheckTotalPLC ;   //是否检测PLC在线，‘01’检测，‘00’不检测
        QString StrReset_Valid ;   //是否启用复位信号，‘01’启用，‘00’不启用
        QString StrReset_DelRecord ;   //是否在有复位信号时清零记录，‘01’清零，‘00’不清零
        QString ControlType ;//当前工控制范围
        QString Control_cclink_can ;//cclink can控制
        QString Control_SoftFunciton ;//01上线，02中间，03最终控制
        QString Control_SoftLoad ;//01上线，02中间，03最终控制
        QString Control_Pick_DO ;//01配膳02作业联锁
        QString Interlock_ManuINPUT ;//01检测人工输入，02不检测人工输入
        QString filepath_01win02linux ;//01windows，02linux

        QString Current_Station_NO_first ;                //当前设定工序（前工序）
        QString Current_Station_NO_second ;                //当前设定工序（后工序）
        QString Current_EGtypeFlow_first ;                //当前设定工序（前工序）
        QString Current_EGtypeFlow_second ;                //当前设定工序（后工序）
        QString Current_EGtypeFlow_Pick ;                //零件指示的当前机型流水
        QString PRI_EGtypeFlow_Pick ;                //零件指示的前一机型流水
        qint32 Control_MaxRFIDNO ;//最大的RFID编号

        QString Sub_Device_First ; //未连接00，是否连接试漏机01或阿特拉斯02或技研的拧紧枪03且需要采集数据
        QString Sub_Device_Second ;//未连接00，是否连接试漏机01或阿特拉斯02或技研的拧紧枪03且需要采集数据
        QString Sub_Device_StationNO_First ;  //连接的站号
        QString Sub_Device_StationNO_Second ; //连接的站号

        QString ReadRFID_NoPosition_First ; //RFID读卡位置与作业位置是否一致
        QString ReadRFID_NoPosition_Second ;
        QString ReadRFID_NoPosition_StationNO_First ;//不一致时对应的站号
        QString ReadRFID_NoPosition_StationNO_Second ;

        QString SubDevice_StartEND_StationNO_First ; //外接设备启动停止对应的站号
        QString SubDevice_StartEND_StationNO_Second ;
        QString SubDevice_OKNGLamp_StationNO_First ;//NGOK指示灯对应的站号
        QString SubDevice_OKNGLamp_StationNO_Second ;
        QString LeakerType ;//试漏机类型
        QString DataTableName ;//试漏机数据存储表名
        QString BlockEnalbed ;//拧紧机升降气缸启动

        QString BlockStationNO ;//拧紧机升降气缸站号
        QString BlockLockStationNO ;//拧紧机升降气缸 锁站号

        QString CycleTime_Arravl_from_IO_StationNO ;//联锁节拍达到站号
        QString ControlforLine_Manu_Auto ;//当前联锁是用在手动线还是自动线
        QString TR_test_cylce ;//变速箱寿命

        QString PickBox_ID_EGType ;//当前料架上的ID机型

        QString Auto_Run ;//自动启动进入工作界面

        QString CH_UPLINE_PARTLAMP_CHECK ;//缸盖分装上线零件指示有效01无效00


        QString PC_DB_IO ;  //使用DB或IO
  } IR_TYPE;

typedef struct cmd_coumunicationtype{

       QString CMD_Display_CBtype ;//0x31更新机型短码指令
       QString CMD_Display_CBtype_RE ;//0x32更新机型短码返回指令
       QString CMD_ACTIVE ;//  	0x11  //PC查询IC是否准备OK
       QString CMD_GETDATA ;//  	  	0x22  //PC向IC查询完成信号状态值
       QString CMD_READY ;//  	  		0x33  //IC回复PC准备OK
       QString CMD_SENDDATA ;//  		0x44  //IC回复完成信号状态值

       QString CMD_SELECTPRGNO ;//  	   0x12  //PC向IC发送程序选择号
       QString CMD_SELECTPRGNOOK ;//  	 0x13  //IC告知PC已经成功选择程序

       QString CMD_RESETVALUE ;//  	   0x14  //PC向IC发送复位完成信号状态值命令
       QString CMD_RESETVALUEOK ;//  	 0x15  //IC回复PC收到复位值的操作命令完成

       QString CMD_DATASHOW ;//  	   0x16  //PC向IC发送复发动机型号数据
       QString CMD_REDATASHOW ;//  	 0x17 //IC回复PC发动机型号数据接收成功
       QString CMD_ID1_485 ;//  	 AA 		// 数据帧识别符1
       QString CMD_ID2_485 ;//  	 8A			// 数据帧识别符2
       QString CMD_IDSend_CAN ;//  	 55 		// 数据帧识别符1
       QString CMD_IDRec_CAN ;//  	 AA			// 数据帧识别符2
       QString CMD_IDRE232_CAN ;//  	 ACAA			// 数据帧回复从232口获取数据
       QString CMD_RECFRMMAXLEN ;//  	 16 		// 接收帧的最大长度，超过此值认为帧超长错误
       QString CMD_STATUSMAXLEN ;//  	 1			// 设备状态信息最大长度
       QString CMD_DATALEN ;//  	 9			// 当前项目数据长度
       QString CMD_ANDON_IOLamp ;//  	   18  //PC向IC发送复安东报警工序号，以点亮灯
       QString CMD_ANDON_IOLamp_RE ;//  	 19 //IC回复PC发动机型号数据接收成功


       QString CMD_INCValue ;//充值指令
       QString CMD_INCValue_RE ;//充值操作返回指令
       QString CMD_READ ;//读指令

       QString CMD_HAND ;
       QString CMD_USERPASSWORD ;
       QString CMD_3FPASSWORD ;
       QString CMD_3FLODADATE ;
       QString CMD_HAND_RE ;
       QString CMD_USERPASSWORD_RE ;
       QString CMD_3FPASSWORD_RE ;
       QString CMD_WRITETOFLASH_Sub_RE ;
       QString CMD_3FLODADATE_RE ;

    }CMD_COUMUNICATIONTYPE;


typedef struct ir_iotype
{
  QString IR_IO_Pin_Output_State;         //IO模块针脚号 IR_IO_Pin[i][j] i是站号，j是针脚号
  QString IR_IO_Pin_Output_State_Reset;         //IO模块针脚号 IR_IO_Pin[i][j] i是站号，j是针脚号

  QString IR_IO_Pin_Input_State;
  QString IR_IO_Pin_Input_length;  //IO模块的针脚数
  QString IR_IO_Pin_Output_length;  //IO模块的针脚数
  QString IR_IO_Module_Type;  //IO模块的类型（16输入-01，8入8出-02，16出-03）
  QString IR_MAP_StationNO;//绑定的RFID的站点
  QString IR_IO_Station_NO; //对应的IO站号
  QString IR_IO_Actived_Station_NO;//当前零件指示的有效站
  QString IR_IO_Userfor;//（拧紧作业联锁-01，零件指示-02，安东显示-03）
  QString Prg_NO_Bin;//当前RFID控制的有效站对应拧紧程序号2进制
  QString Prg_NO_Hex;//当前RFID控制的有效站对应拧紧程序号16进制
  QString strIR_IPUT_PIN[17];//输入针脚端子设定的功能代码
  QString strIR_OPUT_PIN[17]; //输出针脚端子设定的功能代码
  QString strIR_IPUT_PIN0[17];//输入针脚端子设定的功能描述
  QString strIR_OPUT_PIN0[17];//输出针脚端子设定的功能描述


  QString IR_IO_Pin_Reset;//复位脚
  QString IR_IO_Pin_Finished;//作业完成脚
  QString MINID_IR_RUN_Station;//对应IO站最小的记录号
  QString MINPrg;//对应IO站最小的程序号

  QString IR_MAP_PLC_D;//对应IO站最小的记录号
}IR_IOTYPE;

typedef struct ir_stopgotype
{
QString STOPGO_Pin_Output_PinNo;         //IO模块针脚号 IR_IO_Pin[i][j] i是站号，j是针脚号
QString STOPGO_Pin_Input_PinNo;

QString STOPGO_Station_NO; //对应的IO站号
QString STOPGO_Userfor;//
bool STOPGO_Eanbled;//
QString STOPGO_State;//'1'许可放行但未放行，'2'已经放行 ，'3'不能放行
}IR_STOPGOTYPE;

typedef struct ir_can_rfidtype
{
  QString IR_RFID_ReadDATA; //RFID读取的机型流水号
  QString Fram_Rfid_Data[4];//CAN总线读取RFID时有4帧，才能组合完成全部数据
  QString Fram_Rfid_Data_Rev;//CAN总线读取RFID全部数据
  qint32 Fram_Rfid_Data_Count;//帧总数
  QString Fram_Rfid_Reset;//帧总数
  QString MIN_ID;//RFID读取的机型流水号中最旧的记录，以实现先进先出
  bool MIN_ID_YES;
  QString EGType_Flow;//RFID读取的机型流水号中最旧的记录，以实现先进先出
  QString ShortEGType;//
  QString IR_RFID_CurrentRFID_NO; //当前RFID对应的工序号
  bool IR_Start; //联锁循环开始
  bool IR_Finished; //联锁循环完成
  qint32 IR_CAN_MaxRFIDNo; //许可连接的最大RFID数
  bool IR_Send_Prg_Select_Finished;//程序选择发送完成，为TRUE时许可查询拧紧完成情况
  QString Actived_Station_NO[32];//当前RFID控制的有效站
  QString Prg_NO[32][16];//当前RFID控制的有效站对应拧紧程序号
  QString Prg_NO_HaveSend[32];//当前RFID控制的有效站对应刚发送完毕的程序号
  bool Prg_NO_HaveFinished[32][16];//当前RFID控制的有效站对应刚发送完毕的程序号
  qint32 IR_CANLINE; //PC许可连接的CAN总线数
  QString IR_RFID_ReadSta; //RFID处于读卡状态
  QString IR_RFID_WriteSta; //RFID处于写卡状态
  QString IR_RFID_ReadData_Only; //对应的RFID读取的值唯一性标识

  QString RFID_Station_NO; //对应的RFID站号
  QString RFID_LINE_NAME; //对应的RFID站号的生产线名称
  QString RFID_MACHINE_NAME; //对应的RFID站号的设备名称
  QString RFID_IP_ADD; //对应的RFID站号的设备IP地址
  QString RFID_CANLine_NO; //对应的RFID站号的CAN总线编号
  QString RFID_LINE_ID; //对应的RFID站号的生产线ID
  QString RFID_LINE_ID2; //对应的RFID站号的生产线ID下的分装ID
  qint32 RFID_IP_Station_NO; //对应的RFID站号对应设备的IP站号
  QString RFID_IP_NET_NO ; //对应的RFID站号对应设备的IP网络号
  QString RFID_IP_Group_NO ; //对应的RFID站号对应设备的IP网络号
  QString RFID_IP_Arr_NO ; //对应的数组序号
  qint32 RFID_FDJTypeState; //对应的PLC有发动机请求
  QString RFID_EGType_Daddr ; //存储机型的开始D地址
  QString RFID_PLCRequestEGType_Daddr; //PLC请求更新机型的开始D地址
  QString RFID_Oddest_EGtypeFlow;//在读取的记录中[3F_IR_RFID_READDATALIST]最老的机型记录
  QString RFID_Oddest_ID;//在读取的记录中[3F_IR_RFID_READDATALIST]最老的机型记录
  QString RFID_EGType_Daddr_Length; //存储机型的D地址 长度
  QString RFID_EGFlow_Daddr; //存储流水的开始D地址
  QString RFID_EGFlow_Daddr_Length ; //存储流水的D地址 长度
  QString RFID_Data_InputPin_State;//IO板子输入状态值
  QString RFID_Data_OutputPin_State;//IO板子输出状态值
  QString RFID_IO_Init;//IO板子输出初始化
  bool RFID_WriteEGtypeFLOW_TO_PLC;//将机型、流水、分组号发送给PLC
  QString IR_RFID_TuopanArraved;//托盘到位
  QString RFID_ReadID_Finished[2];//RFID读卡完成信号，'1'表示完成，'2'未完成（主要是用在CCLINK方案中）
  QString RFID_SendToPLC_IP;//发送机型对应的PLC的IP
  QString RFID_EGTypeGroupvalue; //对应的RFID站号

  QString IR_IO_Type;         //模块类型（IO、232、）
  QString IR_IO_Pin;
  //绑定的IO站总数,即受控于当前RFID的从站数，此数量从[IR_RUN]数据表中统计读取
  qint32 RFID_CAN_MaxMAPStationNO;
  IR_IOTYPE *IR_Belong_IO_Arr;//数组
  IR_STOPGOTYPE IR_Belong_STOPGO;
  QString HAVESENDPRG_TOIO;

  QString IR_Belong_PROCESSNO;

  QString IR_MAP_PLC_D_Arrival;//托盘到位D
  QString IR_MAP_PLC_D_Reset;//托盘离开D
  QString IR_MAP_PLC_D_Fin;//作业完成D
  QString IR_PLC_D_EGTypeGroupNO;//作业完成D

  QString IR_CycleTimeOver;//节拍到达
  QString IR_CycleTimeOver_alramsta;//节拍到达报警状态

  QString IR_PARTlist_NO;//零件号,E看板使用
  qint32 IR_PARTlist_Count;//当前IP和RFID管亥的站总数,也就是零件总数,E看板使用
}IR_CAN_RFIDTYPE;


typedef struct ir_rfidtype
{

  QString IR_RFID_ReadDATA; //RFID读取的机型流水号
  QString Fram_Rfid_Data[4];//CAN总线读取RFID时有4帧，才能组合完成全部数据
  QString MIN_ID;//RFID读取的机型流水号中最旧的记录，以实现先进先出
  bool MIN_ID_YES;
  QString EGType_Flow;//RFID读取的机型流水号中最旧的记录，以实现先进先出
  QString IR_RFID_CurrentRFID_NO; //当前RFID对应的工序号
  bool IR_Start; //联锁循环开始
  bool IR_Finished; //联锁循环完成
  qint32 IR_MaxRFIDNo; //许可设定的最大工序号
  bool IR_Send_Prg_Select_Finished;//程序选择发送完成，为TRUE时许可查询拧紧完成情况
  QString Actived_Station_Total;//该RFID所对应的IO站的总数（其值取最大的站号）
  QString Actived_Station_NO[32];//当前RFID控制的有效站
  QString Prg_NO[32][16];//当前RFID控制的有效站对应拧紧程序号
  QString Prg_NO_HaveSend[32];//当前RFID控制的有效站对应刚发送完毕的程序号
  bool Prg_NO_HaveFinished[32][16];//当前RFID控制的有效站对应刚发送完毕的程序号
  QString IR_RFID_TuopanArraved; //托盘到位检测开关信号
  bool IR_RFID_ReadSta; //RFID处于读卡状态
  bool IR_RFID_WriteSta; //RFID处于写卡状态
  QString IR_RFID_Type;//RFID厂家
  QString IR_RFID_Type_Init;//RFID厂家   打开
  QString IR_RFID_CANStation_NO;//RFID的对应的CAN站点编号
  QString IR_IR_ENGType;//当前正在进行联锁作业的发动机型号
}IR_RFIDTYPE;

IR_RFIDTYPE IR_RFID;//
IR_TYPE IR_Infor;    //系统背景文件名 20121219联锁系统用
IR_RFIDTYPE IR_RFID_First;//前工序
IR_RFIDTYPE IR_RFID_Second;//后工序
CMD_COUMUNICATIONTYPE CMD_COUMUNICATION ;//通信指令
IR_CAN_RFIDTYPE *IR_CAN_RFID_Arr;//CAN总线上的RFID专用,数组



//定义动态数组,如果要真正使用时需要用recData_fromICLst_CAN_RFID_COM =new Qstring[s];
QString *recData_fromICLst_CAN_RFID_COM;//recData_fromICLst_CAN_RFID_COM:array of  Tstrings;//数组

qint32 RFID_CAN_MaxStationNO;


#endif // COMVAR

