@echo off

REM
REM  To run this at startup, use this as your shortcut target:
REM  %windir%\system32\cmd.exe /k w:\handmade\misc\shell.bat
REM

call "U:\Programs\Visual Studio\VC\vcvarsall.bat" x64
set path=Y:\NesEmuD\misc;%path%
