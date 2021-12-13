@ECHO OFF

REM Delete Build
rd /s /q build\

REM Delete Exe, .obj, .pdb
del /s /q bin\Debug\*.exe  
del /s /q bin\Debug\*.ilk
del /s /q bin\Debug\*.pdb

REM Create folder "build"
if not exist "\build\" mkdir build

REM Create delete.me to upload "build" folder
copy nul build\delete.me