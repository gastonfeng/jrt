unit Main;

interface

uses SysUtils, Windows, Messages, Classes, Graphics, Controls,
  Forms, Dialogs, StdCtrls, Buttons, ExtCtrls, Menus, ComCtrls, ToolWin,
  DBTables, OleCtrls, graphsv3, Grids, TeEngine, TeeFunci,
  Series, TeeProcs, Chart, Mask,iniFiles, CPDrv;

type
  TMainForm = class(TForm)
    StatusLine: TStatusBar;
    ToolBar1: TToolBar;
    connectbtn: TSpeedButton;
    display: TPageControl;
    TabSheet1: TTabSheet;
    TabSheet2: TTabSheet;
    TabSheet3: TTabSheet;
    disconnectbtn: TSpeedButton;
    StringGrid1: TStringGrid;
    StringGrid2: TStringGrid;
    Chart1: TChart;
    Series2: TLineSeries;
    Series3: TLineSeries;
    Series4: TLineSeries;
    TeeFunction1: TAddTeeFunction;
    TeeFunction2: TAddTeeFunction;
    TeeFunction3: TAddTeeFunction;
    TeeFunction4: TAddTeeFunction;
    Series1: TLineSeries;
    setupbtn: TSpeedButton;
    exitbtn: TSpeedButton;
    aboutbtn: TSpeedButton;
    savebtn: TSpeedButton;
    SaveDialog1: TSaveDialog;
    CommPortDriver1: TCommPortDriver;
    refresh: TSpeedButton;
    procedure connect(Sender: TObject);
    procedure HelpAboutItemClick(Sender: TObject);
    procedure applycommsettings;
    procedure exitbtnClick(Sender: TObject);
    procedure disconnectbtnClick(Sender: TObject);
    procedure CommPortDriver1ReceivePacket(Sender: TObject;
      Packet: Pointer; DataSize: Cardinal);
    procedure FormActivate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure savebtnClick(Sender: TObject);
    procedure refreshClick(Sender: TObject);
    procedure TabSheet1Show(Sender: TObject);
    procedure TabSheet2Show(Sender: TObject);
  end;
 type now=record
     btv:smallint;
     sv:array[1..6]of smallint;
     loadv:smallint;
     temp:smallint;
     zz:smallint;
     loadi:smallint;
     bti:smallint;
     sri0:smallint;
     sri1:smallint;
     sri2:smallint;
     sri3:smallint;
     sri4:smallint;
     sri5:smallint;
     maxvoltage:smallint;
     minvoltage:smallint;
     chargeah:longint;
     dischargeah:longint;
     min:shortint;
     hour:shortint;
     status:shortint;
     load:shortint;
     BOOST:smallint;
     TAPER:smallint;
     FLOAT_MAX_S:smallint;
     FLOAT_MIN_S:smallint;
     BOOST_BELOW:smallint;
     COMP_TEMP:smallint;
     DELAY_CHANGE:smallint;
     BT_MIN:smallint;
     BT_RELOAD:smallint;
     BT_MAX:smallint;
     DELAY_DISCONNECT:smallint;
     LOCAL:smallint;
     eHOUR:smallint;
     MINUTE:smallint;
     AUTO_UPLOAD:smallint;
     UP_HOUR:smallint;
     UP_MIN:smallint;
     checksum:smallint;
end;
var
  MainForm: TMainForm;
  inifile:TIniFile;
  RECeIVEFLAG:integer;
  Connected:boolean;
  autostarted,MODEMCONNECT:boolean;
  mypath:string;
  nowrec:^now;

CONST
     DIAL=1;
     ASKSITE=2;
     CURRENT=3;
     HISTORY=4;
     SETUPDATA=5;
     INIT=0;

implementation
{$R *.DFM}

uses SiteUnit, about, CoverUnit;

procedure TMainForm.connect(Sender: TObject);
begin
//    siteform:=tsiteform.create(application);
    siteform.ShowModal;
end;


procedure TMainForm.HelpAboutItemClick(Sender: TObject);
begin
  showaboutbox(application);
end;

procedure tmainform.applycommsettings;
begin
  inifile:=Tinifile.create(mypath+'SPS.ini');
  with inifile do
  begin
    commportdriver1.port:=tportnumber(inifile.readinteger('PORT','COM',1)+1);
    commportdriver1.BaudRate:=tbaudrate(inifile.readinteger('PORT','BAUDRATE',12)+1);
  end;
  inifile.free;
  commportdriver1.databits:=db8bits;
  commportdriver1.parity:=ptnone;
  commportdriver1.hwflow:=hfnone;
  commportdriver1.swflow:=sfnone;
  commportdriver1.CheckLineStatus:=TRUE;
  commportdriver1.EnableDTROnOpen:=true;
end;

procedure TMainForm.exitbtnClick(Sender: TObject);
begin
disconnectbtnclick(application);
close;
end;


procedure TMainForm.disconnectbtnClick(Sender: TObject);
begin
     commportdriver1.SendChar('3');
     refresh.Enabled:=false;
     commportdriver1.Disconnect;
     connectbtn.Enabled:=true;
     disconnectbtn.enabled:=false;
     SETUPBtn.Enabled:=true;
     connected:=false;
end;

procedure TMainForm.CommPortDriver1ReceivePacket(Sender: TObject;
  Packet: Pointer; DataSize: Cardinal);

  type tresult=array[0..260] of byte;
var
  p:^tresult;
  i:integer;
begin
     messagebeep(0);
  case receiveflag of
    current:
    begin
      if datasize<>88 then exit;
      nowrec:=packet;
      mainform.StringGrid1.Cells[1,1]:=floattostr(nowrec.btv/10)+'V';
      mainform.StringGrid1.cells[1,2]:=floattostr(nowrec.loadv/10)+'V';
      mainform.StringGrid1.Cells[1,3]:=inttostr(nowrec.temp+20)+'��';
      mainform.StringGrid1.cells[1,4]:=floattostr(nowrec.bti/10)+'A';
      mainform.StringGrid1.cells[1,5]:=floattostr(nowrec.loadi/10)+'A';
      mainform.StringGrid1.Cells[1,6]:=floattostr(nowrec.sri0/10)+'A';
      mainform.StringGrid1.cells[1,7]:=floattostr(nowrec.sri1/10)+'A';
      mainform.StringGrid1.Cells[1,8]:=floattostr(nowrec.sri2/10)+'A';
      mainform.StringGrid1.cells[1,9]:=floattostr(nowrec.sri3/10)+'A';
      mainform.StringGrid1.cells[1,10]:=floattostr(nowrec.sri4/10)+'A';
      mainform.StringGrid1.Cells[1,11]:=floattostr(nowrec.sri5/10)+'A';
      mainform.StringGrid1.cells[1,12]:=floattostr(nowrec.maxvoltage/10)+'V';
      mainform.StringGrid1.Cells[1,13]:=floattostr(nowrec.minvoltage/10)+'V';
      mainform.StringGrid1.cells[1,14]:=floattostr(nowrec.chargeah/36000)+'Ah';
      mainform.StringGrid1.cells[1,15]:=floattostrf(nowrec.dischargeah/36000,ffGeneral,2,2)+'Ah';
      mainform.StringGrid1.Cells[1,16]:=inttostr(nowrec.hour)+':'+inttostr(nowrec.min);
      mainform.StringGrid1.cells[1,17]:=inttostr(nowrec.load);
      mainform.StringGrid1.Cells[1,18]:=inttostr(nowrec.status);
      stringgrid1.Cells[4,1]:=floattostr(nowrec.boost/10)+'V';
      stringgrid1.Cells[4,2]:=floattostr(nowrec.taper/10)+'V';
      stringgrid1.cells[4,3]:=floattostr(nowrec.float_max_s/10)+'V';
      stringgrid1.Cells[4,4]:=floattostr(nowrec.float_min_s/10)+'V';
      stringgrid1.Cells[4,5]:=floattostr(nowrec.boost_below/10)+'V';
      stringgrid1.Cells[4,6]:='-'+floattostr(nowrec.comp_temp)+'mV';
      stringgrid1.Cells[4,7]:=floattostr(nowrec.bt_max/10)+'V';
      stringgrid1.Cells[4,8]:=floattostr(nowrec.bt_min/10)+'V';
      stringgrid1.Cells[4,9]:=floattostr(nowrec.bt_reload/10)+'V';
      stringgrid1.Cells[4,10]:=floattostr(nowrec.delay_disconnect)+'S';
      stringgrid1.Cells[4,11]:=floattostr(nowrec.delay_change)+'S';
      refresh.enabled:=true;
    end;
    history:
    begin
      if datasize<>260 then exit;
      p:=packet;
      mainform.Series1.Clear;
      mainform.Series2.Clear;
      mainform.Series3.Clear;
      mainform.Series4.Clear;
      for i:=0 to 31 do
      begin
        mainform.StringGrid2.Cells[0,i+2]:=inttostr(i);
        mainform.StringGrid2.cells[1,i+2]:=floattostr((p[i*8+2]+p[i*8+3]*256)/10);
        mainform.Series1.Add((p[i*8+2]+p[i*8+3]*256)/10,'',clteecolor);
        mainform.StringGrid2.Cells[2,i+2]:=floattostr((p[4+i*8]+p[5+i*8]*256)/10);
        mainform.Series2.Add((p[4+i*8]+p[5+i*8]*256)/10,'',clteecolor);
        mainform.StringGrid2.cells[3,i+2]:=inttostr(p[6+i*8]+p[7+i*8]*256);
        mainform.Series3.Add(p[6+i*8]+p[7+i*8]*256,'',clteecolor);
        mainform.StringGrid2.cells[4,i+2]:=inttostr(p[8+i*8]+p[9+i*8]*256);
        mainform.Series4.Add(p[8+i*8]+p[9+i*8]*256,'',clteecolor);
      end;
      refresh.enabled:=true;
      savebtn.Enabled:=true;
    end;
  end;
end;

procedure TMainForm.FormActivate(Sender: TObject);
begin
  mypath:=extractfilepath(application.exename);
  mainform.StringGrid1.Cells[0,1]:='���ص�ѹ';
  mainform.StringGrid1.Cells[0,2]:='���ص�ѹ';
  mainform.StringGrid1.Cells[0,3]:='�����¶�';
  mainform.StringGrid1.Cells[0,4]:='������';
  mainform.StringGrid1.Cells[0,5]:='�ŵ����';
  mainform.StringGrid1.Cells[0,6]:='����1������';
  mainform.StringGrid1.Cells[0,7]:='����2������';
  mainform.StringGrid1.Cells[0,8]:='����3������';
  mainform.StringGrid1.Cells[0,9]:='����4������';
  mainform.StringGrid1.Cells[0,10]:='����5������';
  mainform.StringGrid1.Cells[0,11]:='����6������';
  mainform.StringGrid1.Cells[0,12]:='������ߵ�ѹ';
  mainform.StringGrid1.Cells[0,13]:='������͵�ѹ';
  mainform.StringGrid1.Cells[0,14]:='���������';
  mainform.StringGrid1.Cells[0,15]:='����ŵ����';
  mainform.StringGrid1.Cells[0,16]:='������ʱ��';
  mainform.StringGrid1.Cells[0,17]:='���״̬';
  mainform.StringGrid1.Cells[0,18]:='���״̬';
  stringgrid1.Cells[3,1]:='ǿ������ѹ';
  stringgrid1.Cells[3,2]:='ǿ��ݼ���ѹ';
  stringgrid1.cells[3,3]:='�����ѹ����';
  stringgrid1.Cells[3,4]:='�����ѹ����';
  stringgrid1.Cells[3,5]:='����ǿ���ѹ';
  stringgrid1.Cells[3,6]:='�¶Ȳ�����ѹ';
  stringgrid1.Cells[3,7]:='�澯��ѹ����';
  stringgrid1.Cells[3,8]:='�澯��ѹ����';
  stringgrid1.Cells[3,9]:='Ƿѹ�澯�����ѹ';
  stringgrid1.Cells[3,10]:='�澯ǰ��ʱ';
  stringgrid1.Cells[3,11]:='���״̬ת����ʱ';
  mainform.StringGrid2.Cells[0,1]:='����';
  mainform.StringGrid2.Cells[1,1]:='������ߵ�ѹ';
  mainform.StringGrid2.Cells[2,1]:='������͵�ѹ';
  mainform.StringGrid2.Cells[3,1]:='���������';
  mainform.StringGrid2.Cells[4,1]:='����ŵ����';;
  CoverForm.Hide;
  CoverForm.free;
  with tinifile.Create(mypath+'SPS.INI') do
  begin
    autostarted:=readbool('START','START',TRUE);
    if autostarted then
    begin
      siteform:=tsiteform.create(application);
      siteform.ShowModal;
    end;
    free;
  end;
end;

procedure TMainForm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  with tinifile.Create(mypath+'SPS.INI') do
  begin
    writebool('START','START',autostarted);
    free;
  end;
end;

procedure TMainForm.savebtnClick(Sender: TObject);
var f1:Textfile;
    i:integer;
begin
  savedialog1.InitialDir:=mypath;
  savedialog1.Execute;
  Assignfile(f1,savedialog1.filename);
  rewrite(f1);
  writeln(f1,'<��ǰ״̬��¼>');
  writeln(f1,'��������ʱ��:'+stringgrid1.cells[1,16]);
  writeln(f1,'���ص�ѹ = '+stringgrid1.cells[1,1]);
  writeln(f1,'���س����� = '+stringgrid1.cells[1,4]);
  writeln(f1,'���ص�ѹ = '+stringgrid1.cells[1,2]);
  writeln(f1,'���ص��� = '+stringgrid1.cells[1,5]);
  writeln(f1,'�����¶� = '+stringgrid1.cells[1,3]);
  writeln(f1,'���״̬ = '+stringgrid1.cells[1,17]);
  writeln(f1,'���״̬ = '+stringgrid1.cells[1,18]);
  writeln(f1,'̫���ܵ�ص�1����� = '+stringgrid1.cells[1,6]);
  writeln(f1,'̫���ܵ�ص�2����� = '+stringgrid1.cells[1,7]);
  writeln(f1,'̫���ܵ�ص�3����� = '+stringgrid1.cells[1,8]);
  writeln(f1,'̫���ܵ�ص�4����� = '+stringgrid1.cells[1,9]);
  writeln(f1,'̫���ܵ�ص�5����� = '+stringgrid1.cells[1,10]);
  writeln(f1,'̫���ܵ�ص�6����� = '+stringgrid1.cells[1,11]);
  writeln(f1,'�����ۼƳ����� = '+stringgrid1.cells[1,14]);
  writeln(f1,'�����ۼƷŵ���� = '+stringgrid1.cells[1,15]);
  writeln(f1,'����������ߵ�ѹ = '+stringgrid1.cells[1,12]);
  writeln(f1,'����������͵�ѹ = '+stringgrid1.cells[1,13]);
  writeln(f1,'<����״̬��¼>');
  writeln(f1,'ǿ������ѹ = '+stringgrid1.cells[4,1]);
  writeln(f1,'ǿ��ݼ���ѹ = '+stringgrid1.cells[4,2]);
  writeln(f1,'�����ѹ���� = '+stringgrid1.cells[4,3]);
  writeln(f1,'�����ѹ���� = '+stringgrid1.cells[4,4]);
  writeln(f1,'����ǿ���ѹ = '+stringgrid1.cells[4,5]);
  writeln(f1,'�¶Ȳ�����ѹ = '+stringgrid1.cells[4,6]);
  writeln(f1,'�澯��ѹ���� = '+stringgrid1.cells[4,7]);
  writeln(f1,'�澯��ѹ���� = '+stringgrid1.cells[4,8]);
  writeln(f1,'���������ӵ�ѹ = '+stringgrid1.cells[4,9]);
  writeln(f1,'�Ͽ�����ǰ��ʱ = '+stringgrid1.cells[4,10]);
  writeln(f1,'״̬ת����ʱ = '+stringgrid1.cells[4,11]);
  writeln(f1,'<��ʷ���ݼ�¼>');
  writeln(f1,'����'+chr(9)+'��ߵ�ѹ'+chr(9)+'��͵�ѹ'+chr(9)+'������'+chr(9)+'�ŵ����');
	for i:=0 to 31 do
        writeln(f1,'��'+inttostr(i)+'��'+chr(9)+chr(9)+mainform.StringGrid2.cells[1,i+2]+'V'+chr(9)+chr(9)+mainform.StringGrid2.cells[2,i+2]+'V'+chr(9)+chr(9)+mainform.StringGrid2.cells[3,i+2]+'AH'+chr(9)+chr(9)+mainform.StringGrid2.cells[4,i+2]+'AH');
  closefile(f1);
end;

procedure TMainForm.refreshClick(Sender: TObject);
begin
  if Connected then
  begin
    if receiveflag=CURRENT then
    begin
      CommPortDriver1.PacketSize:=88;
      CommPortDriver1.PacketTimeout:=60000;
      CommPortDriver1.SendByte(10);
      refresh.enabled:=false;
    end
    else
    begin
      CommPortDriver1.packetsize:=260;
      CommPortDriver1.PacketTimeout:=60000;
      CommPortDriver1.SendByte(72);
      receiveflag:=history;
      refresh.enabled:=false;
    end;
  end
  else
  begin
    connectbtn.Enabled:=true;
    disconnectbtn.Enabled:=false;
  end;
end;

procedure TMainForm.TabSheet1Show(Sender: TObject);
begin
  receiveflag:=CURRENT;
end;

procedure TMainForm.TabSheet2Show(Sender: TObject);
begin
  receiveflag:=HISTORY;
end;

end.
