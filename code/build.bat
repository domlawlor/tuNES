@echo off

mkdir ..\build
pushd ..\build
cl -Fc -Zi y:\NesEmuD\code\nesemu.cpp user32.lib Gdi32.lib
popd
