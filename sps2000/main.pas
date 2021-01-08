unit Main;

interface

uses SysUtils, Windows, Messages, Classes, Graphics, Controls,
  Forms, Dialogs, StdCtrls, Buttons, ExtCtrls, Menus, ComCtrls, ToolWin,
  DBTables, OleCtrls, graphsv3, Grids, TeEngine, TeeFunci,
  Series, TeeProcs, Chart, Mask,iniFiles, CPDrv, TAPIcomp;

  procedure sendreq(ch:byte);

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
    exitbtn: TSpeedButton;
    aboutbtn: TSpeedButton;
    savebtn: TSpeedButton;
    SaveDialog1: TSaveDialog;
    CommPortDriver1: TCommPortDriver;
    refresh: TSpeedButton;
    Timer1: TTimer;
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
    procedure StringGrid1SelectCell(Sender: TObject; ACol, ARow: Integer;
      var CanSelect: Boolean);
    procedure Timer1Timer(Sender: TObject);
    procedure TabSheet1Show(Sender: TObject);
  end;

type variableitem=record
  site:byte;
  name:string;
  unitname:string;
  scale:integer;
  col,row:integer;
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
//  RECeIVEFLAG:integer;
  Connected:boolean;
  autostarted,MODEMCONNECT:boolean;
  mypath:string;
//  nowrec:^now;
  timeout:integer;
  req:boolean;
  ch:byte;
  variablelist:array[1..100] of variableitem;
  sending:boolean;

CONST
     DIAL=1;
     ASKSITE=2;
     CURRENT=3;
     HISTORY=4;
     SETUPDATA=5;
     INIT=0;
     timeoutvalue=100;

implementation
{$R *.DFM}

uses SiteUnit, about, CoverUnit, modi;

procedure TMainForm.connect(Sender: TObject);
begin
    siteform:=tsiteform.create(application);
    siteform.ShowModal;
end;


procedure TMainForm.HelpAboutItemClick(Sender: TObject);
begin
  showaboutbox(application);
end;

procedure tmainform.applycommsettings;
begin
//  inifile:=Tinifile.create(mypath+'SPS.ini');
//  with inifile do
//  begin
    commportdriver1.port:=tportnumber(siteform.portcombobox.itemindex+1);//inifile.readinteger('PORT','COM',1)+1);
    commportdriver1.BaudRate:=tbaudrate(7);//inifile.readinteger('PORT','BAUDRATE',12)+1);
//  end;
//  inifile.free;
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
//     commportdriver1.SendChar('3');
  timer1.Enabled:=false;
     commportdriver1.Disconnect;
       connectbtn.Enabled:=true;
       disconnectbtn.enabled:=false;
//     SETUPBtn.Enabled:=true;
       connected:=false;
       refresh.Enabled:=false;
end;

procedure TMainForm.CommPortDriver1ReceivePacket(Sender: TObject;
  Packet: Pointer; DataSize: Cardinal);

  type tresult=record
    begincode:byte;
    varicode:byte;
    variable:smallint;
    checksum:byte;
  end;
  type bb=record //array[1..5] of byte;
    b1,b2,b3,b4,b5:byte;
  end;
var
  p:^tresult;
  bbb:^bb;
  te:byte;
begin
//     messagebeep(0);
     if Datasize<>5 then
     begin
//       reply:=true;
       exit;
     end;
  bbb:=Packet;
//     te:=bbb[4]+bbb[1]+bbb[2]+bbb[3];
     te:=byte(bbb.b1+bbb.b2+bbb.b3+bbb.b4);
     if te<>bbb.b5 then exit;
     p:=Packet;
     if p.varicode>60 then
     begin
       stringgrid2.Cells[((p.varicode-68) mod 4)+1,(p.varicode - 64) div 4]:=floattostr(p.variable/10);
       if (p.varicode-68)mod 4=0 then series1.Add(p.variable/10,'',clteecolor);
       if (p.varicode-68)mod 4=1 then series2.Add(p.variable/10,'',clteecolor);
       if (p.varicode-68)mod 4=2 then series3.Add(p.variable/10,'',clteecolor);
       if (p.varicode-68)mod 4=3 then series4.Add(p.variable/10,'',clteecolor);
       if p.varicode=195 then savebtn.Enabled:=true;
       exit;
     end;
     for te:=1 to 100 do
     begin
       if variablelist[te].site=p.varicode then break;
       if variablelist[te].site=$ff then break;
     end;
     if te<100 then
     stringgrid1.Cells[variablelist[te].col+2,variablelist[te].row]:=floattostr(p.variable/variablelist[te].scale);
 //    case p.varicode of
 //      0..21:stringgrid1.Cells[1,p.varicode+1]:=inttostr(p.variable);
 //      $20..$2e:stringgrid1.Cells[4,p.varicode-$20]:=inttostr(p.variable);
 //      67..127:stringgrid2.Cells[p.varicode mod 4,p.varicode div 4]:=inttostr(p.variable);
//     end;
     inc(ch);
     if variablelist[ch].site=$ff then
     begin
       timer1.Enabled:=false;
       ch:=1;
       sendreq(67);
     end;
     req:=false;
end;

procedure TMainForm.FormActivate(Sender: TObject);
var
  i:integer;
begin
  mypath:=extractfilepath(application.exename);
  stringgrid1.Cells[0,0]:='��������';
  stringgrid1.Cells[1,0]:='��λ';
  stringgrid1.Cells[2,0]:='��ֵ';
  stringgrid1.Cells[4,0]:='���ò�������';
  stringgrid1.Cells[5,0]:='��λ';
  stringgrid1.Cells[6,0]:='��ֵ(���������޸�)';
  stringgrid2.Cells[0,0]:='���������';
  stringgrid2.Cells[1,0]:='������ߵ�ѹ(V)';
  stringgrid2.Cells[2,0]:='������͵�ѹ(V)';
  stringgrid2.Cells[3,0]:='���ճ�����(AH)';
  stringgrid2.Cells[4,0]:='���շŵ����(AH)';
  for i := 1 to 32 do
  begin
    stringgrid2.Cells[0,i]:=inttostr(i-1);
  end;
//  mainform.StringGrid1.Cells[0,1]:='���ص�ѹ';
//  stringgrid1.Cells[1,1]:='V';
//  mainform.StringGrid1.Cells[0,2]:='���ص�ѹ';
//  mainform.StringGrid1.Cells[0,3]:='�����¶�';
//  mainform.StringGrid1.Cells[0,4]:='������';
//  mainform.StringGrid1.Cells[0,5]:='�ŵ����';
//  mainform.StringGrid1.Cells[0,6]:='����1������';
//  mainform.StringGrid1.Cells[0,7]:='����2������';
//  mainform.StringGrid1.Cells[0,8]:='����3������';
//  mainform.StringGrid1.Cells[0,9]:='����4������';
//  mainform.StringGrid1.Cells[0,10]:='����5������';
 // mainform.StringGrid1.Cells[0,11]:='����6������';
//  mainform.StringGrid1.Cells[0,12]:='������ߵ�ѹ';
//  mainform.StringGrid1.Cells[0,13]:='������͵�ѹ';
 // mainform.StringGrid1.Cells[0,14]:='���������';
//  mainform.StringGrid1.Cells[0,15]:='����ŵ����';
//  mainform.StringGrid1.Cells[0,16]:='������ʱ��';
//  mainform.StringGrid1.Cells[0,17]:='���״̬';
//  mainform.StringGrid1.Cells[0,18]:='���״̬';
//  stringgrid1.Cells[3,1]:='ǿ������ѹ';
//  stringgrid1.Cells[3,2]:='ǿ��ݼ���ѹ';
//  stringgrid1.cells[3,3]:='�����ѹ����';
//  stringgrid1.Cells[3,4]:='�����ѹ����';
 // stringgrid1.Cells[3,5]:='����ǿ���ѹ';
//  stringgrid1.Cells[3,6]:='�¶Ȳ�����ѹ';
//  stringgrid1.Cells[3,7]:='�澯��ѹ����';
//  stringgrid1.Cells[3,8]:='�澯��ѹ����';
//  stringgrid1.Cells[3,9]:='Ƿѹ�澯�����ѹ';
//  stringgrid1.Cells[3,10]:='�澯ǰ��ʱ';
//  stringgrid1.Cells[3,11]:='���״̬ת����ʱ';
//  mainform.StringGrid2.Cells[0,1]:='����';
//  mainform.StringGrid2.Cells[1,1]:='������ߵ�ѹ';
//  mainform.StringGrid2.Cells[2,1]:='������͵�ѹ';
//  mainform.StringGrid2.Cells[3,1]:='���������';
//  mainform.StringGrid2.Cells[4,1]:='����ŵ����';;
  with tinifile.Create(mypath+'variable.ini') do
  begin
    for i:=1 to 100 do
    begin
      variablelist[i].site:=readinteger(inttostr(i),'SITE',$FF);
      if variablelist[i].site=$ff then break;
      variablelist[i].name:=readstring(inttostr(i),'VARIABLE NAME','');
      variablelist[i].unitname:=readstring(inttostr(i),'UNIT','');
      variablelist[i].scale:=readinteger(inttostr(i),'SCALE',1);
      variablelist[i].col:=readinteger(inttostr(i),'COL',0);
      variablelist[i].row:=readinteger(inttostr(i),'ROW',0);
      stringgrid1.cells[variablelist[i].col,variablelist[i].row]:=variablelist[i].name;
      stringgrid1.Cells[variablelist[i].col+1,variablelist[i].row]:=variablelist[i].unitname;
    end;
    free;
  end;
  CoverForm.Hide;
  CoverForm.free;
  ch:=1;
  sending:=false;
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
  for i:=1 to 100 do
  begin
    if variablelist[i].site=$ff then break;
    writeln(f1,variablelist[i].name+' = '+stringgrid1.Cells[variablelist[i].col+2,variablelist[i].row]+' '+variablelist[i].unitname);
  end;
//  writeln(f1,'��������ʱ��:'+stringgrid1.cells[1,16]);
//  writeln(f1,'���ص�ѹ = '+stringgrid1.cells[1,1]);
//  writeln(f1,'���س����� = '+stringgrid1.cells[1,4]);
//  writeln(f1,'���ص�ѹ = '+stringgrid1.cells[1,2]);
//  writeln(f1,'���ص��� = '+stringgrid1.cells[1,5]);
//  writeln(f1,'�����¶� = '+stringgrid1.cells[1,3]);
//  writeln(f1,'���״̬ = '+stringgrid1.cells[1,17]);
//  writeln(f1,'���״̬ = '+stringgrid1.cells[1,18]);
//  writeln(f1,'̫���ܵ�ص�1����� = '+stringgrid1.cells[1,6]);
//  writeln(f1,'̫���ܵ�ص�2����� = '+stringgrid1.cells[1,7]);
//  writeln(f1,'̫���ܵ�ص�3����� = '+stringgrid1.cells[1,8]);
//  writeln(f1,'̫���ܵ�ص�4����� = '+stringgrid1.cells[1,9]);
//  writeln(f1,'̫���ܵ�ص�5����� = '+stringgrid1.cells[1,10]);
//  writeln(f1,'̫���ܵ�ص�6����� = '+stringgrid1.cells[1,11]);
//  writeln(f1,'�����ۼƳ����� = '+stringgrid1.cells[1,14]);
//  writeln(f1,'�����ۼƷŵ���� = '+stringgrid1.cells[1,15]);
//  writeln(f1,'����������ߵ�ѹ = '+stringgrid1.cells[1,12]);
//  writeln(f1,'����������͵�ѹ = '+stringgrid1.cells[1,13]);
//  writeln(f1,'<����״̬��¼>');
//  writeln(f1,'ǿ������ѹ = '+stringgrid1.cells[4,1]);
//  writeln(f1,'ǿ��ݼ���ѹ = '+stringgrid1.cells[4,2]);
//  writeln(f1,'�����ѹ���� = '+stringgrid1.cells[4,3]);
//  writeln(f1,'�����ѹ���� = '+stringgrid1.cells[4,4]);
//  writeln(f1,'����ǿ���ѹ = '+stringgrid1.cells[4,5]);
//  writeln(f1,'�¶Ȳ�����ѹ = '+stringgrid1.cells[4,6]);
//  writeln(f1,'�澯��ѹ���� = '+stringgrid1.cells[4,7]);
//  writeln(f1,'�澯��ѹ���� = '+stringgrid1.cells[4,8]);
//  writeln(f1,'���������ӵ�ѹ = '+stringgrid1.cells[4,9]);
//  writeln(f1,'�Ͽ�����ǰ��ʱ = '+stringgrid1.cells[4,10]);
//  writeln(f1,'״̬ת����ʱ = '+stringgrid1.cells[4,11]);
  writeln(f1,'');
  writeln(f1,'<��ʷ���ݼ�¼>');
  writeln(f1,'����'+chr(9)+chr(9)+'��ߵ�ѹ'+chr(9)+chr(9)+'��͵�ѹ'+chr(9)+chr(9)+'������'+chr(9)+chr(9)+'�ŵ����');
	for i:=0 to 31 do
	writeln(f1,'��'+inttostr(i)+'��'+chr(9)+chr(9)+mainform.StringGrid2.cells[1,i+1]+'V'+chr(9)+chr(9)+mainform.StringGrid2.cells[2,i+1]+'V'+chr(9)+chr(9)+mainform.StringGrid2.cells[3,i+1]+'AH'+chr(9)+chr(9)+mainform.StringGrid2.cells[4,i+1]+'AH');
  closefile(f1);
end;

procedure TMainForm.StringGrid1SelectCell(Sender: TObject; ACol,
  ARow: Integer; var CanSelect: Boolean);
begin
  if ACol<>6 then
  begin
    showmessage('���ֶβ��ɸ��ģ�');
  end
  else
  begin
    modiform:=tmodiform.Create(Application);
    modiform.label1.Caption:='��ǰ����:'+stringgrid1.Cells[ACol-2,ARow];
    modiform.label2.Caption:='��ǰֵ:'+stringgrid1.Cells[Acol,ARow]+stringgrid1.cells[ACol-1,ARow];
    modiform.edit1.text:=stringgrid1.Cells[Acol,ARow];
    modiform.tag:=ARow;
    timer1.Enabled:=false;
    modiform.showmodal;
  end;
end;

procedure sendreq(ch:byte);
var
  checksum:byte;
begin
  while sending=true do ;
  sending:=true;
  mainform.CommPortDriver1.SendByte($aa);
//  checksum:=$aa;
  mainform.CommPortDriver1.SendByte(1);
  checksum:=$ab;
  mainform.CommPortDriver1.SendByte(ch);
  checksum:=checksum+ch;
  mainform.CommPortDriver1.SendByte(0);
  mainform.CommPortDriver1.SendByte(0);
  mainform.CommPortDriver1.SendByte(checksum);
  sending:=false;
end;

procedure TMainForm.refreshClick(Sender: TObject);
begin
  if Connected then
  begin
    ch:=1;
    timer1.Enabled:=true;
    series1.Clear;
    series2.Clear;
    series3.Clear;
    series4.Clear;
//    sendreq(67);
  end;
end;


procedure TMainForm.Timer1Timer(Sender: TObject);
begin
  if Connected then
  begin
    if req=true then
    begin
      inc(timeout);
      if timeout>timeoutvalue then
      begin
//	showmessage('Timeout!');
	mainform.StatusLine.SimpleText:='��ʱ����.';
	req:=false;
	timeout:=0;
      end;
    end
    else
    begin
      sendreq(variablelist[ch].site);
      req:=true;
//      mainform.StatusLine.SimpleText:=pchar(variablelist[ch].name);
      timeout:=0;
    end;
  end;
end;

procedure TMainForm.TabSheet1Show(Sender: TObject);
begin
//  timer1.Enabled:=true;
end;

end.