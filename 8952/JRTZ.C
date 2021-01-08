/*==================*/
/*  JRTZ.C          */
/*  1997.6.1        */
/*==================*/

/*==============*/
/*  °üº¬Í·ÎÄ¼þ  */
/*==============*/
#ifdef POD8751
    #include <reg52.h>
#else
    #include <io52.h>
#endif

#include <stdarg.h>
#include "jrt.h"

/*==============*/
/*  ÏµÍ³ºê¶¨Òå  */
/*==============*/
#define FREQ    11.0592
#define DELAY_SCALE 115 /*1000/(12/FREQ)*/
#define TIMEBASE    50     /*  ¶¨Ê±ÖÐ¶ÏÊ±¼ä,µ¥Î»ms */
#define T0_INIT     19457   /*0xffff-(TIMEBASE*1000)/(12/FREQ)  */
#define WAITTIME 3
#define COMP_ZERO   200

/*==================*/
/*  ÏµÍ³±äÁ¿¶¨Òå    */
/*==================*/
#pragma memory=idata

struct {
    unsigned int bt_v;
    unsigned int Solar_Voltage[6];
    unsigned int load_v;
	int Battery_Temp;
    int In_Temp;
/*    int Nc1;
    int Nc2;
    int Door;
    int Yan;*/
    unsigned int load_i;
    unsigned int bt_i;
    unsigned int Solar_Current[6];

    unsigned int bt_v_max;
    unsigned int bt_v_min;
    unsigned long charge_ah;
    unsigned long discharge_ah;
    unsigned char hour;
    unsigned char min;
    unsigned char solar_state;
    unsigned char load;
    unsigned int CheckSum;
}now;
unsigned char ad_ch;

struct{
    unsigned int bt_min;
    unsigned int bt_max;
    unsigned int bt_reload;
    unsigned char delay_disconnect;
}load_set;

    bit up_load,up;
struct{
    unsigned char up_hour,up_min;
}remote;

unsigned char sec,time;


/*
    V[0]=0
    V[1]=taper
    V[2]=(4taper+bm)/5
    V[3]=(3taper+2bm)/5
    V[4]=(2taper+3bm)/5
    V[5]=(taper+4bm)/5
    V[6]=(bm)
    */



unsigned int ad_result;
unsigned char please;
unsigned char Delay_change;
unsigned char Delay_disconnect;

unsigned char port,sr_out,load_out;
unsigned char buf[10];               /*  ½ÓÊÕ»º³åÇø£¬ÕâÊÇÒ»¸öÏÈ½øÏÈ³ö¶ÔÁÐ  */
unsigned char bufin,bufout;

unsigned char clip[12];
#pragma memory=data

struct {
    unsigned int float_max;
    unsigned int float_min;
    unsigned int boost_below;
    unsigned int comp_temp;
    unsigned int  delay_change;
    unsigned int v[7];
}set;



#pragma memory=default

/*==================*/
/*  ÊäÈëÊä³ö¿Ú¶¨Òå  */
/*==================*/
#ifdef POD8751
    sbit port_out_en=P2^5;
    sbit sr_out_en=P2^6;
    sbit load_out_en=P2^7;
    sbit wdg=P2^4;
#else
    bit port_out_en=0xa5,sr_out_en=0xa6,load_out_en=0xa7;
    bit wdg=0xb4;
#endif


#ifdef POD8751
    sbit RS=P2^0;
    sbit rw=P2^1;
    sbit e=P2^2;
#else
    bit RS=0xa0,rw=0xa1,e=0xa2;
/*    bit RS=0xd8,rw=0xd9,e=0xda;*/
#endif


#ifdef POD8751
  sbit Do=P1^3;
  sbit di=P1^2;
  sbit clk=P1^1;
  sbit cs=P1^0;
#else  
  bit clk=0x96,cs=0x97,di=0x95,Do=0x94;
#endif


#ifdef POD8751
    sbit sk=P1^2;
    sbit dout=P2^3;
#else
    bit sk=0xb7,dout=0xb5,cs0=0xb2;
#endif

/*======================*/
/*      º¯ÊýÔ¤ËµÃ÷      */
/*======================*/
lcd_init(void);
aprintf(char *format,...);
delay(unsigned int times);
set_eprom(unsigned char command);
unsigned int eprom(unsigned char address);
unsigned char wait_key();
write_eprom(unsigned char address,unsigned int Data);
sendstr(unsigned char *Data);

/*==============*/
/*  ÏµÍ³³õÊ¼»¯  */
/*==============*/
init()
{
    #ifdef LOAD_DEFAULT
        unsigned char i;
    #endif
    WR=0;
    P0=sr_out=load_out=0;
    load_out_en=0;
    sr_out_en=0;
    port=P0=0x10;
    port_out_en=0;
    now.load=0;
    now.bt_v_min=1024;
    /*  LCDÏÔÊÆ÷³õÊ¼»¯ */
    lcd_init();
    /*  ÏµÍ³×´Ì¬ÅÐ±ð    */
#if MODEL==70    
    aprintf("\a   JRT-48-100");
#endif
#if MODEL==71
    aprintf("\a   JRT-24-100");
#endif
    /*  ¶¨Ê±Æ÷0³õÊ¼»¯    */
    arg();
    IE=0x82;
    SCON=0x70;
    TMOD=0x21;
    TH1=0xfd;
    PCON=0;
    TR0=ES=TR1=EA=1;
    sendstr("AT&FE0V0X0S0=3\r");
	delay(1000);	    
    bufin=bufout=0;
    #ifdef LOAD_DEFAULT
    if(eprom(SYSTEM_FLAG)!=MODEL){
        aprintf("\aDATA ERROR!\nLoad Default.");
        wait_key();
        set_eprom(EWEN);
        write_eprom(BOOST,BOOST_DEFAULT_VALUE);
        write_eprom(TAPER,TAPER_DEFAULT_VALUE);
        write_eprom(FLOAT_MAX_S,FLOAT_MAX_DEFAULT_VALUE);
        write_eprom(FLOAT_MIN_S,FLOAT_MIN_DEFAULT_VALUE);
        write_eprom(DELAY_CHANGE,DELAY_CHANGE_DEFAULT_VALUE);
        write_eprom(BOOST_BELOW,BOOST_BELOW_DEFAULT_VALUE);
        write_eprom(COMP_TEMP,COMP_TEMP_DEFAULT_VALUE);
        write_eprom(LCD_TEMP,LCD_TEMP_DEFAULT_VALUE);
        write_eprom(BT_MIN,BT_MIN_DEFAULT_VALUE);
        write_eprom(BT_MAX,BT_MAX_DEFAULT_VALUE);
        write_eprom(BT_RELOAD,BT_RELOAD_DEFAULT_VALUE);
        write_eprom(DELAY_DISCONNECT,DELAY_DISCONNECT_DEFAULT_VALUE);
        for(i=DATA_INDEX;i<DATA+4*32;i++)write_eprom(i,0);
        for(i=PASSWORD;i<PASSWORD+8;i++)write_eprom(i,'8');
        for(i=PHONE;i<PHONE+12;i++)write_eprom(i,'*');
        write_eprom(SYSTEM_FLAG,MODEL);
        set_eprom(EWDS);
    }
    #endif
}

moring(){
    unsigned char i;
    i=eprom(DATA_INDEX);
    i=i*4+DATA;
    write_eprom(i,now.bt_v_max);
    write_eprom(i+1,now.bt_v_min);
    write_eprom(i+2,now.charge_ah/3600);
    write_eprom(i+3,now.discharge_ah/3600);
    write_eprom(DATA_INDEX,i==31?0:i++);
    write_eprom(DAY,eprom(DAY)+1);
    exit();
}

arg(){
    unsigned char temp;
/*    int *p;
    p=(int *)&set;
    for(temp=0;temp<5;temp++)p[temp]=eprom(FLOAT_MAX_S+temp);*/
    set.float_max=eprom(FLOAT_MAX_S);
    set.float_min=eprom(FLOAT_MIN_S);
    Delay_change=set.delay_change=eprom(DELAY_CHANGE);
    set.boost_below=eprom(BOOST_BELOW);
    set.comp_temp=eprom(COMP_TEMP);
/*    p=(int *)&load_set;
    for(temp=0;temp<4;temp++)p[temp]=eprom(BT_MIN+temp);*/
    load_set.bt_min=eprom(BT_MIN);
    load_set.bt_max=eprom(BT_MAX);
    load_set.bt_reload=eprom(BT_RELOAD);
    Delay_disconnect=load_set.delay_disconnect=eprom(DELAY_DISCONNECT);
    set.v[6]=eprom(BOOST);
    set.v[1]=eprom(TAPER);
    temp=(set.v[6]-set.v[1])/5;
    set.v[0]=0;
    set.v[2]=set.v[1]+temp;
    set.v[3]=set.v[2]+temp;
    set.v[4]=set.v[3]+temp;
    set.v[5]=set.v[4]+temp;
    up_load=eprom(AUTO_UPLOAD);
    remote.up_hour=eprom(UP_HOUR);
    remote.up_min=eprom(UP_MIN);
    for(temp=0;temp<12;temp++)clip[temp]=eprom(CLIP+temp);
    if(eprom(DAY)>=400)while(1){
        lcd_init();
        aprintf("ERROR 601");
    }
}

int ad(unsigned char ch)
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
    a.l=128+clip[ch];
    a.l=result*a.l;
    result=a.l/1024;
    sk=dout=cs0=1;
    return result;
}

ad_get(unsigned char ch)
{
    switch(ch){
        case LOAD_V:return now.bt_v-ad(ch);
        case TEMP_BT:return (ad(ch)*4-2732-COMP_ZERO)/10;
        case BT_V:return ad(ch)-(set.comp_temp*(MODEL==71?12:24)*now.Battery_Temp)/100;
        case LOAD_I:return ad(ch)/2;
        case SR_I0:sr_out=1;ch=BT_I;break;
        case SR_I1:sr_out=2;ch=BT_I;break;
        case SR_I2:sr_out=4;ch=BT_I;break;
        case SR_I3:sr_out=8;ch=BT_I;break;
        case SR_I4:sr_out=0x10;ch=BT_I;break;
        case SR_I5:sr_out=0x20;ch=BT_I;break;
    }
    if(now.load!=0)sr_out|=LOAD;else sr_out&=~LOAD;
    P0=sr_out;
    sr_out_en=1;
    sr_out_en=0;
    return ad(ch);
}

#ifndef POD8751
    interrupt [0x0b] void T0_int (void)
#else
    timer0() interrupt 1 using 2
#endif
{
    static unsigned char ms;
    static unsigned char next_state;
    int *now_point;
    unsigned char tmp;

    /*¶¨Ê±Æ÷³õÖµÉèÖÃ*/
    TH0=T0_INIT/256;
    TL0=T0_INIT%256;

    /*50mS¼ÓÒ»*/
    if((ms++)==(1000/TIMEBASE-1)){

        /*¿´ÃÅ¹·¸´Î»*/
        wdg=~wdg;
        /*1Ãë*/
        ms=0;
        /*³äµçÁ÷³Ì¿ØÖÆ*/
        if(now.solar_state<7){
            /*Ç¿³ä×´Ì¬,1-6*/
            if(now.bt_v<set.v[now.solar_state-1]){
                if(next_state==now.solar_state-1){
                    if(Delay_change==0)now.solar_state-=1;
                }
                else{
                    next_state=now.solar_state-1;
                    Delay_change=set.delay_change;
                }
            }
            else {
                if(now.bt_v>=set.v[now.solar_state]){
                    if(next_state==now.solar_state+1){
                        if(Delay_change==0)now.solar_state+=1;
                    }
                    else{
                        next_state=now.solar_state+1;
                        Delay_change=set.delay_change;
                    }
                }
                else next_state=0;
            }
        }
        else{
            /*¸¡³ä×´Ì¬,7-13*/
            if(now.bt_v<(now.solar_state==13?set.boost_below:set.float_min)){
                if(next_state==now.solar_state+1){
                    if(Delay_change==0)now.solar_state=now.solar_state==13?1:now.solar_state+1;
                }
                else{
                    next_state=now.solar_state+1;
                    Delay_change=set.delay_change;
                }
            }
            else {
                if(now.bt_v>set.float_max && now.solar_state!=7){
                    if(next_state==now.solar_state-1){
                        if(Delay_change==0)now.solar_state-=1;
                    }
                    else{
                        next_state=now.solar_state-1;
                        Delay_change=set.delay_change;
                    }
                }
                else next_state=0;
            }
        }
        /*×î¸ß,×îµÍµçÑ¹,³äµç,·Åµç°²Ê±¼ÇÂ¼*/
        if(now.bt_v>now.bt_v_max)now.bt_v_max=now.bt_v;
        if(now.bt_v<now.bt_v_min)now.bt_v_min=now.bt_v;
        now.charge_ah+=now.bt_i;
        now.discharge_ah+=now.load_i;

        /*¸ºÔØ¿ØÖÆ,load=0,¸ºÔØÕý³£¹©µç*/
        if(!now.load){
            if(now.bt_v>load_set.bt_max){
                /*¹ýÑ¹¸æ¾¯*/
                if(Delay_disconnect==0){
                    load_out|=(ALARM_HI|LOAD_DISABLE|UJ);
                    now.load=1;
                }
            }
            else{
                if(now.bt_v<load_set.bt_min){
                    /*Ç·Ñ¹¸æ¾¯*/
                    if(Delay_disconnect==0){
                        load_out|=(ALARM_LO|LOAD_DISABLE|UJ);
                        now.load=2;
                    }
                }
                else{
                    Delay_disconnect=load_set.delay_disconnect;
                }
            }
        }
        else{
            if(now.load==1){
                /*¹ýÑ¹¸æ¾¯»Ö¸´*/
                if(now.bt_v<load_set.bt_max-40){
                    load_out&=~(ALARM_HI|LOAD_DISABLE|UJ);
                    now.load=0;
                }
            }
            else{
                /*Ç·Ñ¹¸æ¾¯»Ö¸´*/
                if(now.bt_v>load_set.bt_reload){
                    load_out&=~(ALARM_LO|LOAD_DISABLE|UJ);
                    now.load=0;
                }
            }
        }
        Delay_change--;
        Delay_disconnect--;

        if((sec++)==59){
            sec=0;
            time++;
            if((now.min++)==59){
                now.min=0;
                if((now.hour++)==23){

                    exit();
                }
            }
        }
    }
    tmp=P0;
    /*A/DÑ­»·²ÉÑù*/
    now_point=(int *)&now;
    now_point[ad_ch]=ad_get(ad_ch>9?ad_ch+4:ad_ch);

    ad_ch>=11?ad_ch=0:ad_ch++;
    if(please!=0xff){
        ad_result=ad_get(please);
        please=0xff;
    }
    P0=load_out;
    load_out_en=1;
    load_out_en=0;
    switch(now.solar_state){
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
        default:now.solar_state=1;
    }
    if(now.load!=0)sr_out|=LOAD;else sr_out&=~LOAD;
    P0=sr_out;
    sr_out_en=1;
    sr_out_en=0;
    P0=tmp;
}

DELAY()
{
    clk=1;
    clk=0;
}
  
/*----------------------*/
/*  EPROMÉèÖÃ¼°²Á³ý³ÌÐò */
/*  command=00H,EWDS  */
/*  command=01H,EWEN  */
/*----------------------*/
set_eprom(unsigned char command)
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
/*  EPROMÐ´Èë³ÌÐò           */
/*  address<256,Ð´µ¥¸öÊý¾Ý  */
/*--------------------------*/
write_eprom(unsigned char address,unsigned int Data)
{
    unsigned char k;
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
}

/*------------------*/
/*  EPROM¶Á³ö³ÌÐò   */
/*------------------*/
unsigned int eprom(unsigned char address)
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
/*  Òº¾§ÏÔÊ¾Çý¶¯³ÌÐò    */
/*  ÐÍºÅ:   MDLS-16265B */
/*----------------------*/
/*----------------------*/
/*  Òº¾§ÏÔÊ¾Æ÷ÅÐÃ¦³ÌÐò  */
/*----------------------*/

unsigned char busy()
{
    unsigned char ac;
    P0=0xff;
    RS=0;
    rw=1;
    EA=0;
    e=1;
    ac=P0;
    e=0;
    EA=1;
    return ac>>7;
}

lcd_opr(unsigned char ch)
{
    while(busy());
    RS=rw=0;
    P0=ch;
    e=1;
    e=0;
}

/*--------------------------*/
/*  Òº¾§ÏÔÊ¾Æ÷³õÊ¼»¯³ÌÐò    */
/*--------------------------*/
lcd_init(void)
{
    /*  ¹¤×÷·½Ê½ÉèÖÃ,8Î»Êý¾Ý½Ó¿Ú,Á½ÐÐÏÔÊ¾,5X7µãÕó   */
    lcd_opr(0x38);
    /*  ÊäÈë·½Ê½Ñ¡Ôñ,AC×Ô¶¯¼Ó1,»­Ãæ²»¶¯ */
    lcd_opr(6);
    /*  ÏÔÊ¾¿ª¹Ø¿ØÖÆ,¿ªÏÔÊ¾,¹Ø¹â±ê,½ûÖ¹ÉÁË¸ */
    lcd_opr(12);
    lcd_opr(1);
}


/*------------------------------*/
/*  µ¥×Ö·ûÏÔÊ¾,¹©printf()µ÷ÓÃ   */
/*------------------------------*/
unsigned int aputchar(unsigned char c)
{
    while(busy());
    RS=rw=0;
/*    if(c!='\n'){
        if(c!='\b'){
            if(c!='\a'){
                RS=1;
                P0=c;
            }
            else P0=1;
        }
        else P0=2;
    }
    else P0=0xc0;*/
    switch(c){
        case '\n':P0=0xc0;break;
        case '\b':P0=2;break;
        case '\a':P0=1;break;
        default:RS=1;P0=c;
    }
    e=1;
    e=0;
}

/*------------------*/
/*  ¸ñÊ½»¯Êä³öÃüÁî  */
/*------------------*/
aprintf (char *format,...)
{
    unsigned char format_flag,*ptr;                               /*¸ñÊ½±êÖ¾*/
    unsigned int div_val, u_val, base;
    va_list ap;
    va_start(ap,format);
    for (;;){
        while ((format_flag = *format++) != '%'){      /* Until '%' or '\0' */
            if (!format_flag){
                return (1);
            }
            aputchar (format_flag);
        }
        switch (format_flag = *format++){
            case 'c':
                format_flag = va_arg(ap,int);
            default:
                aputchar (format_flag);
            continue;
            case 's':
                ptr = va_arg(ap,char *);
            while (format_flag = *ptr++){
                aputchar (format_flag);
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
    #pragma memory=code
    const unsigned char key_tab[16]={112,115,114,113,
                        179,178,177,211,
                        210,209,227,176,
                        208,226,225,224};
/*    const unsigned unsigned char i[4]={0xfe,0xfd,0xfb,0xf7};*/
    #pragma memory=default
    unsigned char j,k;
    unsigned char key;
    key=0xff;
    for(j=0;j<4;j++){
        P1=~(1<<j);
        k=P1|0xf;
        if(k!=0xff){
            delay(20);
            if(k==P1|0xf)key=(k&0xf0)+j;
            break;
        }
    }
    P1=0xff;
    for(j=0;j<16;j++){
        if(key_tab[j]==key){
            key=j;
            time=0;
            return key;
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
            if(k== CANCEL && i>0)i--;format[i]=0;
            if(k<10){
                format[i]=k+48;
                if(i<lenth)i++;
                for(k=i;k<lenth;k++)format[k]=0;
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

/*char password()
{
    unsigned char pass[9],i;
    for(i=0;i<9;i++)pass[i]=0;
    aprintf("\bEnter Password:");
    if(ascanf(pass,7))return 0;
    for(i=0;i<8;i++){
        if(pass[i]!=eprom(PASSWORD+i)){
            aprintf("\bPassword Error!");
            return 0;
        }
    }
    return 1;
} */

control_set()
{
    unsigned char k;
    #pragma memory=code
    unsigned char *prompt[]={"\bMAXIMUM BOOST\nVOLTAGE %oV ",
                    "\bBOOST TAPER AT\nVOLTAGE %oV ",
                    "\bFLOAT MAXIMUM\nVOLTAGE %oV ",
                    "\bFLOAT MINIMUM\nVOLTAGE %oV ",
                    "\bRETURN TO BOOST\nMODE BELOW %oV ",
                    "\bTEMPERATURE COMP\nAT -%dmV/ßC/CELL ",
                    "\bSTATE CHANGE\nDELAY %d S  "
                };
    unsigned int MAX[]={BOOST_MAX,TAPER_MAX,FLOAT_MAX,FLOAT_MAX,BOOST_BELOW_MAX,COMP_TEMP_MAX,DELAY_CHANGE_MAX};
    unsigned int MIN[]={BOOST_MIN,TAPER_MIN,FLOAT_MIN,FLOAT_MIN,BOOST_BELOW_MIN,COMP_TEMP_MIN,DELAY_CHANGE_MIN};
    #pragma memory=default
    unsigned int temp[7];
/*    if(!password())return 0;*/
    for(k=0;k<7;k++)
        if((temp[k]=setup(prompt[k],eprom(BOOST+k),MAX[k],MIN[k]))==0xffff)return 0;
    aprintf("\aSAVE CHANGE?\nINC=YES OTHER=NO");
    if(wait_key()==INC){
        set_eprom(EWEN);
        for(k=0;k<7;k++)write_eprom(BOOST+k,temp[k]);
        set_eprom(EWDS);
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
    #ifdef PASSWORD_USED
    if(!password())return 0;
    #endif
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
    #ifdef PASSWORD_USED
    if(!password())return 0;
    #endif
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

unsigned int get_result(unsigned char a)
{
    please=a;
    while(please!=0xff);
    return ad_result;
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
        aprintf("\bBANK%d   %oV \n[%s]     %oA  ",bank,now.Solar_Voltage[bank],prompt[state(bank)-'0'],get_result(SR_I0+bank));
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
        address=((i+eprom(DATA_INDEX))%32)*4+DATA;
        aprintf("\aD%d %oBV~%oV\nCHR %d  DIS %d",i,eprom(address),eprom(address+1),eprom(address+2),eprom(address+3));
        switch(wait_key()){
            case INC:i=(i+1)%32;break;
            case DEC:if(i==0)i=31;else i--;break;
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
    unsigned char k=0;
    unsigned char i=0;
    while(k!=OK){
        aprintf("\a%s ",prompt[i]);
        k=wait_key();
        if(k==0xff || k==CANCEL)return 0;
        if(k==INC || k==DEC)if(i==0)i=1;else i=0;
    }
    if(i==0){
        set.delay_change=load_set.delay_disconnect=2;
        while(time<60 && key()!=CANCEL){
            aprintf("\bTEST A:%d %c%c%c%c%c%c \nBattery: %oV ",now.solar_state,state(0),state(1),state(2),state(3),state(4),state(5),now.bt_v);
/*            delay(500);*/
        }
        set.delay_change=eprom(DELAY_CHANGE);
        load_set.delay_disconnect=eprom(DELAY_DISCONNECT);
    }
    else{
        for(i=0;i<16;i++){
            k=NO;
            while(k!=OK){
                aprintf("\aIN %d\n%d  ",i,clip[i]);
                k=wait_key();
                if(k==NO || k==CANCEL)return 0;
                if(k==INC)clip[i]++;
                if(k==DEC)clip[i]--;
            }
            set_eprom(EWEN);
            write_eprom(CLIP+i,clip[i]);
            set_eprom(EWDS);
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
/*  µ¥Æ¬»ú´®¿ÚÍ¨Ñ¶»ù±¾³ÌÐò  */
/*  ÎÞÓ²¼þÎÕÊÖÐÅºÅ          */
/*  ÖÐ¶Ï½ÓÊÕ¡¢²éÑ¯·¢ËÍ      */
/****************************/


/*------------------*/
/*  µ¥×Ö·û·¢ËÍ      */
/*------------------*/
send(unsigned char Data)
{
	while(TI);
    SBUF=Data;
    delay(10);
}

/*------------------------------*/
/*  ·¢ËÍÒ»¸ö×Ö·û´®£¬Ö±µ½×Ö·ûÎª0 */
/*------------------------------*/
sendstr(unsigned char *Data)
{
    while(*Data!=0){
    	send(*Data);
        Data++;
    }
}

/*--------------------------*/
/*  µ÷ÖÆ½âµ÷Æ÷³õÊ¼»¯        */
/*  ÉèÖÃÎªÎÞ»ØÏÔ¡¢Êý×Ö»ØÓ¦  */
/*--------------------------*/

/*--------------------------*/
/*  ´®ÐÐÍ¨Ñ¶ÖÐ¶Ï³ÌÐò        */
/*  ½ÓÊÕÖÐ¶Ï½«×Ö·û          */
/*  ·¢ËÍÖÐ¶ÏÇå³ý·¢ËÍ±êÖ¾    */
/*--------------------------*/
#ifndef POD8751
    interrupt [0x23] void SCON_int (void)
#else
    serial() interrupt 4 using 2
#endif
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
    sendstr("ATDP");
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
    unsigned char flag=1,i;
    unsigned int *p,a,b;
    for(i=0;i<6;i++)now.Solar_Current[i]=get_result(SR_I0+i);
    while(flag){
	now.CheckSum=0;
	switch(getb()){
		case NowData:{
			#ifdef DEBUG_TERM
			aprintf("\bsendnow");
			#endif
            p=(unsigned int *)&now;
			for(a=0;a<sizeof(now)/2;a++){
				b=p[a];
				now.CheckSum+=b;
				sendw(b);
			}
			#ifdef DEBUG_TERM
			aprintf ("\nchecksum:%d",now.CheckSum);
			#endif
			break;
		}
		case HistoryData:{
			#ifdef DEBUG_TERM
			aprintf("\bsethis");
			#endif
			for(a=DATA_INDEX;a<DATA+4*32;a++){
				b=eprom(a);
				now.CheckSum+=b;
				sendw(b);
			}
			sendw(now.CheckSum);
			#ifdef DEBUG_TERM
			aprintf ("\nchecksum:%d",now.CheckSum);
			#endif
			break;
		}
		case SetData:{
			#ifdef DEBUG_TERM
			aprintf("\bsetSET");
			#endif
			for(a=0;a<30;a++){
				b=eprom(a);
				now.CheckSum+=b;
				sendw(b);
			}
			sendw(now.CheckSum);
			#ifdef DEBUG_TERM
			aprintf ("\nchecksum:%d",now.CheckSum);
			#endif
			break;
		}
		case SendSetData:{
			set_eprom(EWEN);
			for(a=0;a<30;){
				send(a);
				b=getw();
				if(getw()==b){
					write_eprom(a,b);
					#ifdef DEBUG_TERM
					aprintf("\nnow:%d",a);
					#endif
					a++;
				}
			}
			set_eprom(EWDS);
			send(30);
			#ifdef DEBUG_TERM
			aprintf("\bok");
			#endif
			break;
		}
		case NO_CARRIER:flag=0;
	}
    }
}

main()
{
    init();
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
                aprintf("\bCURRENT TIME:\n%d:%d:%d   ",now.hour,now.min,sec);
/*                aprintf("\bBT_TEMP:\n%d",now.Battery_Temp);*/
                switch(key()){
                    case BT_V_KEY:{
                        while(key()==BT_V_KEY);
                        while(key()==NO&&time<WAITTIME)aprintf("\bBATTERY VOLTAGE\n          %oV ",now.bt_v);
                        lcd_opr(1);
                        break;
                    }
                    case BT_I_KEY:{
                        while(key()==BT_I_KEY);
                        while(key()==NO&&time<WAITTIME)aprintf("\bCHARGE CURRENT\n          %oA ",now.bt_i);
                        lcd_opr(1);
                        break;
                    }
                    case LOAD_V_KEY:{
                        while(key()==LOAD_V_KEY);
                        while(key()==NO&&time<WAITTIME)aprintf("\bLOAD VOLTAGE\n          %oV ",get_result(LOAD_V));
                        lcd_opr(1);
                        break;
                    }
                    case LOAD_I_KEY:{
                        while(key()==LOAD_I_KEY);
                        while(key()==NO&&time<WAITTIME)aprintf("\bLOAD CURRENT\n          %oA ",now.load_i);
                        lcd_opr(1);
                        break;
                    }
                    case SOLAR:solar();break;
                    case CONTROL_SET:control_set();break;
                    case TESTSELF:testself();break;
                    case REMOTE_SET:remote_set();break;
                    case LOAD_SET:load_setup();break;
                    case VIEW:view();break;
                }
            }
                #ifndef DEBUG_TERM
            lcd_opr(8);
            port&=~LAMP;
            P0=port;
            port_out_en=1;
            port_out_en=0;
            #endif
        }
        if(up_load && now.hour==remote.up_hour && now.min>=remote.up_min){
            if(dial()==1){
                talk();
                up_load=0;
            }
        }
                #ifdef DEBUG_TERM
        aprintf("\bbuf:%d,%d",bufin,bufout);
        #endif
        if(recviced()){
            if(getcode()==CONNECT){
                #ifdef DEBUG_TERM
                aprintf("\bconnect!");
                #endif
                talk();
            }
        }
    }
}

