@echo off

set PATH=%PATH%;C:\Qt\2010.05\mingw\bin;C:\Qt\2010.05\bin;C:\Qt\2010.05\qt\bin;C:\Program Files\NSIS

set INSTALLER_DIR=installer\windows

qmake Familiar.pro
mingw32-make -j6 -f Makefile.Release 
makensis.exe installer\familiar.nsi
mkdir %INSTALLER_DIR%
if exist %INSTALLER_DIR%\familiar-inst.exe del %INSTALLER_DIR%\familiar-inst.exe
move familiar-inst.exe %INSTALLER_DIR%

