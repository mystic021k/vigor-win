@echo off
cd /d "%~dp0"
if exist obj rmdir /s /q obj
if exist vigor.exe del /q vigor.exe
if exist objs.rsp del /q objs.rsp
if exist vigor.pdb del /q vigor.pdb
if exist vigor.ilk del /q vigor.ilk
if exist vigor.exp del /q vigor.exp
echo Cleaned compile output.
