@echo off

rem figure out the folder name
set MONETDB=%~dp0

rem remove the final backslash from the path
set MONETDB=%MONETDB:~0,-1%

rem extend the search path with our EXE and DLL folders
rem we depend on pthreadVCE.dll having been copied to the lib folder
set PATH=%MONETDB%\bin;%MONETDB%\lib;%PATH%

set MONETDBFARM=
set SQLLOGDIR=
if "%ALLUSERSPROFILE%" == "" goto skip
set MONETDBFARM="--dbfarm=%ALLUSERSPROFILE%\Application Data\MonetDB"
set SQLLOGDIR=--set "sql_logdir=%ALLUSERSPROFILE%\Application Data\MonetDB\var\MonetDB\log"
:skip

rem start the real server
"%MONETDB%\bin\Mserver.exe" --set "prefix=%MONETDB%" --set "exec_prefix=%MONETDB%" %MONETDBFARM% %SQLLOGDIR% %*
