@echo off

subst /D "W:"
subst W: %CD%\..
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cls
set path=W:\tools;%path% rem
call launch_vs.bat
