@echo off

call buildscripts\versioninfo.bat

set APPINFORCFILE=rc\appinfo.rc
set VERSIONDEFSHFILE=src\versiondefs.h

set FILEDESCRIPTION=Familiar Executable
set COPYRIGHT=Copyright (c) Neeraj Jakhar 2009-%COPYRIGHTENDYEAR%.

echo Generating %APPINFORCFILE%
if exist %APPINFORCFILE% del /F /Q %APPINFORCFILE%
echo # if defined(UNDER_CE) >> %APPINFORCFILE%
echo #  include "winbase.h" >> %APPINFORCFILE%
echo # else >> %APPINFORCFILE%
echo #  include "winver.h" >> %APPINFORCFILE%
echo # endif >> %APPINFORCFILE%
rem echo. >> %APPINFORCFILE%
echo VS_VERSION_INFO VERSIONINFO >> %APPINFORCFILE%
echo 	FILEVERSION %VER_MAJ%,%VER_MIN%,%VER_PAT%,0 >> %APPINFORCFILE%
echo 	PRODUCTVERSION %VER_MAJ%,%VER_MIN%,%VER_PAT%,0 >> %APPINFORCFILE%
echo 	FILEFLAGSMASK 0x3fL >> %APPINFORCFILE%
echo #ifdef _DEBUG >> %APPINFORCFILE%
echo 	FILEFLAGS VS_FF_DEBUG >> %APPINFORCFILE%
echo #else >> %APPINFORCFILE%
echo 	FILEFLAGS 0x0L >> %APPINFORCFILE%
echo #endif >> %APPINFORCFILE%
echo 	FILEOS VOS__WINDOWS32 >> %APPINFORCFILE%
echo 	FILETYPE VFT_DLL >> %APPINFORCFILE%
echo 	FILESUBTYPE 0x0L >> %APPINFORCFILE%
echo 	BEGIN >> %APPINFORCFILE%
echo 		BLOCK "StringFileInfo" >> %APPINFORCFILE%
echo 		BEGIN >> %APPINFORCFILE%
echo 			BLOCK "040904B0" >> %APPINFORCFILE%
echo 			BEGIN >> %APPINFORCFILE%
echo 				VALUE "CompanyName", "%COMPANY_NAME%\0" >> %APPINFORCFILE%
echo 				VALUE "FileDescription", "%FILEDESCRIPTION%\0" >> %APPINFORCFILE%
echo 				VALUE "FileVersion", "%VER_MAJ%.%VER_MIN%.%VER_PAT%.0\0" >> %APPINFORCFILE%
echo 				VALUE "LegalCopyright", "%COPYRIGHT%\0" >> %APPINFORCFILE%
echo 				VALUE "OriginalFilename", "Familiar.exe\0" >> %APPINFORCFILE%
echo 				VALUE "ProductName", "Familiar\0" >> %APPINFORCFILE%
echo 			END >> %APPINFORCFILE%
echo 		END >> %APPINFORCFILE%
echo 	END >> %APPINFORCFILE%
echo /* End of Version info */ >> %APPINFORCFILE%
rem echo. >> %APPINFORCFILE%
rem echo. >> %APPINFORCFILE%
echo IDI_ICON1               ICON    DISCARDABLE     "familiar.ico" >> %APPINFORCFILE%
rem echo. >> %APPINFORCFILE%

echo Generating %VERSIONDEFSHFILE%
if exist %VERSIONDEFSHFILE% del /f /q %VERSIONDEFSHFILE%
echo #ifndef VERSIONDEFS_H >> %VERSIONDEFSHFILE%
echo #define VERSIONDEFS_H >> %VERSIONDEFSHFILE%
rem echo. >> %VERSIONDEFSHFILE%
echo #define VER_MAJ          %VER_MAJ% >> %VERSIONDEFSHFILE%
echo #define VER_MIN          %VER_MIN% >> %VERSIONDEFSHFILE%
echo #define VER_PAT          %VER_PAT% >> %VERSIONDEFSHFILE%
echo #define FAMILIARAPP      "Familiar" >> %VERSIONDEFSHFILE%
echo #define FAMILIARORG      "%COMPANY_NAME%" >> %VERSIONDEFSHFILE%
echo #define FAMILIARDOMAIN   "%DOMAIN_NAME%" >> %VERSIONDEFSHFILE%
rem echo. >> %VERSIONDEFSHFILE%
echo #endif >> %VERSIONDEFSHFILE%
rem // VERSIONDEFS_H" >> %VERSIONDEFSHFILE%
rem echo. >> %VERSIONDEFSHFILE%

