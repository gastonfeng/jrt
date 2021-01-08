unit setupunit;

interface

uses
  SysUtils, Windows, Messages, Classes, Graphics, Controls,
  StdCtrls, ExtCtrls, Forms, ComCtrls,inifiles,winprocs;

type
  Tsetup = class(TForm)
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    PageControl1: TPageControl;
    TabSheet2: TTabSheet;
    Label1: TLabel;
    Label2: TLabel;
    ComboBox1: TComboBox;
    ComboBox2: TComboBox;
    setup_ok: TButton;
    setup_cancel: TButton;
    procedure FormCreate(Sender: TObject);
    procedure setup_okClick(Sender: TObject);
    procedure setup_cancelClick(Sender: TObject);
  end;


var
  setup: Tsetup;
  inifile:TIniFile;

procedure showsetupbox(sender:tobject);

implementation

uses main;

{$ifndef TranslatedForm}
{$R *.DFM}
{$else}
{$R *.TRA}
{$endif}
procedure showsetupbox(Sender: TObject);
begin
{  with setup.create(Application) do
    try
      showmodal;
    finally
      free;
  end;}
  setup:=tsetup.Create(application);
  setup.ShowModal;
end;

procedure Tsetup.FormCreate(Sender: TObject);
begin
  inifile:=Tinifile.Create(mypath+'SPS.ini');
  combobox1.ItemIndex:=inifile.ReadInteger('PORT','COM',-1);
  combobox2.ItemIndex:=inifile.ReadInteger('PORT','BAUDRATE',-1);
  inifile.Free;
end;


procedure Tsetup.setup_okClick(Sender: TObject);
begin
  inifile:=TIniFile.Create(mypath+'SPS.ini');
  inifile.writeinteger('PORT','COM',combobox1.itemindex);
  inifile.writeinteger('PORT','BAUDRATE',combobox2.itemindex);
  inifile.free;
  ModalResult:=1;
end;

procedure Tsetup.setup_cancelClick(Sender: TObject);
begin
  MOdalResult:=1;
end;

end.
