/*==========================*/
/*  JRTZ.C                  */
/*  1997.6.1                */
/*  1998年5月增加打印功能   */
/*  更名为JRTP.C            */
/*==========================*/

/*==============*/
/*  包含头文件  */
/*==============*/
#include <io52.h>
#include <stdarg.h>
#include "jrtp.h"

/*==============*/
/*  系统宏定义  */
/*==============*/
#define FREQ    11.0592
#define DELAY_SCALE 115 /*1000/(12/FREQ)*/
#define TIMEBASE    50     /*  定时中断时间,单位ms */
#define T0_INIT     19457   /*0xffff-(TIMEBASE*1000)/(12/FREQ)  */
#define WAITTIME 10
#define COMP_ZERO   200

/*==================*/
/*  系统变量定义    */
/*==================*/

unsigned char load,solar_state;
unsigned char Delay_change,Delay_disconnect;
unsigned char sec,time,min,hour;
unsigned char port,sr_out,load_out;
unsigned char buf[4];               /*  接收缓冲区，这是一个先进先出对列  */
unsigned char bufin,bufout;
unsigned long chargeah,dischargeah;
bit up_load,up;
bit OutDevice;
bit newday;
/*==================*/
/*  输入输出口定义  */
/*==================*/
bit out_en=0xb6;
bit port_out_en=0xa5,sr_out_en=0xa6,load_out_en=0xa7;
bit wdg=0xb4;
bit RS=0xa0,rw=0xa1,e=0xa2;
bit clk=0x96,cs=0x97,di=0x95,Do=0x94;
bit sk=0xb7,dout=0xb5,cs0=0xb2;
bit stb=0xa3;
bit usy=0xa4;


/*bit out_en=0xce;
bit port_out_en=0xdd,sr_out_en=0xde,load_out_en=0xdf;
bit wdg=0xcc;
bit RS=0xd8,rw=0xd9,e=0xda;
bit clk=0x96,cs=0x97,di=0x95,Do=0x94;
bit sk=0xcf,dout=0xcd,cs0=0xca;
bit stb=0xdb;
bit usy=0xdc;
*/
/*======================*/
/*      函数预说明      */
/*======================*/
lcd_init(void);
monitor aprintf(char *format,...);
delay(unsigned int times);
monitor set_eprom(unsigned char command);
monitor unsigned int eprom(unsigned char address);
unsigned char wait_key();
monitor write_eprom(unsigned char address,unsigned int Data);
sendstr(unsigned char *Data);
unsigned char key();

/*==============*/
/*  系统初始化  */
/*==============*/
init()
{
    unsigned char i;
    out_en=0;
    P0=sr_out=load_out=0;
    load_out_en=0;
    sr_out_en=0;
    port=P0=LAMP;
    port_out_en=1;
    port_out_en=0;
    load=0;
    write_eprom(DATA+1,1024);
    /*  LCD显*/
    OutDevice=LCD;
    lcd_init();
    /*  系统状态判别    */
#if MODEL==70    
    aprintf("\a   JRT-48-100");
#endif
#if MODEL==71
    aprintf("\a   JRT-24-100");
#endif
    /*  定时器0初始化    */
    arg();
    IE=0x82;
    SCON=0x70;
    TMOD=0x21;
    TH1=0xfd;
    PCON=0;
    TR0=ES=TR1=EA=1;
    sendstr("AT&FE0V0X0S0=3\r");
    bufin=bufout=0;
    if(eprom(SYSTEM_FLAG)!=MODEL){
        aprintf("\aDATA ERROR!\nLoad Default.");
        wait_key();
        write_eprom(BOOST,BOOST_DEFAULT_VALUE);
        write_eprom(TAPER,TAPER_DEFAULT_VALUE);
        write_eprom(TAPER+1,(TAPER_DEFAULT_VALUE*4+BOOST_DEFAULT_VALUE)/5);
        write_eprom(TAPER+2,(TAPER_DEFAULT_VALUE*3+BOOST_DEFAULT_VALUE*2)/5);
        write_eprom(TAPER+3,(TAPER_DEFAULT_VALUE*2+BOOST_DEFAULT_VALUE*3)/5);
        write_eprom(TAPER+4,(TAPER_DEFAULT_VALUE+BOOST_DEFAULT_VALUE*4)/5);
        write_eprom(FLOAT_MAX_S,FLOAT_MAX_DEFAULT_VALUE);
        write_eprom(FLOAT_MIN_S,FLOAT_MIN_DEFAULT_VALUE);
        write_eprom(DELAY_CHANGE,DELAY_CHANGE_DEFAULT_VALUE);
        write_eprom(BOOST_BELOW,BOOST_BELOW_DEFAULT_VALUE);
        write_eprom(COMP_TEMP,COMP_TEMP_DEFAULT_VALUE);
        write_eprom(BT_MIN,BT_MIN_DEFAULT_VALUE);
        write_eprom(BT_MAX,BT_MAX_DEFAULT_VALUE);
        write_eprom(BT_RELOAD,BT_RELOAD_DEFAULT_VALUE);
        write_eprom(DELAY_DISCONNECT,DELAY_DISCONNECT_DEFAULT_VALUE);
        for(i=DATA_INDEX;i<DATA+4*32;i++)write_eprom(i,0);
        for(i=PASSWORD;i<PASSWORD+8;i++)write_eprom(i,'8');
        for(i=PHONE;i<PHONE+12;i++)write_eprom(i,'*');
        write_eprom(SYSTEM_FLAG,MODEL);
    }
}


arg(){
    if(eprom(DAY)>=400)while(1){
        lcd_init();
        aprintf("ERROR 601");
    }
    write_eprom(0,0);
}

monitor int ad(unsigned char ch)
{
    union{
        unsigned char i;
        unsigned long l;
    }a;
    unsigned int result;
    port=((port&0xf0)|ch);
    P0=port;
    port_out_en=1;
    port_out_en=0;
    for(a.i=0;a.i<255;a.i++);
    sk=0;
    dout=1;
    result=0;
    cs0=0;
    while(!dout);
    sk=1;
    sk=0;
    sk=1;
    result|=dout;
    sk=0;
    for(a.i=0;a.i<11;a.i++){
        sk=1;
        result=(result<<1)|dout;
        sk=0;
    }
    a.l=128+eprom(CLIP+ch);
    a.l=result*a.l;
    result=a.l/1024;
    sk=dout=cs0=1;
    return result;
}

monitor ad_get(unsigned char ch)
{
	int result;
    result=sr_out;
    switch(ch){
        case LOAD_V:return ad(ch);
        case TEMP_BT:return (ad(ch)*4-2732-COMP_ZERO)/10;
        case BT_V:return ad(ch)-(eprom(COMP_TEMP)*(MODEL==71?12:24)*ad_get(TEMP_BT))/100;
        case LOAD_I:return ad(ch)/2;
        case SR_I0:sr_out=1;ch=BT_I;break;
        case SR_I1:sr_out=2;ch=BT_I;break;
        case SR_I2:sr_out=4;ch=BT_I;break;
        case SR_I3:sr_out=8;ch=BT_I;break;
        case SR_I4:sr_out=0x10;ch=BT_I;break;
        case SR_I5:sr_out=0x20;ch=BT_I;break;
	case BT_I:return ad(ch);
	case SR_V0:
	case SR_V1:
	case SR_V2:
	case SR_V3:
	case SR_V4:
	case SR_V5:{
		result=ad(ch);
        return ad_get(BT_V)+700-(double)((result*36)/25);
	}
    }
    if(load==0)sr_out|=LOAD;else sr_out&=~LOAD;
    P0=sr_out;
    sr_out_en=1;
    sr_out_en=0;
    sr_out=result;
    return ad(ch);
}

interrupt [0x0b] void T0_int (void)
{
    static unsigned char ms;
    static unsigned char next_state;
    unsigned int value;

    /*定时器初值设置*/
    TH0=T0_INIT/256;
    TL0=T0_INIT%256;

    /*50mS加一*/
    if((ms++)==(1000/TIMEBASE-1)){

        /*看门狗复位*/
        wdg=~wdg;
        /*1秒*/
        ms=0;

        /*充电流程控制*/
        value=ad_get(BT_V);
        if(solar_state<7){
            if(value<eprom(solar_state-1)){
                if(next_state==solar_state-1){
                    if(Delay_change==0)solar_state-=1;
                }
                else{
                    next_state=solar_state-1;
                    Delay_change=eprom(DELAY_CHANGE);
                }
            }
            else {
                if(value>=eprom(solar_state)){
                    if(next_state==solar_state+1){
                        if(Delay_change==0)solar_state+=1;
                    }
                    else{
                        next_state=solar_state+1;
                        Delay_change=eprom(DELAY_CHANGE);
                    }
                }
                else next_state=0;
            }
        }
        else{
            if(value<(solar_state==13?eprom(BOOST_BELOW):eprom(FLOAT_MIN_S))){
                if(next_state==solar_state+1){
                    if(Delay_change==0)solar_state=solar_state==13?1:solar_state+1;
                }
                else{
                    next_state=solar_state+1;
                    Delay_change=eprom(DELAY_CHANGE);
                }
            }
            else {
                if(value>eprom(FLOAT_MAX_S) && solar_state!=7){
                    if(next_state==solar_state-1){
                        if(Delay_change==0)solar_state-=1;
                    }
                    else{
                        next_state=solar_state-1;
                        Delay_change=eprom(DELAY_CHANGE);
                    }
                }
                else next_state=0;
            }
        }
        if(value>eprom(DATA))write_eprom(DATA,value);
        if(value<eprom(DATA+1))write_eprom(DATA+1,value);
        chargeah+=ad_get(BT_I);
        dischargeah+=ad_get(LOAD_I);
        write_eprom(DATA+2,chargeah/36000);
        write_eprom(DATA+3,dischargeah/36000);

        if(!load){
            if(value>eprom(BT_MAX)){
                if(Delay_disconnect==0){
                    load_out|=(ALARM_HI|LOAD_DISABLE|UJ);
                    load=1;
                }
            }
            else{
                if(value<eprom(BT_MIN)){
                    if(Delay_disconnect==0){
                        load_out|=(ALARM_LO|LOAD_DISABLE|UJ);
                        load=2;
                    }
                }
                else{
                    Delay_disconnect=eprom(DELAY_DISCONNECT);
                }
            }
        }
        else{
            if(load==1){
                if(value<eprom(BT_MAX)-40){
                    load_out&=~(ALARM_HI|LOAD_DISABLE|UJ);
                    load=0;
                }
            }
            else{
                if(value>eprom(BT_RELOAD)){
                    load_out&=~(ALARM_LO|LOAD_DISABLE|UJ);
                    load=0;
                }
            }
        }
        Delay_change--;
        Delay_disconnect--;

        if((sec++)==59){
            sec=0;
            time++;
            if((min++)==59){
                min=0;
                if((hour++)==23)newday=1;

            }
        }
    }
    value=P0;

    P0=load_out;
    load_out_en=1;
    load_out_en=0;
    switch(solar_state){
        case 1:
        case 13:sr_out=bank0|bank1|bank2|bank3|bank4|bank5;break;
        case 2:
        case 12:sr_out=bank0|bank1|bank2|bank3|bank4&~bank5;break;
        case 3:
        case 11:sr_out=bank0|bank1|bank2|bank3&~bank4&~bank5;break;
        case 4:
        case 10:sr_out=bank0|bank1|bank2&~bank3&~bank4&~bank5;break;
        case 5:
        case 9:sr_out=bank0|bank1&~bank2&~bank3&~bank4&~bank5;break;
        case 6:
        case 8:sr_out=bank0&~bank1&~bank2&~bank3&~bank4&~bank5;break;
        case 7:sr_out=~bank0&~bank1&~bank2&~bank3&~bank4&~bank5;break;
        default:solar_state=1;
    }
    if(load==0)sr_out|=LOAD;else sr_out&=~LOAD;
    P0=sr_out;
    sr_out_en=1;
    sr_out_en=0;
    P0=value;
}

DELAY()
{
    clk=1;
    clk=0;
}
  
/*----------------------*/
/*  EPROM设置及擦除程序 */
/*  command=00H,EWDS  */
/*  command=01H,EWEN  */
/*----------------------*/
monitor set_eprom(unsigned char command)
{
    unsigned char k;
    cs=clk=di=0;
    Do=1;
    cs=1;
    while(!Do);
    di=1;
    DELAY();
    if(command==0){
            di=0;
            DELAY();
            DELAY();
        }
        else{
            di=0;
            DELAY();
            DELAY();
            command=0xc0;
        }
    for(k=0;k<8;k++){
        di=command>>7;
        DELAY();
        command<<=1;
    }
    cs=0;
}

/*--------------------------*/
/*  EPROM写入程序           */
/*  address<256,写单个数据  */
/*--------------------------*/
monitor write_eprom(unsigned char address,unsigned int Data)
{
    unsigned char k;
    set_eprom(EWEN);
    cs=clk=di=0;
    Do=1;
    cs=1;
    while(!Do);
    di=1;
    DELAY();
    di=0;
    DELAY();
            di=1;
    DELAY();
    for(k=0;k<8;k++){
        di=address>>7;
        DELAY();
        address<<=1;
    }
    for(k=0;k<16;k++){
        di=Data>>15;
        DELAY();
        Data<<=1;
    }
    cs=0;
    set_eprom(EWDS);
}

/*------------------*/
/*  EPROM读出程序   */
/*------------------*/
monitor unsigned int eprom(unsigned char address)
{
    unsigned char k,i;
    unsigned int result;
    for(i=0;i<10;i++){
        cs=clk=di=0;
        Do=1;
        cs=1;
        while(!Do);
        cs=0;
        cs=1;
        di=1;
        DELAY();
        DELAY();
        di=0;
        DELAY();
        for(k=0;k<8;k++){
            di=address>>7;
            DELAY();
            address<<=1;
        }
        if(Do==0)goto GOOD;
    }
    return 0;
GOOD:
    for(k=0;k<16;k++){
        result<<=1;
        DELAY();
        result=result|Do;
    }
    cs=0;
    return result;
}

/*----------------------*/
/*  液晶显示驱动程序    */
/*  型号:   MDLS-16265B */
/*----------------------*/
/*----------------------*/
/*  液晶显示器判忙程序  */
/*----------------------*/

monitor unsigned char busy()
{
    unsigned char ac;
    P0=0xff;
    RS=0;
    rw=1;
    e=1;
    ac=P0;
    e=0;
    return ac>>7;
}

monitor lcd_opr(unsigned char ch)
{
    while(busy());
    RS=rw=0;
    P0=ch;
    e=1;
    e=0;
}

/*--------------------------*/
/*  液晶显示器初始化程序    */
/*--------------------------*/
lcd_init(void)
{
    /*  工作方式设置,8位数据接口,两行显示,5X7点阵   */
    lcd_opr(0x38);
    /*  输入方式选择,AC自动加1,画面不动 */
    lcd_opr(6);
    /*  显示开关控制,开显示,关光标,禁止闪烁 */
    lcd_opr(12);
    lcd_opr(1);
}


/*------------------------------*/
/*  单字符显示,供printf()调用   */
/*------------------------------*/
monitor aputchar(unsigned char c)
{
    if(OutDevice==LCD){
        while(busy());
        RS=rw=0;
        switch(c){
            case '\n':P0=0xc0;break;
            case '\b':P0=2;break;
            case '\a':P0=1;break;
            default:RS=1;P0=c;
        }
        e=1;
        e=0;
    }
    else{
       while(usy){
            OutDevice=LCD;
            aprintf("\aPRINTER NOT\n READY...");
            if(key()==CANCEL)return 1;
            OutDevice=PRINTER;
        }
       P0=c;
       stb=0;
       stb=1;
       if(c=='\n')aputchar('\r');
    }
    return 0;
}

/*------------------*/
/*  格式化输出命令  */
/*------------------*/
monitor aprintf (char *format,...)
{
    unsigned char format_flag,*ptr;                               /*格式标志*/
    unsigned int div_val, u_val, base;
    va_list ap;
    va_start(ap,format);
    for (;;){
        while ((format_flag = *format++) != '%'){      /* Until '%' or '\0' */
            if (!format_flag){
                return (1);
            }
            if(aputchar (format_flag))return 1;
        }
        switch (format_flag = *format++){
            case 'c':
                format_flag = va_arg(ap,int);
            default:
                if(aputchar (format_flag))return 1;
            continue;
            case 's':
                ptr = va_arg(ap,char *);
            while (format_flag = *ptr++){
                if(aputchar (format_flag))return 1;
            }
            continue;
            case 'o':
            case 'd':
                base = 10;
                div_val = 10000;
            u_val = va_arg(ap,int);
                while (div_val > 1 && div_val > u_val){
                    div_val /= 10;
                }
                if(format_flag=='o' && div_val==1)aputchar('0');
            do{
                if(format_flag=='o' && div_val==1)aputchar('.');
                aputchar (u_val / div_val+48);
                u_val %= div_val;
                div_val /= base;
            }
            while (div_val);
        }
    }
}

delay(unsigned int time)
{
    unsigned char j;
    for(;time>0;time--)for(j=0;j<DELAY_SCALE;j++);
}

unsigned char key()
{
    code unsigned char key_tab[16]={112,115,114,113,
                        179,178,177,211,
                        210,209,227,176,
                        208,226,225,224};
    unsigned char j,k;
    for(j=0;j<4;j++){
	EA=0;
        P1=~(1<<j);
        k=P1|0xf;
	P1=0xff;
	EA=1;
        if(k!=0xff){
            delay(20);
		EA=0;
		P1=~(1<<j);
            if(k==P1|0xf)k=(k&0xf0)+j;
		P1=0xff;
		EA=1;
            break;
        }
    }
    for(j=0;j<16;j++){
        if(key_tab[j]==k){
            k=j;
            time=0;
            return k;
        }
    }
    return NO;
}

unsigned char wait_key()
{
    unsigned char k;
    while(key()!=NO&&time<WAITTIME);
    while((k=key())==NO && time<WAITTIME);
	return k;
}	
	
unsigned int ascanf(char *format,unsigned char lenth)
{
    unsigned char k,i;
    i=0;
    for(;;){
        k=wait_key();
            if(k== OK)return 1;
            if(k== CANCEL && i>0){
                i--;
                format[i]=0;
            }
            if(k<10){
                if(i<(lenth)){
                    format[i]=k+48;
                    i++;
                    for(k=i;k<lenth;k++)format[k]=0;
                }
            }
        aprintf("\n%s            ",format);
    }
}

unsigned int setup(char *message,int a,int max,int min)
{
    unsigned char k;
loop:
        lcd_opr(1);
        aprintf(message,a);
    if((k=wait_key())==NO)return 0xffff;
            if(k== OK)return a;
            if(k== INC){
                if(a<max)a++;
                aprintf(message,a);
                while(key()==INC){
                    delay(500);
                    while(key()==INC){
                        if(a<max)a++;
                        aprintf(message,a);
                    }
                }
            }
            if(k== DEC){
                if(a>min)a--;
                aprintf(message,a);
                while(key()==DEC){
                    delay(500);
                    while(key()==DEC){
                        if(a>min)a--;
                        aprintf(message,a);
                    }
                }
        }
    goto loop;
}

char password()
{
    unsigned char pass[9],i;
    for(i=0;i<9;i++)pass[i]=0;
    aprintf("\aEnter Password:");
    ascanf(pass,8);
    for(i=0;i<8;i++){
        if(pass[i]!=eprom(PASSWORD+i)){
            aprintf("\bPassword Error!");
            wait_key();
            return 0;
        }
    }
    return 1;
}

control_set()
{
    unsigned char k;
    #pragma memory=code
    unsigned char *prompt[]={"\bMAXIMUM BOOST\nVOLTAGE %oV ",
                    "\bBOOST TAPER AT\nVOLTAGE %oV ",
                    "\bFLOAT MAXIMUM\nVOLTAGE %oV ",
                    "\bFLOAT MINIMUM\nVOLTAGE %oV ",
                    "\bRETURN TO BOOST\nMODE BELOW %oV ",
                    "\bTEMPERATURE COMP\nAT -%dmV/逤/CELL ",
                    "\bSTATE CHANGE\nDELAY %d S  "
                };
    unsigned int MAX[]={BOOST_MAX,TAPER_MAX,FLOAT_MAX,FLOAT_MAX,BOOST_BELOW_MAX,COMP_TEMP_MAX,DELAY_CHANGE_MAX};
    unsigned int MIN[]={BOOST_MIN,TAPER_MIN,FLOAT_MIN,FLOAT_MIN,BOOST_BELOW_MIN,COMP_TEMP_MIN,DELAY_CHANGE_MIN};
    #pragma memory=default
    unsigned int temp[7];
    if(!password())return 0;
    if((temp[0]=setup(prompt[0],eprom(BOOST),MAX[0],MIN[0]))==0xffff)return 0;
    if((temp[1]=setup(prompt[1],eprom(TAPER),MAX[1],MIN[1]))==0xffff)return 0;
    if((temp[2]=setup(prompt[2],eprom(FLOAT_MAX_S),MAX[2],MIN[2]))==0xffff)return 0;
    if((temp[3]=setup(prompt[3],eprom(FLOAT_MIN_S),MAX[3],MIN[3]))==0xffff)return 0;
    if((temp[4]=setup(prompt[4],eprom(BOOST_BELOW),MAX[4],MIN[4]))==0xffff)return 0;
    if((temp[5]=setup(prompt[5],eprom(COMP_TEMP),MAX[5],MIN[5]))==0xffff)return 0;
    if((temp[6]=setup(prompt[6],eprom(DELAY_CHANGE),MAX[6],MIN[6]))==0xffff)return 0;

    aprintf("\aSAVE CHANGE?\nINC=YES OTHER=NO");
    if(wait_key()==INC){
        write_eprom(BOOST,temp[0]);
        write_eprom(TAPER,temp[1]);
        write_eprom(TAPER+1,(temp[1]*4+temp[0])/5);
        write_eprom(TAPER+2,(temp[1]*3+temp[0]*2)/5);
        write_eprom(TAPER+3,(temp[1]*2+temp[0]*3)/5);
        write_eprom(TAPER+4,(temp[1]+temp[0]*4)/5);
        write_eprom(FLOAT_MAX_S,temp[2]);
        write_eprom(FLOAT_MIN_S,temp[3]);
        write_eprom(BOOST_BELOW,temp[4]);
        write_eprom(COMP_TEMP,temp[5]);
        write_eprom(DELAY_CHANGE,temp[6]);
        arg();
    }
    lcd_opr(1);
}

load_setup()
{
    unsigned char k;
    #pragma memory=code
    unsigned char *prompt[]={
        "\bLOW BATTERY AL-\nARM ON AT %oV ",
        "\bRECONNECT LOAD\nABOVE %oV ",
        "\bHIGH BATTERY AL-\nARM ON AT %oV ",
        "\bDELAY BEFORE\nDISCONNECT %d S  "
    };
    unsigned int MAX[]={BT_MIN_MAX,BT_RELOAD_MAX,BT_MAX_MAX,DELAY_DIS_MAX};
    unsigned int MIN[]={BT_MIN_MIN,BT_RELOAD_MIN,BT_MAX_MIN,DELAY_DIS_MIN};
    #pragma memory=default
    unsigned int temp[4];
    if(!password())return 0;
    for(k=0;k<4;k++)
        if((temp[k]=setup(prompt[k],eprom(BT_MIN+k),MAX[k],MIN[k]))==0xffff)return 0;
    aprintf("\aSAVE CHANGE?\nINC=YES OTHER=NO");
    if(wait_key()==INC){
        set_eprom(EWEN);
        for(k=0;k<4;k++)
            write_eprom(BT_MIN+k,temp[k]);
        set_eprom(EWDS);
        arg();
    }
    lcd_opr(1);
}


remote_set()
{
    #pragma memory=code
    const unsigned char *prompt[]={"OFF","ON"};
    #pragma memory=default
    unsigned char Phone[13],Local,Up_hour,Up_min;
    bit Upload;
    unsigned char k;
    if(!password())return 0;
    Upload=eprom(AUTO_UPLOAD);
    for(k=0;k<12;k++)Phone[k]=eprom(PHONE+k);
    Phone[12]=0;
    do{
        aprintf("\aAuto Upload:\n[%s] ",prompt[k=Upload]);
        if((k=wait_key())==NO)return 0;
        if(k==DEC || k==INC)Upload=~Upload;
    }while(k!=OK);
    if(Upload){
        if((Local=setup("\bSit Number:\n%d  ",eprom(LOCAL),255,0))==0xffff)return 0;
        if((Up_hour=setup("\bUpload Hour:\n%d ",eprom(UP_HOUR),23,0))==0xffff)return 0;
        if((Up_min =setup("\bUpload Minute:\n%d ",eprom(UP_MIN),59,0))==0xffff)return 0;
        aprintf("\aRemote Phone:\n%s",Phone);
        if(!ascanf(Phone,12))return 0;
    }
    aprintf("\aSAVE CHANGE?\nINC=YES OTHER=NO");
    if(wait_key()==INC){
        set_eprom(EWEN);
        write_eprom(AUTO_UPLOAD,Upload);
        write_eprom(LOCAL,Local);
        for(k=0;k<12;k++)write_eprom(PHONE+k,Phone[k]);
        write_eprom(UP_HOUR,Up_hour);
        write_eprom(UP_MIN,Up_min);
        set_eprom(EWDS);
        arg();
    }
    lcd_opr(1);
}


unsigned char state(unsigned char a)
{
    return ((sr_out>>a)&1)?'1':'0';
}

solar()
{
    #pragma memory=code
    const char *prompt[]={"OFF","ON"};
    #pragma memory=default
    unsigned int bank=0;
    lcd_opr(1);
    while(1){
        aprintf("\bBANK%d   %oV \n[%s]     %oA  ",bank,ad_get(SR_V0+bank),prompt[state(bank)-'0'],ad_get(SR_I0+bank));
        delay(200);
        switch(key()){
            case INC:bank=(bank+1)%6;break;
            case DEC:if(bank==0)bank=5;else bank--;break;
            case SOLAR:
            case NO:break;
            default:lcd_opr(1);return 0;
        }
        if(time>WAITTIME){
                lcd_opr(1);
                return 0;
        }
    }
}

view()
{
    unsigned char k,address;
    unsigned int i;
    i=k=0;
    for(;;){
        address=i*4+DATA;
        aprintf("\aD%d H:%o L:%oV\nC:%d D:%d",i,eprom(address),eprom(address+1),eprom(address+2),eprom(address+3));
        switch(wait_key()){
            case INC:i=(i+1)%32;break;
            case DEC:if(i==0)i=31;else i--;break;
            case F1:{
                for(i=0;i<32;i++){
                    write_eprom(DATA+i*4,0);
                    write_eprom(DATA+i*4+1,0);
                    write_eprom(DATA+i*4+2,0);
                    write_eprom(DATA+i*4+3,0);
                }
                return 0;
            }
            case F2:{
                for(i=0;i<32;i++){
                    write_eprom(DATA+i*4,450+i*5);
                    write_eprom(DATA+i*4+1,400+i*5);
                    write_eprom(DATA+i*4+2,1000+i*100);
                    write_eprom(DATA+i*4+3,500+i*100);
                }
                return 0;
            }
            default:lcd_opr(1);return 0;
        }
     }
}

testself()
{
    #pragma memory=code
    const char *prompt[]={
        "Test Mode",
        "Meter Adjust"
    };
    #pragma memory=default
    unsigned char k=0,l;
    unsigned char i=0;
    while(k!=OK){
        aprintf("\a%s ",prompt[i]);
        k=wait_key();
        if(k==0xff || k==CANCEL)return 0;
        if(k==INC || k==DEC)if(i==0)i=1;else i=0;
    }
    if(i==0){
        k=eprom(DELAY_CHANGE);
        i=eprom(DELAY_DISCONNECT);
        write_eprom(DELAY_CHANGE,2);
        write_eprom(DELAY_DISCONNECT,2);
        while(time<60 && key()!=CANCEL){
            aprintf("\bTEST A:%d %c%c%c%c%c%c \nBattery: %oV ",solar_state,state(0),state(1),state(2),state(3),state(4),state(5),ad_get(BT_V));
/*            delay(500);*/
        }
        write_eprom(DELAY_CHANGE,k);
        write_eprom(DELAY_DISCONNECT,i);
    }
    else{
        for(i=0;i<16;i++){
            k=NO;
            l=eprom(CLIP+i);
            while(k!=OK){
                aprintf("\aIN %d\n%d  ",i,l);
                k=wait_key();
                if(k==NO || k==CANCEL)return 0;
                if(k==INC)l++;
                if(k==DEC)l--;
            }
            write_eprom(CLIP+i,l);
            lcd_opr(1);
        }
    }
/*    else{
        set_eprom(EWEN);
        for(i=0;i<256;i++){
            tmp=eprom(i);
            write_eprom(i,0);
            if(eprom(i)!=0)goto ERR;
            write_eprom(i,0xff);
            if(eprom(i)!=0xff)goto ERR;
            write_eprom(i,tmp);
            if(eprom(i)!=tmp)goto ERR;
        }
        aprintf("\bEEPROM TEST OK!");
        while(wait_key()!=OK);
        goto next;
ERR:
        aprintf("\bEEPROM TEST \nFAILED!");
        while(wait_key()!=OK);
        return 0;
next:
        for(i=0;i<6;i++){
            EA=0;
            sr_out=1<<i;
            P0=sr_out;
            sr_out_en=1;
            sr_out_en=0;
            while(key()!=CANCEL)aprintf("\bTEST B:  BANK%d\n%oV  %oA  ",i,ad_get(SR_V0+i),ad_get(BT_I));
        }
        EA=1;
    }*/
    lcd_opr(1);
}

/****************************/
/*  单片机串口通讯基本程序  */
/*  无硬件握手信号          */
/*  中断接收、查询发送      */
/****************************/


/*------------------*/
/*  单字符发送      */
/*------------------*/
send(unsigned char Data)
{
	while(TI);
    SBUF=Data;
    delay(10);
}

/*------------------------------*/
/*  发送一个字符串，直到字符为0 */
/*------------------------------*/
sendstr(unsigned char *Data)
{
    while(*Data!=0){
    	send(*Data);
        Data++;
    }
}

/*--------------------------*/
/*  调制解调器初始化        */
/*  设置为无回显、数字回应  */
/*--------------------------*/

/*--------------------------*/
/*  串行通讯中断程序        */
/*  接收中断将字符          */
/*  发送中断清除发送标志    */
/*--------------------------*/
interrupt [0x23] void SCON_int (void)
{
    if(RI){
        buf[bufin]=SBUF;
        bufin>=(sizeof(buf)-1)?bufin=0:bufin++;
        RI=0;
    }
    TI=0;
}

unsigned char recviced()
{
    unsigned char i;
    if(bufin!=bufout){
        for(i=bufout;i!=bufin;){
            if(buf[i]==13)return 1;
            i>=(sizeof(buf)-1)?i=0:i++;
        }
    }
    return 0;
}

unsigned int getb()
{
    unsigned char i;
    if(bufout!=bufin){
        i=buf[bufout];
        bufout>=(sizeof(buf)-1)?bufout=0:bufout++;
        return i;
    }
    else return 0xffff;
}

unsigned char getcode()
{
    unsigned int b1=0xffff,b2=0xffff;
    while((b1=getb())!=0xffff){
	if(b1==13)return b2;
	if((b2=getb())==13)return b1;
    }
}

unsigned char dial()
{
    unsigned char i,phone[13];
    for(i=0;i<13;i++)phone[i]=eprom(PHONE+i);
    bufin=bufout=0;
    sendstr("ATDT");
    sendstr(phone);
    send('\r');
    for(i=0;i<120;i++){
	delay(500);
	if(recviced())break;
    }
    if(getcode()==CONNECT)return 1;
    else return 0;
}

sendw(unsigned int a)
{
    union{
    unsigned int a;
    unsigned char c[2];
    }d;
    d.a=a;
    send(d.c[1]);
    send(d.c[0]);
}

unsigned int getw(){
    union{
    unsigned int a;
    unsigned char b[2];
    }c;
    unsigned int i;
    while((i=getb())==0xffff);
    c.b[0]=i;
    while((i=getb())==0xffff);
    c.b[1]=i;
    return c.a;
}


talk()
{
    unsigned char flag=1,i,CheckSum;
    unsigned int a,b;
    while(flag){
    CheckSum=0;
	switch(getb()){
		case NowData:{
                b=ad_get(BT_V);
                CheckSum+=b;
				sendw(b);
                for(a=0;a<6;a++){
                    b=ad_get(SR_V0+a);
                    CheckSum+=b;
                    sendw(b);
                }
                b=ad_get(LOAD_V);
                CheckSum+=b;
                sendw(b);
                b=ad_get(TEMP_BT);
                CheckSum+=b;
                sendw(b);
                b=ad_get(LOAD_I);
                CheckSum+=b;
                sendw(b);
                b=ad_get(BT_I);
                CheckSum+=b;
                sendw(b);
                for(a=0;a<6;a++){
                    b=ad_get(SR_I0+a);
                    CheckSum+=b;
                    sendw(b);
                }
                b=eprom(DATA);
                CheckSum+=b;
                sendw(b);
                b=eprom(DATA+1);
                CheckSum+=b;
                sendw(b);
                b=chargeah/65536;
                CheckSum+=b;
                sendw(b);
                b=chargeah%65536;
                CheckSum+=b;
                sendw(b);
                b=dischargeah/65536;
                CheckSum+=b;
                sendw(b);
                b=dischargeah%65536;
                CheckSum+=b;
                sendw(b);
                b=min*256+hour;
                CheckSum+=b;
                sendw(b);
                b=load*256+solar_state;
                CheckSum+=b;
                sendw(b);
                b=CheckSum;
                CheckSum+=b;
                sendw(b);
			break;
		}
		case HistoryData:{
			for(a=DATA_INDEX;a<DATA+4*32;a++){
				b=eprom(a);
                CheckSum+=b;
				sendw(b);
			}
            sendw(CheckSum);
			break;
		}
		case SetData:{
			for(a=0;a<30;a++){
				b=eprom(a);
                CheckSum+=b;
				sendw(b);
			}
            sendw(CheckSum);
			break;
		}
		case SendSetData:{
			set_eprom(EWEN);
			for(a=0;a<30;){
				send(a);
				b=getw();
				if(getw()==b){
					write_eprom(a,b);
					a++;
				}
			}
			set_eprom(EWDS);
			send(30);
			break;
		}
		case NO_CARRIER:flag=0;
	}
    }
}

ToPrinter()
{
    #pragma memory=code
    const char fs1[]={0x1b,0x2d,1};
    const char fs0[]={0x1b,0x21,0};
    #pragma memory=default
    char Phone[13];
	int i;
    aprintf("\nPress F2 Print...");
    if(wait_key()!=F2)return 1;
    aprintf("\a\nNow Print...");
    OutDevice=PRINTER;
  aputchar(27);
  aputchar(64);
    aprintf("记录时间:%d时%d分\n",hour,min);
    aprintf(fs1);
    aprintf("\t\t<当前状态记录>");
    aprintf(fs0);aputchar(0);
    aprintf("\n蓄电池电压 = %o V",ad_get(BT_V));
    aprintf("\t蓄电池充电电流 = %o A\n",ad_get(BT_I));
    aprintf("负载电压 = %o V",ad_get(LOAD_V));
    aprintf("\t负载电流 = %o A\n",ad_get(LOAD_I));
    aprintf("蓄电池温度 = %d 摄氏度",ad_get(TEMP_BT));
    aprintf("\t充电状态 = %d \n",solar_state);
    for(i=0;i<6;i++)aprintf("太阳能电池第%d组电压 = %o V\t电流 = %o A\n",i+1,ad_get(SR_V0+i),ad_get(SR_I0+i));
    aprintf("当日累计充电电量 = %d AH",eprom(DATA+2));
    aprintf("\t当日累计放电电量 = %d AH\n",eprom(DATA+3));
    aprintf("当日蓄电池最高电压 = %oV",eprom(DATA));
    aprintf("\t当日蓄电池最低电压 = %oV\n",eprom(DATA+1));
    aprintf(fs1);
    aprintf("\t\t<设置状态记录>");aprintf(fs0);aputchar(0);
    aprintf("\n强充最大电压 = %oV\t",eprom(BOOST));
    aprintf("强充递减电压 = %oV\n",eprom(TAPER));
    aprintf("浮充电压上限 = %oV\t",eprom(FLOAT_MAX_S));
    aprintf("浮充电压下限 = %oV\n",eprom(FLOAT_MIN_S));
    aprintf("返回强充电压 = %oV\t",eprom(BOOST_BELOW));
    aprintf("温度补偿电压 = %dmV\n",eprom(COMP_TEMP));
    aprintf("告警电压上限 = %oV\t",eprom(BT_MAX));
    aprintf("告警电压下限 = %oV\n",eprom(BT_MIN));
    aprintf("负载重连接电压 = %oV\t",eprom(BT_RELOAD));
    aprintf("断开负载前延时 = %dS\n",eprom(DELAY_DISCONNECT));
    aprintf("状态转换延时 = %dS\t",eprom(DELAY_CHANGE));
    aprintf("定时自动上载 = %s\n",up_load?"Yes":"No");
    aprintf("自动上载时间 = %d:%d\t",eprom(UP_HOUR),eprom(UP_MIN));
    for(i=0;i<12;i++)Phone[i]=eprom(PHONE+i);Phone[12]=0;
    aprintf("上载电话号码 = %s\n",Phone);
    aprintf(fs1);
    aprintf("\t\t<历史数据记录>");aprintf(fs0);aputchar(0);
    aprintf("\n天数\t最高电压\t最低电压\t充电电量\t放电电量\n");
	for(i=0;i<32;i++){
        aprintf("第%d天\t%oV\t%oV\t%dAH\t%dAH\n",i,eprom((i*4+DATA)),eprom((i*4+DATA)+1),eprom((i*4+DATA)+2),eprom((i*4+DATA)+3));
	}
    aputchar(12);
    OutDevice=LCD;
    aprintf("\a");
	return 0;
}

monitor graph()
{
  #define MAXLINE 25		/*打印的总行数*/
  #define VALUEY  70		/*Y轴的最大值	*/
  #define SCALE   10		/*Y轴的比例因子*/
  int line,day,i;
  code const char g[]={27,42,1,65,2,0};
  code unsigned int s[4][10]={{700,600,500,400,300,200,100,0,0,0},{700,600,500,400,300,200,100,0,0,0},{4800,4500,4000,3500,3000,2500,2000,1500,1000,500},{4800,4500,4000,3500,3000,2500,2000,1500,1000,500}};
  code const char g1[]={27,42,1,8,0};
  code char *prompt[]={"最后32天最高电压曲线(V/天数)\n\n",
                       "最后32天最低电压曲线(V/天数)\n\n",
                       "最后32天充电量曲线(安时/天数)\n\n",
                       "最后32天放电量曲线(安时/天数)\n\n"};
  char a,counter;
  char b=0;
  aprintf("\aPrint His-Plot\nPress OK Confirm");
  if(wait_key()!=OK)return 1;
  aprintf("\aNow Printing...");
  OutDevice=PRINTER;
  aputchar(27);             /*设定1/8换行量*/
  aputchar(48);
  aputchar(27);             /*设定水平跳格位置为2*/
  aputchar(68);
  aputchar(2);
  aputchar(0);
  aputchar(27);             /*单向打印*/
  aputchar(85);
  aputchar(1);
  for(counter=0;counter<4;counter++){
     b=0;
     if(counter<2)aprintf("   V\n");
     else         aprintf("   AH\n");
      for(line=0;line<MAXLINE;line++){
        if((s[counter][b])<=((MAXLINE-line)*(s[counter][0]/MAXLINE))&& (s[counter][b])>=((MAXLINE-line-1)*(s[counter][0]/MAXLINE))){
          aprintf("%d",counter>1?s[counter][b]:s[counter][b]/10);
          aputchar(9);
          a=1<<(((s[counter][b]-((MAXLINE-line-1)*(s[counter][0]/MAXLINE)))*8)/(s[counter][0]/MAXLINE));
          aprintf(g1);aputchar(0);
          for(i=0;i<8;i++)aputchar(a);
          b++;
        }
        else{
            aputchar(9);
          aprintf(g1);aputchar(0);
          for(i=0;i<8;i++)aputchar(0);
          }
          aprintf(g);
        aputchar(255);
        for(day=31;day>=0;day--){
          if(eprom(DATA+day*4+counter)>=((MAXLINE-line)*(s[counter][0]/MAXLINE))){
        aputchar(255);
        for(i=0;i<16;i++)aputchar(0);
        aputchar(255);
          }
          else{
        if(eprom(DATA+day*4+counter)>=((25-line-1)*(s[counter][0]/MAXLINE))){
          a=1<<(((eprom(DATA+day*4+counter)-((25-line-1)*(s[counter][0]/MAXLINE)))*8)/(s[counter][0]/MAXLINE));
          aputchar(a*2-1);
          for(i=0;i<16;i++)aputchar(a);
          aputchar(a*2-1);
        }
        else for(i=0;i<18;i++)aputchar(0);
          }
       }
        aputchar(13);
        aputchar(10);
      }
      aputchar(9);aprintf(g1);aputchar(0);
      for(i=0;i<8;i++)aputchar(0);
      aprintf(g);for(i=0;i<590;i++)aputchar(0x80);
        aputchar(13);
        aputchar(10);
      aprintf("     1       5         10        15        20        25          31  (天数)\n");
        aputchar(13);
        aputchar(10);
        aprintf("\n\t\t%s\n\n",prompt[counter]);
        if(counter==1)aputchar(12);
    }
    aputchar(12);

  OutDevice=LCD;
  aprintf("\a");
  return 0;
}

save()
{
    char i;
    set_eprom(EWEN);
    for(i=30;i>=0;i--){
        write_eprom(DATA+(i+1)*4,eprom(DATA+i*4));
        write_eprom(DATA+(i+1)*4+1,eprom(DATA+i*4+1));
        write_eprom(DATA+(i+1)*4+2,eprom(DATA+i*4+2));
        write_eprom(DATA+(i+1)*4+3,eprom(DATA+i*4+3));
    }
    write_eprom(DATA,0);
    write_eprom(DATA+1,10000);
    write_eprom(DATA+2,0);
    write_eprom(DATA+3,0);
    set_eprom(EWDS);
    chargeah=dischargeah=0;
}

main()
{
    init();
    chargeah=dischargeah=0;
    goto start;
    while(1){
        if(key()!=NO){
start:
            lcd_init();
            port|=LAMP;
            P0=port;
            port_out_en=1;
            port_out_en=0;
            while(time<WAITTIME){
                aprintf("\bCURRENT TIME:\n%d:%d:%d   ",hour,min,sec);
                switch(key()){
                    case BT_V_KEY:{
                        while(key()==BT_V_KEY);
                        while(key()==NO&&time<WAITTIME)aprintf("\bBATTERY VOLTAGE\n          %oV ",ad_get(BT_V));
                        lcd_opr(1);
                        break;
                    }
                    case BT_I_KEY:{
                        while(key()==BT_I_KEY);
                        while(key()==NO&&time<WAITTIME)aprintf("\bCHARGE CURRENT\n          %oA ",ad_get(BT_I));
                        lcd_opr(1);
                        break;
                    }
                    case LOAD_V_KEY:{
                        while(key()==LOAD_V_KEY);
                        while(key()==NO&&time<WAITTIME)aprintf("\bLOAD VOLTAGE\n          %oV ",ad_get(LOAD_V));
                        lcd_opr(1);
                        break;
                    }
                    case LOAD_I_KEY:{
                        while(key()==LOAD_I_KEY);
                        while(key()==NO&&time<WAITTIME)aprintf("\bLOAD CURRENT\n          %oA ",ad_get(LOAD_I));
                        lcd_opr(1);
                        break;
                    }
                    case SOLAR:solar();break;
                    case CONTROL_SET:control_set();break;
                    case TESTSELF:testself();break;
                    case REMOTE_SET:remote_set();break;
                    case LOAD_SET:load_setup();break;
                    case VIEW:view();break;
                    case F1:ToPrinter();break;
                    case F2:graph();
                }
            }
            lcd_opr(8);
            port&=~LAMP;
            P0=port;
            port_out_en=1;
            port_out_en=0;
        }
        if(up_load && hour==eprom(UP_HOUR) && min>=eprom(UP_MIN)){
            if(dial()==1){
                talk();
                up_load=0;
            }
        }
        if(recviced()){
            if(getcode()==CONNECT){
                talk();
            }
        }
        if(newday){
            save();
            newday=0;
        }
    }
}

