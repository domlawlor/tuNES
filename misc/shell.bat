@echo off

pushd "%~dp0"
call "U:\Programs\Visual Studio\VC\Auxiliary\Build\vcvarsall.bat" x64
popd

set path=Y:\NesEmuD\misc;%path%

