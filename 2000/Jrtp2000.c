/*==========================*/
/*	JRTP2000.C				*/
/*	2000.5.16				*/
/*	通讯程序加强			*/
/*==========================*/
/*  JRTZ.C                  */
/*  1997.6.1                */
/*  1998年5月增加打印功能   */
/*  更名为JRTP.C            */
/*==========================*/

/*==============*/
/*  包含头文件  */
/*==============*/
#include <AT89x52.h>
#include <stdarg.h>
#include "jrtp99.h"

/*==============*/
/*  系统宏定义  */
/*==============*/
#define FREQ    11.0592
#define DELAY_SCALE 100 /*1000/(12/FREQ)*/
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
idata unsigned char buf[6];               /*  接收缓冲区，这是一个先进先出对列  */
unsigned char point;
unsigned long chargeah,dischargeah;
unsigned int btv;
unsigned char please;
unsigned int adresult;
bit OutDevice;
bit newday;
bit IsSolar;
bit received;
bit TEST;
bit TXEN;

/*==================*/
/*  输入输出口定义  */
/*==================*/
sbit out_en=0xb6;
sbit port_out_en=0xa5;
sbit sr_out_en=0xa6;
sbit load_out_en=0xa7;
sbit wdg=0xb4;
sbit RS=0xa0;
sbit rw=0xa1;
sbit e=0xa2;
sbit clk=0x96;
sbit cs=0x97;
sbit di=0x95;
sbit Do=0x94;
sbit sk=0xb7;
sbit dout=0xb5;
sbit cs0=0xb2;
sbit stb=0xa3;
sbit usy=0xa4;


/*======================*/
/*      函数预说明      */
/*======================*/
lcd_init(void);
/*monitor*/ aprintf(char *format,...) reentrant;
delay(unsigned int times);
/*monitor*/ set_eprom(unsigned char command);
/*monitor*/ unsigned int eprom(unsigned char address);
unsigned char wait_key();
/*monitor*/ write_eprom(unsigned char address,unsigned int Data);
sendstr(unsigned char *Data);
unsigned char key();
sendpacket(unsigned char,unsigned int);
void talk(void);

arg(){
//    if(eprom(DAY)>=400)while(1){
//        lcd_init();
//        aprintf("ERROR 601");
//    }
//	wdg=~wdg;
    write_eprom(1,eprom(TAPER));
    write_eprom(2,(eprom(TAPER)*4+eprom(BOOST))/5);
    write_eprom(3,(eprom(TAPER)*3+eprom(BOOST)*2)/5);
    write_eprom(4,(eprom(TAPER)*2+eprom(BOOST)*3)/5);
    write_eprom(5,(eprom(TAPER)+eprom(BOOST)*4)/5);
    write_eprom(6,eprom(BOOST));
}

/*==============*/
/*  系统初始化  */
/*==============*/
init()
{
    unsigned char i;
    out_en=0;
    sr_out=0;
    sr_out|=LOAD;
    P0=sr_out;
    load_out=0;
    load_out_en=0;
    sr_out_en=0;
    port=P0=LAMP;
    port_out_en=1;
    port_out_en=0;
    load=0;
    write_eprom(BT_V_MIN,1024);
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
    delay(3000);
    TXEN=1;
    point=0;
    if(eprom(SYSTEM_FLAG)!=MODEL){
        aprintf("\aEPROM ERROR!\nLoad Default...");
        wait_key();
        write_eprom(BOOST,BOOST_DEFAULT_VALUE);
        write_eprom(TAPER,TAPER_DEFAULT_VALUE);
        write_eprom(FLOAT_MAX_S,FLOAT_MAX_DEFAULT_VALUE);
        write_eprom(FLOAT_MIN_S,FLOAT_MIN_DEFAULT_VALUE);
        write_eprom(DELAY_CHANGE,DELAY_CHANGE_DEFAULT_VALUE);
        write_eprom(BOOST_BELOW,BOOST_BELOW_DEFAULT_VALUE);
        write_eprom(COMP_TEMP,COMP_TEMP_DEFAULT_VALUE);
        write_eprom(BT_MIN,BT_MIN_DEFAULT_VALUE);
        write_eprom(BT_MAX,BT_MAX_DEFAULT_VALUE);
        write_eprom(BT_RELOAD,BT_RELOAD_DEFAULT_VALUE);
        write_eprom(DELAY_DISCONNECT,DELAY_DISCONNECT_DEFAULT_VALUE);
        for(i=PASSWORD;i<PASSWORD+8;i++)write_eprom(i,'8');
        for(i=CLIP;i<CLIP+16;i++)write_eprom(i,128);
        write_eprom(SYSTEM_FLAG,MODEL);
    }
    arg();                                
    /*  定时器0初始化    */
//    IE=0x82;
	IE=0x12;
	IP=0x10;
    SCON=0x70;
    TMOD=0x21;
    TH1=0xfd;
    PCON=0;
    TR0=TR1=1;
    EA=1;
}



/*monitor*/ int ad(unsigned char ch)
{
    union{
        unsigned char i[2];
        unsigned long l;
    }a;
    unsigned int result,sum;
    port=((port&0xf0)|ch);
    P0=port;
    port_out_en=1;
    port_out_en=0;
    sum=0;
    for(a.i[0]=0;a.i[0]<255;a.i[0]++);
    for(a.i[0]=0;a.i[0]<16;a.i[0]++){
    	result=0;
    	sk=0;
    	dout=1;
    	cs0=0;
    	while(!dout);
    	sk=1;
    	sk=0;
    	sk=1;
    	result|=dout;
    	sk=0;
    	for(a.i[1]=0;a.i[1]<11;a.i[1]++){
        	sk=1;
        	result=(result<<1)|dout;
        	sk=0;
        }
    	cs0=sk=dout=1;
    	sum+=result;
    	wdg=~wdg;
    }
    result=sum/16;
//    a.l=128+eprom(CLIP+ch);
//    a.l=result*a.l;
//    result=a.l/1024;
    return result;
}

        /*
monitor ad_get(unsigned char ch)
{
        if(ch==LOAD_V)return ad_get(BT_V)-ad(ch);
        if(ch==TEMP_BT)return (ad(ch)*4-2732-COMP_ZERO);
        if(ch==BT_V)return ad(ch)-(eprom(COMP_TEMP)*(MODEL==71?12:24)*ad_get(TEMP_BT));
        if(ch==LOAD_I)return ad(ch)/2;
        if(ch==BT_I)return ad(ch);
        if(ch==SR_I0)sr_out=1;ch=BT_I;
        if(ch==SR_I1)sr_out=2;ch=BT_I;
        if(ch==SR_I2)sr_out=4;ch=BT_I;
        if(ch==SR_I3)sr_out=8;ch=BT_I;
        if(ch==SR_I4)sr_out=0x10;ch=BT_I;
        if(ch==SR_I5)sr_out=0x20;ch=BT_I;
    if(load!=0)sr_out|=LOAD;else sr_out&=~LOAD;
    P0=sr_out;
    sr_out_en=1;
    sr_out_en=0;
    return ad(ch);
}       */
/*monitor*/ ad_get(unsigned char ch)
{
	unsigned int tem;
//    EA=0;
    switch(ch){
        case LOAD_V:
        	tem=ad(ch)/4;
        	if(btv>tem)return btv-tem;
        	return 0;
        case SR_V0:
        case SR_V1:
        case SR_V2:
        case SR_V3:
        case SR_V4:
        case SR_V5:
          sr_out=0x3f;
          if(load!=0)sr_out&=~LOAD;else sr_out|=LOAD;
          P0=sr_out;
          sr_out_en=1;
          sr_out_en=0;
          return btv+700-(ad(ch)*9)/25;
        case TEMP_BT:
        	tem=(ad(ch)-2732-COMP_ZERO)/10;
        	if(tem>100)return 0;
        	return tem;
        case BT_V:return ad(ch)/4;
        case LOAD_I:return ad(ch)/10;                                             
        case BT_I:return (ad(ch)*2)/5;
        case SR_I0:sr_out=~1;ch=BT_I;break;
        case SR_I1:sr_out=~2;ch=BT_I;break;
        case SR_I2:sr_out=~4;ch=BT_I;break;
        case SR_I3:sr_out=~8;ch=BT_I;break;
        case SR_I4:sr_out=~0x10;ch=BT_I;break;
        case SR_I5:sr_out=~0x20;ch=BT_I;break;
        default:return 0;
    }
    if(load!=0)sr_out&=~LOAD;else sr_out|=LOAD;
    P0=sr_out;
    sr_out_en=1;
    sr_out_en=0;
//    EA=1;
    return (ad(ch)*2)/5;
}

rad_get(unsigned char CH)
{
	please=CH;
	for(CH=0;CH<255;CH++){
		if(please==0xff)return adresult;
  		delay(20);
   	}
}

//interrupt [0x0b] void T0_int (void)
void T0_int (void) interrupt 1
{
    static unsigned char ms;
    static unsigned char next_state;
    unsigned int value;

    /*定时器初值设置*/
    TH0=T0_INIT/256;
    TL0=T0_INIT%256;
	if(please!=0xff){
		adresult=ad_get(please);
		please=0xff;
	}
	if(received)talk();
    /*50mS加一*/
    if((ms++)==(1000/TIMEBASE-1)){

        /*看门狗复位*/
        wdg=~wdg;
        /*1秒*/
        ms=0;

        /*充电流程控制*/
        btv=ad_get(BT_V);
        value=btv-(eprom(COMP_TEMP)*(MODEL==71?12:24)*ad_get(TEMP_BT))/100;
        if(solar_state<7){
            if(value<eprom(solar_state-1)){
                if(next_state==solar_state-1){
                    if(Delay_change==0)solar_state-=1;
                }
                else{
                    next_state=solar_state-1;
                    Delay_change=TEST?2:eprom(DELAY_CHANGE);
                }
            }
            else {
                if(value>=eprom(solar_state)){
                    if(next_state==solar_state+1){
                        if(Delay_change==0)solar_state+=1;
                    }
                    else{
                        next_state=solar_state+1;
                        Delay_change=TEST?2:eprom(DELAY_CHANGE);
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
                    Delay_change=TEST?2:eprom(DELAY_CHANGE);
                }
            }
            else {
                if(value>eprom(FLOAT_MAX_S) && solar_state!=7){
                    if(next_state==solar_state-1){
                        if(Delay_change==0)solar_state-=1;
                    }
                    else{
                        next_state=solar_state-1;
                        Delay_change=TEST?2:eprom(DELAY_CHANGE);
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
                    Delay_disconnect=TEST?2:eprom(DELAY_DISCONNECT);
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
        case 13:sr_out=0;break;
        case 2:
        case 12:sr_out=0x20;break;
        case 3:
        case 11:sr_out=0x30;break;
        case 4:
        case 10:sr_out=0x38;break;
        case 5:
        case 9:sr_out=0x3c;break;
        case 6:
        case 8:sr_out=0x3e;break;
        case 7:sr_out=0x3f;break;
        default:solar_state=1;
    }
    if(load!=0)sr_out&=~LOAD;else sr_out|=LOAD;
    if(!IsSolar){
        P0=sr_out;
        sr_out_en=1;
        sr_out_en=0;
    }
    P0=value;
}

void talk(void)
{
	unsigned char value;
    	if(buf[1]==1){			//read variable
    		switch(buf[2]){
     			case 0:         //btv
       				sendpacket(0,btv);
       		        break;
       	        case 1:
       	        	sendpacket(1,ad_get(LOAD_V));
       	            break;
       	        case 2:
       	        	sendpacket(2,ad_get(SR_V0));
       	            break;
       	        case 3:
       	        	sendpacket(3,ad_get(SR_V1));
       	            break;
       	        case 4:
       	        	sendpacket(4,ad_get(SR_V2));
       	            break;
       	        case 5:
       	        	sendpacket(5,ad_get(SR_V3));
       	            break;
       	        case 6:
       	        	sendpacket(6,ad_get(SR_V4));
       	            break;
       	        case 7:
       	        	sendpacket(7,ad_get(SR_V5));
       	            break;
       	        case 8:
       	        	sendpacket(8,ad_get(TEMP_BT)+20);
       	            break;
       	        case 9:
       	        	sendpacket(9,(int)solar_state);
       	            break;
       	        case 10:
       	        	sendpacket(10,(int)load);
       	            break;
       	        case 14:    
       	        	sendpacket(14,ad_get(LOAD_I));
       	            break;
       	        case 15:
       	        	sendpacket(15,ad_get(BT_I));
       	            break;
       	        case 0x10:
       	        	sendpacket(0x10,ad_get(SR_I0));
       	            break;
       	        case 0x11:
       	        	sendpacket(0x11,ad_get(SR_I1));
       	            break;
       	        case 0x12:
       	        	sendpacket(0x12,ad_get(SR_I2));
       	            break;
       	        case 0x13:
       	        	sendpacket(0x13,ad_get(SR_I3));
       	            break;
       	        case 0x14:
       	        	sendpacket(0x14,ad_get(SR_I4));
       	            break;
       	        case 0x15:
       	        	sendpacket(0x15,ad_get(SR_I5));
       	            break;
       	        case 0x20:
       	           	sendpacket(0x20,eprom(BOOST));	//BOOST
       	            break;
       	        case 0x21:
       	        	sendpacket(0x21,eprom(TAPER));	//taper
       	            break;
       	        case 0x22:
       	        	sendpacket(0x22,eprom(FLOAT_MAX_S));
       	            break;
       	        case 0x23:
       	        	sendpacket(0x23,eprom(FLOAT_MIN_S));
       	            break;
       	        case 0x24:
       	        	sendpacket(0x24,eprom(BOOST_BELOW));
       	            break;
       	        case 0x25:
       	        	sendpacket(0x25,eprom(COMP_TEMP));
       	            break;
       	        case 0x26:
       	        	sendpacket(0x26,eprom(DELAY_CHANGE));
       	            break;
       	        case 0x27:
       	        	sendpacket(0x27,eprom(BT_MIN));
       	            break;
       	        case 0x28:
       	        	sendpacket(0x28,eprom(BT_RELOAD));
       	            break;
       	        case 0x29:
       	        	sendpacket(0x29,eprom(BT_MAX));
       	            break;
       	        case 0x2a:
       	        	sendpacket(0x2a,eprom(DELAY_DISCONNECT));
       	            break;
       	        case 0x2b:
       	        	sendpacket(0x2b,eprom(BT_V_MIN));
       	            break;
       	        case 0x2c:
       	        	sendpacket(0x2c,eprom(BT_V_MAX));
       	            break;
       	        case 0x2d:
       	        	sendpacket(0x2d,eprom(CHARGE_AH));
       	            break;
       	        case 0x2e:
       	        	sendpacket(0x2e,eprom(DISCHARGE_AH));
       	            break;
       	        case DATA_INDEX:
       	        	for(value=DATA;value<DATA+32*4;value++){
       	        	    sendpacket(value,eprom(value));
       	                wdg=~wdg;
       	            }
       	            break;
       	        default:sendpacket(0xff,0xff); 
       		}
       	}
       	else{                   //write writeable variable
       		switch(buf[2]){
       	        case 0x20:
       	        	write_eprom(BOOST,buf[3]*256+buf[4]);
       	        	sendpacket(0x20,eprom(BOOST));	//BOOST
       	            arg();
       	            break;
       	        case 0x21:
       	        	write_eprom(TAPER,buf[3]*256+buf[4]);
       	        	sendpacket(0x21,eprom(TAPER));	//taper
       	            arg();
       	            break;
       	        case 0x22:
       	        	write_eprom(FLOAT_MAX_S,buf[3]*256+buf[4]);
       	        	sendpacket(0x22,eprom(FLOAT_MAX_S));
       	            break;
       	        case 0x23:
       	        	write_eprom(FLOAT_MIN_S,buf[3]*256+buf[4]);
       	        	sendpacket(0x23,eprom(FLOAT_MIN_S));
       	            break;
       	        case 0x24:
       	        	write_eprom(BOOST_BELOW,buf[3]*256+buf[4]);
       	        	sendpacket(0x24,eprom(BOOST_BELOW));
       	            break;
       	        case 0x25:
       	        	write_eprom(COMP_TEMP,buf[3]*256+buf[4]);
       	        	sendpacket(0x25,eprom(COMP_TEMP));
       	            break;
       	        case 0x26:
       	        	write_eprom(DELAY_CHANGE,buf[3]*256+buf[4]);
       	        	sendpacket(0x26,eprom(DELAY_CHANGE));
       	            break;
       	        case 0x27:
       	        	write_eprom(BT_MIN,buf[3]*256+buf[4]);
       	        	sendpacket(0x27,eprom(BT_MIN));
       	            break;
       	        case 0x28:
       	        	write_eprom(BT_RELOAD,buf[3]*256+buf[4]);
       	        	sendpacket(0x28,eprom(BT_RELOAD));
       	            break;
       	        case 0x29:
       	        	write_eprom(BT_MAX,buf[3]*256+buf[4]);
       	        	sendpacket(0x29,eprom(BT_MAX));
       	            break;
       	        case 0x2a:
       	        	write_eprom(DELAY_DISCONNECT,buf[3]*256+buf[4]);
       	        	sendpacket(0x2a,eprom(DELAY_DISCONNECT));
       	            break;
       	     	default:
       	        	write_eprom(buf[2],buf[3]*256+buf[4]);
       	            sendpacket(buf[2],eprom(buf[2]));
       	            break;
       		}
       	}
		received=0;
    }


/*DELAY()
{
    clk=1;
    clk=0;
} */
  
#define DELAY() clk=1;clk=0;
/*----------------------*/
/*  EPROM设置及擦除程序 */
/*  command=00H,EWDS  */
/*  command=01H,EWEN  */
/*----------------------*/
/*monitor*/ set_eprom(unsigned char command)
{
    unsigned char k;
    EA=0;
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
    EA=1;
}

/*--------------------------*/
/*  EPROM写入程序           */
/*  address<256,写单个数据  */
/*--------------------------*/
/*monitor*/ write_eprom(unsigned char address,unsigned int Data)
{
    unsigned char k;
    set_eprom(EWEN);
    EA=0;
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
    EA=1;
    set_eprom(EWDS);
}

/*------------------*/
/*  EPROM读出程序   */
/*------------------*/
/*monitor*/ unsigned int eprom(unsigned char address)
{
    unsigned char k,i;
    unsigned int result;
    EA=0;
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
    EA=1;
    return result;
}

/*----------------------*/
/*  液晶显示驱动程序    */
/*  型号:   MDLS-16265B */
/*----------------------*/
/*----------------------*/
/*  液晶显示器判忙程序  */
/*----------------------*/

/*monitor*/ unsigned char busy()
{
    unsigned char ac;
    EA=0;
    P0=0xff;
    RS=0;
    rw=1;
    e=1;
    ac=P0;
    e=0;
    EA=1;
    return ac>>7;
}

/*monitor*/ lcd_opr(unsigned char ch)
{
    while(busy());
    EA=0;
    RS=rw=0;
    P0=ch;
    e=1;
    e=0;
    EA=1;
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
/*monitor*/ aputchar(unsigned char c)  reentrant
{
    if(OutDevice==LCD){
        while(busy());
//        EA=0;
        RS=rw=0;
        switch(c){
            case '\n':P0=0xc0;break;
            case '\b':P0=2;break;
            case '\a':P0=1;break;
            default:RS=1;P0=c;
        }
        e=1;
        e=0;
  //      EA=1;
    }
    else{
       while(usy){
            OutDevice=LCD;
            aprintf("\aPRINTER NOT\n READY...");
            if(key()==CANCEL)return 1;
            OutDevice=PRINTER;
        }
        EA=0;
       P0=c;
       stb=0;
       stb=1;
       EA=1;
       if(c=='\n')aputchar('\r');
    }
    return 0;
}

/*------------------*/
/*  格式化输出命令  */
/*------------------*/
/*monitor*/ aprintf (char *format,...) reentrant
{
    unsigned char format_flag,*ptr;                               /*格式标志*/
    unsigned int div_val, u_val, base;
    va_list ap;
    va_start(ap,format);
//    EA=0;
    for (;;){
        while ((format_flag = *format++) != '%'){      /* Until '%' or '\0' */
            if (!format_flag){
//                EA=1;
                return (1);
            }
            if(aputchar (format_flag))return 1;
        }
        switch (format_flag = *format++){
            case 'c':
                format_flag = va_arg(ap,char);
            default:
                if(aputchar (format_flag))return 1;
            continue;
            case 's':
                ptr = va_arg(ap,char *);
            while (format_flag = *ptr++){
                if(aputchar (format_flag))return 1;
            }
            continue;
            case 'e':
                base=10;
                div_val=100;
                u_val=va_arg(ap,unsigned char);
                while(div_val>1&& div_val >u_val){
                    div_val/=10;
                }
                do{
                    aputchar(u_val/div_val+48);
                    u_val%=div_val;
                    div_val/=base;
                }while(div_val);
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
    for(;time>0;time--)for(j=0;j<DELAY_SCALE;j++)wdg=~wdg;
}

unsigned char key()
{
//#pragma memory=code
//    code unsigned char key_tab[16]={112,115,114,113,
//                        179,178,177,211,
//                        210,209,227,176,
//                        208,226,225,224};
//    code unsigned char key_tab[16]={227,211,179,115,//115,114,113,112,
//    								226,210,178,114,//179,178,177,176,
//       					            225,209,177,113,//211,210,209,208,
//       		            			224,208,176,112};//227,226,225,224};
    code unsigned char key_tab[16]={112,176,208,224,
                        113,177,209,225,
                        114,178,210,226,
                        115,179,211,227};   /* 2000,5,20  */

/*    2000,6,22 
	code unsigned char key_tab[16]={112,113,114,115,
					176,177,178,179,
					208,209,210,211,
					224,225,226,227};   */

//#pragma memory=default
    unsigned char j,k;
    wdg=~wdg;
    for(j=0;j<4;j++){
        P1=~(1<<j);
        k=P1|0xf;
        if(k!=0xff){
            delay(20);
            if(k==P1|0xf)k=(k&0xf0)+j;
            break;
        }
    }
    P1=0xff;
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
                        delay(200);
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
                        delay(200);
                    }
                }
        }
    goto loop;
}

char password()
{
    unsigned char pass[4],i;
    for(i=0;i<4;i++)pass[i]=0;
    aprintf("\aEnter Password:");
    ascanf(pass,3);
    for(i=0;i<3;i++){
        if(pass[i]!=eprom(PASSWORD+i)){
            aprintf("\bPassword Error!");
            wait_key();
            aprintf("\a");
            return 0;
        }
    }
    return 1;
}

control_set()
{
    unsigned char k;
//    #pragma memory=code
    code unsigned char *prompt[]={"\bMAXIMUM BOOST\nVOLTAGE %oV ",
                    "\bBOOST TAPER AT\nVOLTAGE %oV ",
                    "\bFLOAT MAXIMUM\nVOLTAGE %oV ",
                    "\bFLOAT MINIMUM\nVOLTAGE %oV ",
                    "\bRETURN TO BOOST\nMODE BELOW %oV ",
                    "\bTEMPERATURE COMP\nAT -%dmV/逤/CELL ",
                    "\bSTATE CHANGE\nDELAY %d S  "
                };
    code unsigned int MAX[]={BOOST_MAX,TAPER_MAX,FLOAT_MAX,FLOAT_MAX,BOOST_BELOW_MAX,COMP_TEMP_MAX,DELAY_CHANGE_MAX};
    code unsigned int MIN[]={BOOST_MIN,TAPER_MIN,FLOAT_MIN,FLOAT_MIN,BOOST_BELOW_MIN,COMP_TEMP_MIN,DELAY_CHANGE_MIN};
//    #pragma memory=default
    unsigned int temp[7];
    if(!password())return 0;
    for(k=0;k<7;k++)
        if((temp[k]=setup(prompt[k],eprom(BOOST+k),MAX[k],MIN[k]))==0xffff)return 0;
    aprintf("\aSAVE CHANGE?\nINC=YES OTHER=NO");
    if(wait_key()==INC){
        for(k=0;k<7;k++)write_eprom(BOOST+k,temp[k]);
        arg();
    }
    lcd_opr(1);
}

load_setup()
{
    unsigned char k;
//    #pragma memory=code
    code unsigned char *prompt[]={
        "\bLOW BATTERY AL-\nARM ON AT %oV ",
        "\bRECONNECT LOAD\nABOVE %oV ",
        "\bHIGH BATTERY AL-\nARM ON AT %oV ",
        "\bDELAY BEFORE\nDISCONNECT %d S  "
    };
    code unsigned int MAX[]={BT_MIN_MAX,BT_RELOAD_MAX,BT_MAX_MAX,DELAY_DIS_MAX};
    code unsigned int MIN[]={BT_MIN_MIN,BT_RELOAD_MIN,BT_MAX_MIN,DELAY_DIS_MIN};
//    #pragma memory=default
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
//    #pragma memory=code
//    code unsigned char *prompt[]={"OFF","ON"};
//    #pragma memory=default
    unsigned char /*Phone[1],*/Local/*,Up_hour,Up_min*/;
    bit Upload=1;
//    unsigned char k;
    if(!password())return 0;
/*    Upload=eprom(AUTO_UPLOAD);*/
    Local=eprom(LOCAL);
/*    for(k=0;k<12;k++)Phone[k]=eprom(PHONE+k);
    Phone[12]=0;*/
/*    while(1){
        k=Upload;
        aprintf("\aAuto Upload:\n[%s] ",prompt[k]);
        k=wait_key();
        if(k==NO)return 0;
        if(k==DEC || k==INC)Upload=~Upload;
        if(k==OK)break;
    }*/
/*    if(Upload){*/
        if((Local=setup("\bSit Number:\n%d  ",eprom(LOCAL),255,0))==0xffff)return 0;
/*        if((Up_hour=setup("\bUpload Hour:\n%d ",eprom(UP_HOUR),23,0))==0xffff)return 0;
        if((Up_min =setup("\bUpload Minute:\n%d ",eprom(UP_MIN),59,0))==0xffff)return 0;
        aprintf("\aRemote Phone:\n%s",Phone);
        if(!ascanf(Phone,12))return 0;
    }*/
    aprintf("\aSAVE CHANGE?\nINC=YES OTHER=NO");
    if(wait_key()==INC){
        set_eprom(EWEN);
/*        write_eprom(AUTO_UPLOAD,Upload);*/
        write_eprom(LOCAL,Local);
/*        for(k=0;k<12;k++)write_eprom(PHONE+k,Phone[k]);
        write_eprom(UP_HOUR,Up_hour);
        write_eprom(UP_MIN,Up_min);*/
        set_eprom(EWDS);
        arg();
    }
    lcd_opr(1);
}


char state(unsigned char a)
{
    return ((sr_out>>a)&1)?'0':'1';
}

solar()
{
//    #pragma memory=code
    code char *prompt[]={"OFF","ON"};
//    #pragma memory=default
    unsigned char bank=0;
    bit change=1;
    lcd_opr(1);
    IsSolar=1;
    while(1){
    	if(change){
    	 	aprintf("\bBANK%e %oVoc   ",bank+1,rad_get(SR_V0+bank));
      		change=0;
       	}
        aprintf("\n[%s]     %oA  ",prompt[state(bank)-'0'],rad_get(SR_I0+bank));
        delay(200);
        switch(key()){
            case INC:bank=(bank+1)%6;change=1;break;
            case DEC:if(bank==0)bank=5;else bank--;change=1;break;
            case SOLAR:change=1;
            case NO:break;
            default:IsSolar=0;lcd_opr(1);return 0;
        }
        if(time>WAITTIME){
                lcd_opr(1);
                IsSolar=0;
                return 0;
        }
    }
}

view()
{
    unsigned char k,address;
    unsigned char i;
    i=k=0;
    for(;;){
        address=i*4+DATA;
        aprintf("\aD%e H:%o L:%oV",i,eprom(address),eprom(address+1));
        aprintf("\nC:%d D:%d",eprom(address+2),eprom(address+3));
        switch(wait_key()){
            case INC:i=(i+1)%32;break;
            case DEC:if(i==0)i=31;else i--;break;
            default:lcd_opr(1);return 0;
        }
     }
}

testself()
{
//    #pragma memory=code
    code char *prompt[]={
        "Test Mode",
        "Meter Adjust"
    };
//    #pragma memory=default
    
    unsigned char k=0,l;
    unsigned char i=0;
    while(k!=OK){
        aprintf("\a%s ",prompt[i]);
        k=wait_key();
        if(k==0xff || k==CANCEL)return 0;
        if(k==INC || k==DEC)if(i==0)i=1;else i=0;
    }
    if(i==0){
        TEST=1;
        while(time<60 && key()!=CANCEL){
            aprintf("\bTEST A:%e",solar_state);
            aprintf(" %c",state(0));
            aprintf("%c",state(1));
            aprintf("%c" ,state(2));
            aprintf("%c",state(3));
            aprintf("%c",state(4));
            aprintf("%c  ",state(5));
            aprintf("\nB: %oV T:%d ",btv,rad_get(TEMP_BT)+20);
            delay(500);
        }
        Delay_change=eprom(DELAY_CHANGE);
        Delay_disconnect=eprom(DELAY_DISCONNECT);
        TEST=0;
    }
    else{
        for(i=0;i<16;i++){
            k=NO;
            l=eprom(CLIP+i);
            while(k!=OK){
                aprintf("\aIN %e\n%e  ",i,l);
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
            while(key()!=CANCEL)aprintf("\bTEST B:  BANK%d\n  %oA  ",i,ad_get(BT_I));
        }
        EA=1;
    }*/
    lcd_opr(1);
}

/****************************/



ToPrinter()
{
//    #pragma memory=code
    code char fs1[]={0x1b,0x2d,1};
    code char fs0[]={0x1b,0x21,0};
//    #pragma memory=default
//    idata char Phone[13];
	int i;
	aprintf("\aPRINT?");
    aprintf("\nPress OK Print...");
    if(wait_key()!=OK)return 1;
    aprintf("\a\nNow Print...");
    OutDevice=PRINTER;
  aputchar(27);
  aputchar(64);
    aprintf(fs1);
    aprintf("\n\t\t<当前状态记录>");
    aprintf(fs0);aputchar(0);
    aprintf("\n蓄电池电压 = %o V",btv);
    aprintf("\t蓄电池充电电流 = %o A\n",rad_get(BT_I));
    aprintf("负载电压 = %o V",rad_get(LOAD_V));
    aprintf("\t负载电流 = %o A\n",rad_get(LOAD_I));
    aprintf("蓄电池温度 = %d 摄氏度",rad_get(TEMP_BT)+20);
    aprintf("\t充电状态 = %e \n",solar_state);
    for(i=0;i<6;i++)aprintf("太阳能电池第%d组电流 = %o A\n",i+1,rad_get(SR_I0+i));
    aprintf("当日累计充电电量 = %d AH",eprom(DATA+2));
    aprintf("\t当日累计放电电量 = %d AH\n",eprom(DATA+3));
    aprintf("当日蓄电池最高电压 = %o V",eprom(DATA));
    aprintf("\t当日蓄电池最低电压 = %o V\n",eprom(DATA+1));
    aprintf("运行时间: %e 时 %e 分\n",hour,min);
    aprintf(fs1);
    aprintf("\n\n\t\t<设置状态记录>\n");aprintf(fs0);aputchar(0);
    aprintf("\n强充最大电压 = %o V\t",eprom(6));
    aprintf("强充递减电压 = %o V\n",eprom(1));
    aprintf("浮充电压上限 = %o V\t",eprom(FLOAT_MAX_S));
    aprintf("浮充电压下限 = %o V\n",eprom(FLOAT_MIN_S));
    aprintf("返回强充电压 = %o V\t",eprom(BOOST_BELOW));
    aprintf("温度补偿电压 = %d mV\n",eprom(COMP_TEMP));
    aprintf("告警电压上限 = %o V\t",eprom(BT_MAX));
    aprintf("告警电压下限 = %o V\n",eprom(BT_MIN));
    aprintf("负载重连接电压 = %o V\t",eprom(BT_RELOAD));
    aprintf("断开负载前延时 = %d S\n",eprom(DELAY_DISCONNECT));
    aprintf("状态转换延时 = %d S\t\n",eprom(DELAY_CHANGE));
//    aprintf("定时自动上载 = %s\n",up_load?"Yes":"No");
//    aprintf("自动上载时间 = %d:%d\t",eprom(UP_HOUR),eprom(UP_MIN));
//    for(i=0;i<12;i++)Phone[i]=eprom(PHONE+i);Phone[12]=0;
//    aprintf("上载电话号码 = %s\n",Phone);
    aprintf(fs1);
    aprintf("\n\n\t\t<历史记录>");aprintf(fs0);aputchar(0);
    aprintf("\n日 期 \t最高电压\t最低电压\t充电电量\t放电电量\n");
	for(i=0;i<32;i++){
        aprintf("第 %d 天\t%oV\t%oV",i,eprom((i*4+DATA)),eprom((i*4+DATA)+1));
        aprintf("\t%d AH\t%d AH\n",eprom((i*4+DATA)+2),eprom((i*4+DATA)+3));
	}
    aputchar(12);
    OutDevice=LCD;
    aprintf("\a");
	return 0;
}

/*monitor*/ graph()
{
  #define MAXLINE 25		/*打印的总行数*/
  #define VALUEY  70		/*Y轴的最大值	*/
  #define SCALE   10		/*Y轴的比例因子*/
  int line;
  int i,day;
  code const char g[]={27,42,1,65,2,0};
  code unsigned int s[4][10]={{700,600,500,400,300,200,100,0,0,0},{700,600,500,400,300,200,100,0,0,0},{4800,4500,4000,3500,3000,2500,2000,1500,1000,500},{4800,4500,4000,3500,3000,2500,2000,1500,1000,500}};
  code const char g1[]={27,42,1,8,0};
  code char *prompt[]={"最后32天最高电压曲线(V/天)\n\n",
                       "最后32天最低电压曲线(V/天)\n\n",
                       "最后32天充电量曲线(安时/天)\n\n",
                       "最后32天放电量曲线(安时/天)\n\n"};
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
      aprintf("     31       25        20        15        10         5          0  (天)\n");
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
    sendstr("AT&FE0V0X0S0=1\r");
}

main()
{
//    unsigned int tt;
    init();
    chargeah=dischargeah=0;
//    set_eprom(EWEN);
    write_eprom(DATA,0);
    write_eprom(DATA+1,10000);
//    set_eprom(EWDS);
    TEST=0;
    sendstr("AT&FE0V0&K0X0S0=1\r");
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
                aprintf("\bBATTERY VOLTAGE\n          %oV ",btv);
                switch(key()){
                    case BT_V_KEY:{
                        while(key()==BT_V_KEY);
                        aprintf("\a");
                        while(key()==NO&&time<WAITTIME){
                            aprintf("\bBATTERY VOLTAGE\n          %oV ",btv);
                            delay(500);
                        }
                        lcd_opr(1);
                        break;
                    }
                    case BT_I_KEY:{
                        while(key()==BT_I_KEY);
                        aprintf("\a");
                        while(key()==NO&&time<WAITTIME){
                            aprintf("\bCHARGE CURRENT\n          %oA ",rad_get(BT_I));
                            delay(500);
                        }
                        lcd_opr(1);
                        break;
                    }
                    case LOAD_V_KEY:{
                        while(key()==LOAD_V_KEY);
                        aprintf("\a");
                        while(key()==NO&&time<WAITTIME){
                            aprintf("\bLOAD VOLTAGE\n          %oV ",rad_get(LOAD_V));
                            delay(500);
                        }
                        lcd_opr(1);
                        break;
                    }
                    case LOAD_I_KEY:{
                        while(key()==LOAD_I_KEY);
                        aprintf("\a");
                        while(key()==NO&&time<WAITTIME){
                            aprintf("\bLOAD CURRENT\n          %oA ",rad_get(LOAD_I));
                            delay(500);
                        }
                        lcd_opr(1);
                        break;
                    }
                    case SOLAR:solar();break;
                    case CONTROL_SET:control_set();break;
                    case TESTSELF:testself();break;
                    case REMOTE_SET:remote_set();break;
                    case LOAD_SET:load_setup();break;
                    case VIEW:view();break;
                    case F1:ToPrinter();aprintf("\a");break;
                    case F2:graph();aprintf("\a");
                }
//	        if(recviced()){
//                tt=getb();
//                if(/*tt==local||*/tt==0xff){
//                	send('K');
//                    aprintf("\aRemote Connect!");
//	                talk();
//	            }
//	        }
            }
            lcd_opr(8);
            port&=~LAMP;
            P0=port;
            port_out_en=1;
            port_out_en=0;
        }
/*        if(up_load && hour==eprom(UP_HOUR) && min>=eprom(UP_MIN)){
            if(dial()==1){
                talk();
                up_load=0;
            }
        }*/
//        if(recviced()){
//            tt=getb();
//            if(/*tt==local||*/tt==0xff){
//            	send('K');
//            	port|=LAMP;
//            	P0=port;
//            	port_out_en=1;
//            	port_out_en=0;
//            	lcd_init();
//                aprintf("\bRemote Connect!");
//                talk();
//                port&=~LAMP;
//            	P0=port;
//            	port_out_en=1;
//            	port_out_en=0;
//            	lcd_opr(8);
//            }
//        }
        if(newday){
            save();
            newday=0;
        }
    }
}

