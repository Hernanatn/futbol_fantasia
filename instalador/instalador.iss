[Setup]
AppName=Fútbol Fantasia
AppVersion=0.0.1-alpha
DefaultDirName={pf}\Fútbol Fantasia
OutputDir=.
OutputBaseFilename=Instalador_FF-0.0.1_alpha

[Files]
Source: "../compilar/Debug/futbol_fantasia.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "../compilar/Debug/futbol_fantasia.pdb"; DestDir: "{app}"; Flags: ignoreversion
Source: "../compilar/Debug/glfw3.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "../compilar/Debug/raylib.dll"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Fútbol Fantasia"; Filename: "{app}\futbol_fantasia.exe"