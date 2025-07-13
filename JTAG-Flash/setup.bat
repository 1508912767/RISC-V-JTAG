@echo off
set NUCLEI_TOOL_ROOT=D:\changrenqiao\NucleiStudio_202212\toolchain
REM Create your setup_config.bat
REM and define NUCLEI_TOOL_ROOT like below
REM set NUCLEI_TOOL_ROOT=D:\NucleiStudio_202212\toolchain
set SETUP_CONFIG=setup_config.bat

if exist %SETUP_CONFIG% call %SETUP_CONFIG%

echo Setup Nuclei SDK Tool Environment
echo NUCLEI_TOOL_ROOT=%NUCLEI_TOOL_ROOT%

set PATH=%NUCLEI_TOOL_ROOT%\gcc\bin;%NUCLEI_TOOL_ROOT%\openocd\bin;%NUCLEI_TOOL_ROOT%\build-tools\bin;%NUCLEI_TOOL_ROOT%\qemu\bin;%PATH%
@echo on
