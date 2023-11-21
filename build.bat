@echo off
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

call %VCVARS% x86_x64

if not exist build mkdir build
cd build
@echo on

python ../configure.py --sdks cs2 --targets=x86_64 --plugin-name=vgrank --plugin-alias=vgrank 

ambuild
exit