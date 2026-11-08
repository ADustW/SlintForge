#ifndef ForgeVersion
  #error ForgeVersion is required
#endif
[Setup]
AppId={{64227F76-6CEB-4F1E-971F-67939FD826C8}
AppName=SlintForge Gallery
AppVersion={#ForgeVersion}
DefaultDirName={localappdata}\Programs\SlintForge
DefaultGroupName=SlintForge
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
OutputDir={#ForgeOutput}
OutputBaseFilename=SlintForge-Gallery-{#ForgeVersion}-Setup
SetupIconFile={#ForgeIcon}
UninstallDisplayIcon={app}\rust\slintforge-gallery-rust.exe
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
SetupLogging=yes
InfoAfterFile={#ForgePayload}\prerequisites\RUNTIME-README.txt
[Files]
Source: "{#ForgePayload}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
[Icons]
Name: "{group}\SlintForge Gallery"; Filename: "{app}\Gallery-Rust.exe"; WorkingDir: "{app}"; IconFilename: "{app}\Gallery-Rust.exe"
Name: "{group}\SlintForge Gallery C++"; Filename: "{app}\Gallery-Cpp.exe"; WorkingDir: "{app}"; IconFilename: "{app}\Gallery-Cpp.exe"
Name: "{group}\SlintForge Runtime Instructions"; Filename: "{app}\prerequisites\RUNTIME-README.txt"
Name: "{group}\SlintForge API Documentation"; Filename: "{app}\SlintForge-Docs.exe"; WorkingDir: "{app}"; IconFilename: "{app}\SlintForge-Docs.exe"
Name: "{group}\Uninstall SlintForge"; Filename: "{uninstallexe}"
