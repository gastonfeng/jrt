program sps;

uses
  Forms,
  main in 'main.pas' {MainForm},
  about in 'about.pas' {AboutBox},
  SiteUnit in 'SiteUnit.pas' {siteform},
  CoverUnit in 'CoverUnit.pas' {CoverForm};


{$R *.RES}

begin
  CoverForm:=TCoverForm.Create(Application);
  CoverForm.Show;
  CoverForm.Update;
  Application.Title := '智能控制器通讯程序';
  Application.CreateForm(TMainForm, MainForm);
  Application.CreateForm(TCoverForm, CoverForm);
  Application.Run;
end.
