; Ratosaurus Windows Installer Script (Inno Setup)
#define MyAppName "Ratosaurus"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Ratosaurus Audio"
#define MyAppURL "https://github.com/Rafthouse/Ratosaurus"

[Setup]
AppId={{4C9E7B2D-1A5F-4B8E-9C3D-7E2F6A1D0B5C}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\Ratosaurus Audio\Ratosaurus
DisableDirPage=yes
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename=Ratosaurus-Windows-Setup-x64
OutputDir=..\dist-windows
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
PrivilegesRequired=admin
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; VST3 bundle into the standard Windows VST3 directory
Source: "..\build\Ratosaurus_artefacts\Release\VST3\Ratosaurus.vst3\*"; DestDir: "{commoncf64}\VST3\Ratosaurus.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
; Standalone application
Source: "..\build\Ratosaurus_artefacts\Release\Standalone\Ratosaurus.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\Ratosaurus.exe"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"

[Messages]
WelcomeLabel2=This will install [name/ver] on your computer.%n%nRatosaurus is a compressor-distressor VST3 plugin by Mykyta Shchur.
