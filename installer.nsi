!include "MUI2.nsh"
!include "LogicLib.nsh"

; metadata
!define APPNAME "aria2Tray"
!define MUI_ICON "assets\icon.ico"
!define BUILD_DIR "dist\win\src"

!ifndef APPVERSION
!define APPVERSION "0.2.0"
!endif

Name "${APPNAME}"
OutFile "dist\${APPNAME}-${APPVERSION}-Windows-x64.exe"
InstallDir "$PROGRAMFILES64\aria2Tray"

!define MUI_FINISHPAGE_RUN "$INSTDIR\aria2Tray.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${APPNAME}"

; pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "aria2Tray Installer"
  SetShellVarContext all

  SetOverwrite on
  AllowSkipFiles off

  SetOutPath $INSTDIR

  File "LICENSE"
  File "${BUILD_DIR}\aria2tray.exe"
  File "${BUILD_DIR}\Qt6*.dll"
  File /r "${BUILD_DIR}\generic"
  File /r "${BUILD_DIR}\imageformats"
  File /r "${BUILD_DIR}\networkinformation"
  File /r "${BUILD_DIR}\platforms"
  File /r "${BUILD_DIR}\styles"
  File /r "${BUILD_DIR}\tls"
  File /r "${BUILD_DIR}\translations"
  
  File /nonfatal "${BUILD_DIR}\aria2c.exe"

  CreateDirectory "$SMPROGRAMS\${APPNAME}"
  CreateShortcut "$SMPROGRAMS\${APPNAME}\aria2Tray.lnk" "$INSTDIR\aria2tray.exe"
  CreateShortcut "$SMPROGRAMS\${APPNAME}\uninstall.lnk" "$INSTDIR\uninstall.exe"

  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon" "$INSTDIR\aria2tray.exe"
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher" "Cudiph"
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "HelpLink" "https://github.com/Cudiph/aria2Tray/issues"
  WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion" "${APPVERSION}"
  WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "EstimatedSize" 27985

  WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Section "uninstall"
  SetShellVarContext all

  nsProcess::_FindProcess "aria2tray.exe"
  Pop $R0
  ${If} $R0 = 0
    MessageBox MB_OK "The program is still running.$\n$\nAborting uninstallation."
    Quit
  ${EndIf}

  DeleteRegValue HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Run" "aria2tray"
  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"

  RMDir /r "$SMPROGRAMS\aria2Tray"
  RMDir /r /REBOOTOK "$INSTDIR"
SectionEnd
