@echo off

mkdir ..\build
pushd ..\build
cl -Fc -Zi y:\nesemu\code\nesemu.cpp user32.lib
popd
