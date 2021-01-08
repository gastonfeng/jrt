/*--------------------------------------*/
/*  JRT-xx-xx系列太阳能智能充电控制器   */
/*  主程序宏定义文件                    */
/*  文件名: JRT.H                       */
/*  CPU型号:  89C52                     */
/*  程序设计: 冯建涛                    */
/*  开始日期: 1997年1月1日              */
/*  1998年5月增加打印功能               */
/*--------------------------------------*/

/*------------------*/
/*  控制器型号定义  */
/*------------------*/
#define MODEL 70

/*----------------------*/
/*      键名定义        */
/*----------------------*/

#define VIEW      	0       /*      键空值          */
#define SOLAR    	1       /*      蓄电池电压      */
#define BT_V_KEY        2       /*      蓄电池充电电流  */
#define BT_I_KEY        4       /*      负载电压        */
#define LOAD_V_KEY      3       /*      太阳电池        */
#define LOAD_I_KEY      5       /*      控制设置        */
#define CONTROL_SET     7       /*      负载电流        */
#define REMOTE_SET      6       /*      自检            */
#define LOAD_SET        9       /*      通讯设置        */
#define TESTSELF        8       /*      负载设置        */
#define OK              10      /*      历史数据察看    */
#define INC             11      /*      加              */
#define DEC             12      /*      减              */
#define CANCEL          13
#define F1              14
#define F2              15
#define NO              16
/*------------------------------*/
/*      设置变量范围定义        */
/*------------------------------*/
#if MODEL==70
#define BOOST_MAX       680
#define BOOST_MIN       540
#define TAPER_MAX       680
#define TAPER_MIN       520
#define BANK_MAX        6
#define BANK_MIN        1
#define FLOAT_MAX       600
#define FLOAT_MIN       480
#define BOOST_BELOW_MAX 600
#define BOOST_BELOW_MIN 480
#define COMP_TEMP_MAX   20
#define COMP_TEMP_MIN   0
#define DELAY_CHANGE_MAX 360
#define DELAY_CHANGE_MIN 6
#define LCD_TEMP_MAX    70
#define LCD_TEMP_MIN    0
#define BT_MIN_MAX      480
#define BT_MIN_MIN      420
#define BT_MAX_MIN      560
#define BT_MAX_MAX      720
#define BT_RELOAD_MIN   420
#define BT_RELOAD_MAX   600
#define DELAY_DIS_MAX   250
#define DELAY_DIS_MIN   1
#endif

#if MODEL==71
#define BOOST_MAX       340
#define BOOST_MIN       270
#define TAPER_MAX       340
#define TAPER_MIN       260
#define BANK_MAX        6
#define BANK_MIN        1
#define FLOAT_MAX       300
#define FLOAT_MIN       240
#define BOOST_BELOW_MAX 300
#define BOOST_BELOW_MIN 240
#define COMP_TEMP_MAX   20
#define COMP_TEMP_MIN   0
#define DELAY_CHANGE_MAX 360
#define DELAY_CHANGE_MIN 6
#define LCD_TEMP_MAX    70
#define LCD_TEMP_MIN    0
#define BT_MIN_MAX      240
#define BT_MIN_MIN      210
#define BT_MAX_MIN      280
#define BT_MAX_MAX      360
#define BT_RELOAD_MIN   210
#define BT_RELOAD_MAX   300
#define DELAY_DIS_MAX   250
#define DELAY_DIS_MIN   1
#endif

/*----------------------*/
/*      变量缺省值定义  */
/*----------------------*/
#if MODEL==70
#define BOOST_DEFAULT_VALUE     600
#define TAPER_DEFAULT_VALUE     560
#define FLOAT_MAX_DEFAULT_VALUE 560
#define FLOAT_MIN_DEFAULT_VALUE 540
#define DELAY_CHANGE_DEFAULT_VALUE      60
#define BOOST_BELOW_DEFAULT_VALUE       496
#define COMP_TEMP_DEFAULT_VALUE         0
#define BANKS_DEFAULT_VALUE             6
#define LCD_TEMP_DEFAULT_VALUE          35
#define BT_MIN_DEFAULT_VALUE            448
#define BT_MAX_DEFAULT_VALUE            640
#define BT_RELOAD_DEFAULT_VALUE         512
#define DELAY_DISCONNECT_DEFAULT_VALUE  200
#endif

#if MODEL==71
#define BOOST_DEFAULT_VALUE     300
#define TAPER_DEFAULT_VALUE     280
#define FLOAT_MAX_DEFAULT_VALUE 280
#define FLOAT_MIN_DEFAULT_VALUE 270
#define DELAY_CHANGE_DEFAULT_VALUE      60
#define BOOST_BELOW_DEFAULT_VALUE       248
#define COMP_TEMP_DEFAULT_VALUE         0
#define BANKS_DEFAULT_VALUE             6
#define LCD_TEMP_DEFAULT_VALUE          35
#define BT_MIN_DEFAULT_VALUE            224
#define BT_MAX_DEFAULT_VALUE            320
#define BT_RELOAD_DEFAULT_VALUE         256
#define DELAY_DISCONNECT_DEFAULT_VALUE  200
#endif

/*------------------------------*/
/*      EPROM存储序列定义       */
/*------------------------------*/
#define TAPER           1
#define BOOST           6
#define FLOAT_MAX_S     7
#define FLOAT_MIN_S     8
#define BOOST_BELOW     9
#define COMP_TEMP       10
#define DELAY_CHANGE    11

#define BT_MIN          12
#define BT_RELOAD       13
#define BT_MAX          14
#define DELAY_DISCONNECT 15

#define LOCAL           16
#define HOUR            17
#define MINUTE          18
#define AUTO_UPLOAD     19
#define UP_HOUR         20
#define UP_MIN          21
#define PHONE           22

#define CLIP            35

#define PASSWORD        55

#define SYSTEM_FLAG     64
#define DAY             65

#define DATA_INDEX      66
#define DATA            67

/*----------------------*/
/*      光标设置        */
/*----------------------*/
#define ON      1
#define OFF     0

/*----------------------*/
/*      EPROM操作代码   */
/*----------------------*/
#define EWEN    0x01
#define EWDS    0
#define ERAL    0x0300
#define ERASE   0x0200

/*------------------------------*/
/*      A/D操作包含文件         */
/*      文件名: PROCESS.H       */
/*------------------------------*/

#define BT_V            0
#define SR_V0           1
#define SR_V1           2
#define SR_V2           3
#define SR_V3           4
#define SR_V4           5
#define SR_V5           6
#define LOAD_V          7
#define TEMP_BT         8
#define TEMP_LCD        9
#define NC1             10
#define NC2             11
#define DOOR            12
#define YAN             13
#define LOAD_I          14
#define BT_I            15
#define SR_I0           0x10
#define SR_I1           0x11
#define SR_I2           0x12
#define SR_I3           0x13
#define SR_I4           0x14
#define SR_I5           0x15

#define ALARM_HI        0x1
#define ALARM_LO        0x2
#define LOAD_DISABLE    0x4
#define UJ              0x8

#define LAMP            0x10
#define bank0           0x1
#define bank1           0x2
#define bank2           0x4
#define bank3           0x8
#define bank4           0x10
#define bank5           0x20
#define LOAD            0X40

/*MODEM CODE*/
#define CONNECT         '1'
#define NO_CARRIER      '3'
#define NowData         10
#define OVER            '5'
#define HistoryData     72
#define SetData         73
#define SendSetData     74

#define LCD     1
#define PRINTER 0
