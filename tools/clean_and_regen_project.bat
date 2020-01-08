@echo off

chdir /D W:\build\
rmdir /S /Q W:\build\vs2019
call regen_project.bat