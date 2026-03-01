@echo off
setlocal

:: -----------------------------
:: 1. Define variables
:: -----------------------------
set SOURCE_FILE=run.cpp
set OUTPUT_FILE=run.exe
set RESPONSE_FILE=buildrun.rsp
set VCPKG_DIR=C:\vcpkg
set BUILD_DIR=%CD%

:: -----------------------------
:: 2. Visual Studio environment
:: -----------------------------
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

:: -----------------------------
:: 3. Generate response file with full paths
:: -----------------------------
(
echo /EHsc
echo /std:c++20
echo %SOURCE_FILE%
echo /link
:: Full paths to all library files
echo user32.lib
echo gdi32.lib
echo shell32.lib
echo ole32.lib
echo winmm.lib
echo /OUT:"%OUTPUT_FILE%"
) > "%RESPONSE_FILE%"

:: -----------------------------
:: 4. Compile using response file
:: -----------------------------
cl @"%RESPONSE_FILE%"

pause
