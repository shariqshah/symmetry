@echo off

call regen_project.bat
msbuild ..\build\vs2019\Symmetry.sln -m -p:Configuration=Release
set version_number=<..\bin\version.txt

git tag %version_number%