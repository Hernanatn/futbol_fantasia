[Setup]
AppName=Fútbol Fantasia
AppVersion=0.0.1-alpha
DefaultDirName={pf}\Fútbol Fantasia
OutputDir=.
OutputBaseFilename=Instalador_FF-0.0.1_alpha

[Files]
Source: "../compilar/futbol_fantasia.exe"; DestDir: "{app}"; Flags: replacesameversion
Source: "../data/publico/recursos/*"; DestDir: "{app}/recursos"; Flags: replacesameversion recursesubdirs

[Icons]
Name: "{group}\Fútbol Fantasia"; Filename: "{app}\futbol_fantasia.exe"