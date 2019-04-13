@echo off

subst /D "W:"
subst W: %CD%\..
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cls
set editor=""C:\Applications\Emacs\bin\runemacs.exe""
set path=W:\tools;%path% rem

