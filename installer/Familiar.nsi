; familiar.exe
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install example2.nsi into a directory that the user selects,

;--------------------------------

; The name of the installer
Name "Familiar"

; The file to write
OutFile "familiar-inst.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Familiar

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Familiar" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

SetCompressor /SOLID lzma

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Familiar"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\bin\windows\Familiar.exe"
  File /r "..\qt\windows\*.*"
  File /r "..\examples"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Familiar "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Familiar" "DisplayName" "Familiar"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Familiar" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Familiar" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Familiar" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Familiar"
  CreateShortCut "$SMPROGRAMS\Familiar\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Familiar\Familiar.lnk" "$INSTDIR\Familiar.exe" "" "$INSTDIR\Familiar.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Familiar"
  DeleteRegKey HKLM SOFTWARE\Familiar

  ; Remove files and uninstaller
  Delete $INSTDIR\Familiar.exe
  Delete $INSTDIR\*.dll
  Delete $INSTDIR\imageformats\*.*
  Delete $INSTDIR\uninstall.exe
  RMDir $INSTDIR\imageformats
  RMDir /r $INSTDIR\examples
  RMDir $INSTDIR

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Familiar\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Familiar"
  RMDir "$INSTDIR"

SectionEnd
