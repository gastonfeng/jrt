/*--------------------------------------*/
/*  JRT-xx-xx系列太阳能智能充电控制器   */
/*  主程序名: JRT.C                     */
/*  CPU型号:  89C52                     */
/*  程序设计: 冯建涛                    */
/*  开始日期: 1997年1月1日              */
/*--------------------------------------*/
#define FREQ    11.0592
#define DELAY_SCALE 50 /*1000/(12/FREQ)*/

/*----------------------*/
/*  调试或实际运行选择  */
/*----------------------*/
#ifdef POD8751
    #include <reg52.h>
#else
    #include <io52.h>
#endif
#include "jrt.h"

/*--------------*/
/*  寄存器定义  */
/*--------------*/
char hour,min,sec;
int v[7],float_min,float_max,boost_below;
/*
    V[0]=0
    V[1]=taper
    V[2]=(4taper+bm)/5
    V[3]=(3taper+2bm)/5
    V[4]=(2taper+3bm)/5
    V[5]=(taper+4bm)/5
    V[6]=(bm)
    */
long charge_ah,discharge_ah;
int bt_v_min,bt_v_max;
char time;      /*  显示器工作时间  */
int bt_v;       /*  蓄电池电压      */
int bt_i;       /*  蓄电池充电电流  */
int load_i;     /*  负载电流        */
char solar_state;
int comp_temp;
int  delay_change;

int bt_min;
int bt_max;
int bt_reload;
char delay_disconnect;

int ad_result;
char please;

unsigned char port,sr_out,load_out;

#ifdef POD8751
    sbit port_out_en=P2^5;
    sbit sr_out_en=P2^6;
    sbit load_out_en=P2^7;
#else
    bit port_out_en=0xa5,sr_out_en=0xa6,load_out_en=0xa7;
#endif

extern int lcd_init(void);
extern int aprintf(char *format,...);
extern int ascanf (char *format,char lenth);
extern int eprom(char address);
extern int write_eprom(int address,int Data);
extern int set_eprom(int command);
extern void cursor(char on);
extern ad_get(char ch);

delay(int time)
{
    int i,j;
    for(i=0;i<time;i++)for(j=0;j<DELAY_SCALE;j++);
}

/*键盘扫描*/
char key()
{
    const char key_tab[15]={211,224,208,176,
                        225,209,177,226,
                        210,178,114,112,
                        113,179,255};
    unsigned char i[4]={0xfe,0xfd,0xfb,0xf7};
    char j,k;
    char key;
    key=0xff;
    for(j=0;j<4;j++){
        P1=i[j];
        k=P1|0xf;
        if(k!=0xff){
            delay(20);
            if(k==P1|0xf)key=(k&0xf0)+j;
            break;
        }
    }
    for(j=0;j<15;j++){
        if(key_tab[j]==key){
            key=j;
            time=0;
            return key;
        }
    }
    return NO;
}

char wait_key()
{
	char k;
	while(key()!=NO&&time<5);
	while((k=key())==NO && time<5);
	return k;
}	
	
init()
{
    char i;
    P0=sr_out=load_out=0;
    load_out_en=1;
    load_out_en=0;
    sr_out_en=1;
    sr_out_en=0;
    P0=0x20;
    port_out_en=1;
    port_out_en=0;
    /*  定时器0初始化    */

    /*  LCD显示器初始化 */
    lcd_init();
    /*  系统状态判别    */
#if MODEL==70    
    aprintf("\b   JRT-48-100");
#endif
#if MODEL==71
	aprintf("\b   JRT-24-100");
#endif
	delay(1000);	    
    if(eprom(SYSTEM_FLAG)!=MODEL){
        aprintf("\bSYSTEM ERROR!\nPlease Resetup!");
        wait_key();
        set_eprom(EWEN);
        write_eprom(BOOST,BOOST_DEFAULT_VALUE);
        write_eprom(TAPER,TAPER_DEFAULT_VALUE);
        write_eprom(FLOAT_MAX,FLOAT_MAX_DEFAULT_VALUE);
        write_eprom(FLOAT_MIN,FLOAT_MIN_DEFAULT_VALUE);
        write_eprom(DELAY_CHANGE,DELAY_CHANGE_DEFAULT_VALUE);
        write_eprom(BOOST_BELOW,BOOST_BELOW_DEFAULT_VALUE);
        write_eprom(COMP_TEMP,COMP_TEMP_DEFAULT_VALUE);
        write_eprom(LCD_TEMP,LCD_TEMP_DEFAULT_VALUE);
        write_eprom(BT_MIN,BT_MIN_DEFAULT_VALUE);
        write_eprom(BT_MAX,BT_MAX_DEFAULT_VALUE);
        write_eprom(BT_RELOAD,BT_RELOAD_DEFAULT_VALUE);
        write_eprom(DELAY_DISCONNECT,DELAY_DISCONNECT_DEFAULT_VALUE);
        write_eprom(DATA_INDEX,0);
        for(i=DATA;i<DATA+4*32;i++)write_eprom(i,0);
        write_eprom(SYSTEM_FLAG,MODEL);
        set_eprom(EWDS);
    }
    v[6]=eprom(BOOST);
    v[1]=eprom(TAPER);
    float_max=eprom(FLOAT_MAX);
    float_min=eprom(FLOAT_MIN);
    delay_change=eprom(DELAY_CHANGE);
    boost_below=eprom(BOOST_BELOW);
    comp_temp=eprom(COMP_TEMP);
    bt_min=eprom(BT_MIN);
    bt_max=eprom(BT_MAX);
    bt_reload=eprom(BT_RELOAD);
    delay_disconnect=eprom(DELAY_DISCONNECT);
}

int setup(char *message,int a,int max,int min)
{
    unsigned char k;
    cursor(ON);
    for(;time<5;){
        aprintf(message,a);
	if((k=wait_key())==NO)break;
        switch(k){
            case OK:cursor(OFF);return a;
            case INC:{
                if(a<max)a++;
                aprintf(message,a);
                while(key()==INC){
                    delay(500);
                    while(key()==INC){
                        if(a<max)a++;
                        aprintf(message,a);
                    }
                }
                break;
            }
            case DEC:{
                if(a>min)a--;
                aprintf(message,a);
                while(key()==DEC){
                    delay(500);
                    while(key()==DEC){
                        if(a>min)a--;
                        aprintf(message,a);
                    }
                }
                break;
            }
        }
    }
    cursor(OFF);
    return -1;
}

char password()
{
    unsigned char pass[12],i;
    aprintf("\bEnter Password:");
    if(ascanf(pass,8)==-1)return -1;
    for(i=0;i<8;i++){
        if(pass[i]!=eprom(PASSWORD+i)){
            aprintf("\bPassword Error!");
            return -1;
        }
    }
    return 0;
}

control_set()
{
    char k;
    int Boost,Taper,Float_max,Float_min,Boost_below,Comp_temp;
    char Banks,Lcd_temp,Delay_change;
    if(password()==-1)return -1;
    if((Boost=setup("\bMAXINUM BOOST\nVOLTAGE %oV",v[6],BOOST_MAX,BOOST_MIN))==-1)return -1;
    if((Taper=setup("\bBOOST TAPER AT\nVOLTAGE %oV",v[1],TAPER_MAX,TAPER_MIN))==-1)return -1;
    if((Float_max=setup("\bFLOAT MAXIMUM\nVOLTAGE %oV",float_max,FLOAT_MAX,FLOAT_MIN))==-1)return -1;
    if((Float_min=setup("\bFLOAT MINIMUM\nVOLTAGE %oV",float_min,FLOAT_MAX,FLOAT_MIN))==-1)return -1;
    if((Boost_below=setup("\bRETURN TO BOOST\nMODE BELOW %oV",boost_below,BOOST_BELOW_MAX,BOOST_BELOW_MIN))==-1)return -1;
    if((Comp_temp=setup("\bTEMPERATURE COMP\nAT - %dmV/C/CELL",comp_temp,COMP_TEMP_MAX,COMP_TEMP_MIN))==-1)return -1;
    if((Delay_change=setup("\bSTATE CHANGE\nDELAY %dMIN",delay_change,DELAY_CHANGE_MAX,DELAY_CHANGE_MIN))==-1)return -1;
    aprintf("\bSAVE CHANGE?\nINC=YES OTHER=NO");
    if(wait_key()==INC){
        v[6]=Boost;
        v[1]=Taper;
        v[2]=(4*v[1]+v[6])/5;
        v[3]=(3*v[1]+2*v[6])/5;
        v[4]=(2*v[1]+3*v[6])/5;
        v[5]=(v[1]+4*v[6])/5;
        float_max=Float_max;
        float_min=Float_min;
        boost_below=Boost_below;
        comp_temp=Comp_temp;
        delay_change=Delay_change;
    }
}

load_set()
{
    int Bt_min,Bt_max,Bt_reload,Delay_disconect;
    if(password()==-1)return -1;
    if((Bt_min=setup("\bLOW BATTERY AL-\nARM ON AT %oV",bt_min,BT_MIN_MAX,BT_MIN_MIN))==-1)return -1;
    if((Bt_reload=setup("\bRECONECT LOAD\nABOVE %oV",bt_reload,BT_RELOAD_MAX,BT_RELOAD_MIN))==-1)return -1;
    if((Bt_max=setup("\bHIGH BATTERY AL-\nARM ON AT %oV",bt_max,BT_MAX_MAX,BT_MAX_MIN))==-1)return -1;
    if((Delay_disconect=setup("\bDELAY BEFORE\nDISCONECT %dS",delay_disconnect,DELAY_DIS_MAX,DELAY_DIS_MIN))==-1)return -1;
    aprintf("\bSave Change?\nINC=YES OTHER=NO");
    if(wait_key()==INC){
        bt_min=Bt_min;
        bt_max=Bt_max;
        bt_reload=Bt_reload;
        delay_disconnect=Delay_disconect;
    }
}

remote_set()
{
    char *prompt[]={"OFF","ON"};
    char Phone[12];
    char Local;
    bit Upload;
    char k;
    if(password()==-1)return -1;
    Upload=eprom(AUTO_UPLOAD);
    Local=eprom(LOCAL);
    for(k=0;k<12;k++)Phone[k]=eprom(PHONE+k);
    do{
        aprintf("\bAuto Upload\n[%s]",prompt[k=Upload]);
        if((k=wait_key())==NO)return -1;
        switch(k){
            case INC:
            case DEC:Upload=~Upload;
        }
    }while(k!=OK);
    if(Upload){
        if((Local=setup("\bSit Number\n%d",eprom(LOCAL),255,0))==-1)return -1;
        aprintf("\bRemote Phone:\n%s",Phone);
        for(k=0;k<12;k++)Phone[k]=0;
        if(ascanf(Phone,12)==-1)return -1;
    }
    aprintf("\bSave Change?\nINC=YES OTHER=NO");
    if(wait_key()==INC){
        write_eprom(AUTO_UPLOAD,Upload);
        write_eprom(LOCAL,Local);
        for(k=0;k<12;k++)write_eprom(PHONE+k,Phone[k]);
    }
}

int get_result(char a)
{
    please=a;
    while(please);
    return ad_result;
}

char state(char a)
{
    return (sr_out>>a)&1;
}

solar()
{
    char *prompt[]={"OFF","ON"};
    int bank;
    char k;
    time=bank=0;
    for(;time<5;){
        aprintf("\bBANK%d   %oV\n[%s]     %oA",bank,get_result(SR_V0+bank),prompt[state(bank)],get_result(SR_I0+bank));
        switch(wait_key()){
            case SOLAR:
            case INC:bank=(bank+1)%6;break;
            case DEC:if(bank==0)bank=5;else bank--;break;
            case NO:return -1;
            default:return 0;
        }
    }
}

view()
{
    char k,address;
    int i;
    i=time=k=0;
    for(;time<5;){
        address=((i+eprom(DATA_INDEX))%32)*4+DATA;
        aprintf("\bD%d  %oBV~%oV\nCHR %d  DIS %d",i,eprom(address),eprom(address+1),eprom(address+2)/3600,eprom(address+3)/3600);
        switch(wait_key()){
        	case INC:i=(i+1)%32;break;
        	case DEC:if(i==0)i=31;else i--;break;
        	case NO:return -1;
        	default:return 0;
        }
     }
}

testself()
{
    unsigned char k;
    int tmp,i;
    i=0;
    while(k!=OK){
        aprintf("\bTest Mode %c",i+'A');
        k=wait_key();
        if(k==-1 || k==CANCEL)return -1;
        if(k==INC || k==DEC)if(i==0)i=1;else i=0;
    }
    if(i==0){
        delay_change=delay_disconnect=2;
        while(time<60 && key()!=CANCEL){
            aprintf("\bTEST A    %c%c%c%c%c%c\nBattery: %oV",state(0),state(1),state(2),state(3),state(4),state(5),bt_v);
            delay(500);
        }
        delay_change=eprom(DELAY_CHANGE);
        delay_disconnect=eprom(DELAY_DISCONNECT);
    }
    else{
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
        return -1;
next:
        for(i=0;i<6;i++){
            EA=0;
            sr_out=1<<i;
            P0=sr_out;
            sr_out_en=1;
            sr_out_en=0;
            while(key()!=OK)aprintf("\bTEST B:BANK%c\n%oV  %oA",i,ad_get(SR_V0+i),ad_get(BT_I));
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
            port|=LAMP;
            P0=port;
            port_out_en=1;
            port_out_en=0;
            while( time<10){
                aprintf("\bBATTERY VOLTAGE\n          %oV",bt_v);
                switch(wait_key()){
                    case BT_V_KEY:{
                        aprintf("\bBATTERY VOLTAGE\n          %oV",bt_v);
                        break;
                    }
                    case BT_I_KEY:{
                        aprintf("\bCHARGE CURRENT\n          %oA",bt_i);
                        break;
                    }
                    case LOAD_V_KEY:{
                        aprintf("\bLOAD VOLTAGE\n          %oV",get_result(LOAD_V));
                        break;
                    }
                    case LOAD_I_KEY:{
                        aprintf("\bLOAD CURRENT\n          %oA",load_i);
                        break;
                    }
                    case SOLAR:solar();break;
                    case CONTROL_SET:control_set();break;
                    case TESTSELF:testself();break;
                    case REMOTE_SET:remote_set();break;
                    case LOAD_SET:load_set();break;
                    case VIEW:view();break;
                }
            }
            lcd_close();
            port&=~LAMP;
            P0=port;
            port_out_en=1;
            port_out_en=0;
        }
    }
}

