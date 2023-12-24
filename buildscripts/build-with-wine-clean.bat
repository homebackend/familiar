@echo off

set PATH=%PATH%;C:\Qt\2010.05\mingw\bin;C:\Qt\2010.05\bin;C:\Qt\2010.05\qt\bin;C:\Program Files\NSIS

qmake Familiar.pro
mingw32-make -j6 -f Makefile.Release clean

