unit SiteUnit;

interface

uses   SysUtils, Windows, Messages, Classes, Graphics, Controls,
  StdCtrls, Forms, DBCtrls, DB, DBGrids, DBTables, Grids, ExtCtrls,Dialogs,inifiles;

type
  Tsiteform = class(TForm)
    telephone: TComboBox;
    number: TComboBox;
    autostart: TCheckBox;
    teleradio: TRadioButton;
    numberradio: TRadioButton;
    GroupBox1: TGroupBox;
    GroupBox2: TGroupBox;
    inok: TButton;
    incancel: TButton;
    procedure inokClick(Sender: TObject);
    procedure incancelClick(Sender: TObject);
    procedure FormActivate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure numberChange(Sender: TObject);
    procedure telephoneChange(Sender: TObject);
    procedure autostartClick(Sender: TObject);
  private
    { private declarations }
  public
    { public declarations }
  end;

var
  siteform: Tsiteform;
implementation
{$R *.DFM}
uses main;

procedure Tsiteform.inokClick(Sender: TObject);
var
   ss:string[20];
   tt:char;
begin
  mainform.applycommsettings;
  if mainform.commportdriver1.connect then
  begin
    if mainform.commportdriver1.getlinestatus=[] then mainform.commportdriver1.checklinestatus:=true;
    mainform.statusline.simpletext:='已连接';
    if siteform.teleradio.Checked then
    begin
      mainform.CommPortDriver1.ToggleDTR(true);
      mainform.CommPortDriver1.ToggleRTS(true);
      mainform.CommPortDriver1.FlushBuffers(true,true);
      ss:=' ';
      mainform.CommPortdriver1.InputTimeout:=500;
      mainform.CommPortdriver1.PausePolling;
      mainform.CommPortDriver1.SendString('AT&F'+#13+#10);
      mainform.CommPortdriver1.ReadData(@ss[1],9);
      if strlicomp(@ss[1],'AT&F'+#$d+#$D+#$A+'OK',9)=0 then
      begin
        mainform.StatusLine.SimpleText:='正在拨号,请等待...';
        mainform.CommPortdriver1.SendString('ATDT'+siteform.telephone.Text+#13+#10);
        mainform.CommPortdriver1.ReadData(@ss[1],15);
        mainform.CommPortdriver1.InputTimeout:=60000;
        mainform.CommPortDriver1.FlushBuffers(true,true);
        mainform.CommPortdriver1.ReadData(@ss[1],9);
        mainform.CommPortDriver1.ContinuePolling;
        if strlicomp(@ss[1],#$D+#$A+'CONNECT',9)=0 then
        begin
          mainform.StatusLine.SimpleText:='成功建立连接!';
          connected:=true;
          mainform.CommPortDriver1.Sendbyte(255);
          mainform.commportdriver1.readchar(tt);
          if tt='K' then
          begin
            mainform.connectbtn.enabled:=false;
            mainform.disconnectbtn.enabled:=true;
            mainform.refresh.Enabled:=true;
            siteform.ModalResult:=1;
          end
          else messagedlg('未能与控制器建立连接!',mterror,[mbok],0);
        end
        else messagedlg('未能建立连接!',mterror,[mbok],0);
      end
      else messagedlg('调制解调器未相应!',mterror,[mbok],0);
    end
    else
    begin
      mainform.CommPortDriver1.Sendbyte(strtoint(number.text));
      mainform.CommPortDriver1.ReadChar(tt);
      if tt='K' then
      begin
        connected:=true;
        mainform.connectbtn.enabled:=false;
        mainform.disconnectbtn.enabled:=true;
        mainform.refresh.Enabled:=true;
        siteform.ModalResult:=1;
      end
      else messagedlg('未能与控制器建立连接!',mterror,[mbok],0);
    end;
  end
  else
  begin
    messagebeep(0);
    MessageDlg('串行口未打开.请检查串行口设置,然后重试.',mtError,[mbOK],0);
  end;
end;

procedure Tsiteform.incancelClick(Sender: TObject);
begin
  siteform.ModalResult:=1;
end;

procedure Tsiteform.FormActivate(Sender: TObject);
begin
  with tinifile.create(mypath+'sps.ini') do
  begin
    readsection('telephone',telephone.items);
    readsection('sitenumber',number.items);
    free;
  end;
  telephone.itemindex:=telephone.Items.Count-1;
  number.itemindex:=number.Items.count-1;
  autostart.Checked:=autostarted;
end;

procedure Tsiteform.FormClose(Sender: TObject; var Action: TCloseAction);
var i:integer;
    ff:boolean;
begin
  if connected then
  begin
    with tinifile.create(mypath+'sps.ini') do
    begin
      ff:=true;
      for i:=0 to telephone.Items.Count do
      begin
        if telephone.Items[i]=telephone.Text then ff:=false;
      end;
      if ff then writestring('telephone',telephone.text,'');
      ff:=true;
      for i:=0 to number.Items.count do
      begin
        if number.Items[i]=number.Text then ff:=false;
      end;
      if ff then writestring('sitenumber',number.text,'');
      free;
    end;
  end;
end;

procedure Tsiteform.numberChange(Sender: TObject);
begin
  numberradio.Checked:=true;
end;

procedure Tsiteform.telephoneChange(Sender: TObject);
begin
  teleradio.Checked:=true;
end;

procedure Tsiteform.autostartClick(Sender: TObject);
begin
  autostarted:=autostart.checked;
end;

end.
