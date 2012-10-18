@echo on 
echo Finishing installation task....

REM
REM mediastreamer2-antisip-4.3.3-rc2
REM
set MS2_DIR=mediastreamer2-antisip-4.3.3-rc2
set UNI_DIR=unimrcp-deps

del %MS2_DIR%
del %UNI_DIR%

mkdir %MS2_DIR%
7z.exe -y x %MS2_DIR%.tar.gz 
if %ERRORLEVEL% NEQ 0 GOTO Error

7z.exe -y x %MS2_DIR%.tar%
if %ERRORLEVEL% NEQ 0 GOTO Error

copy /Y mediastreamer2.def .\%MS2_DIR%\mediastreamer2\build\win32native
if %ERRORLEVEL% NEQ 0 GOTO Error

copy /Y ortp.def  .\%MS2_DIR%\oRTP\build\win32native
if %ERRORLEVEL% NEQ 0 GOTO Error

REM
REM Unimrcp
REM
7z.exe -y x %UNI_DIR%.zip
if %ERRORLEVEL% NEQ 0 GOTO Error

copy /Y apr.vsprops .\unimrcp\unimrcp\build\vsprops 
if %ERRORLEVEL% NEQ 0 GOTO Error

copy /Y sofia.vsprops .\unimrcp\unimrcp\build\vsprops
if %ERRORLEVEL% NEQ 0 GOTO Error

:Error
