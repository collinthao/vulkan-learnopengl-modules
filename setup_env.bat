
@echo off
setlocal

:: -----------------------------
:: 1. Define paths and files
:: -----------------------------
set VCPKG_DIR=C:\vcpkg
set VCPKG_ZIP=%TEMP%\vcpkg.zip
set SOURCE_FILE=main.cpp
set OUTPUT_FILE=main.exe

:: vcpkg packages
set VCPKG_PACKAGES=vulkan glm glfw3 tinyobjloader spirv-tools glslang stb

:: -----------------------------
:: 2. Install vcpkg if missing
:: -----------------------------
if not exist "%VCPKG_DIR%\vcpkg.exe" (
    echo vcpkg not found. Installing...
    powershell -Command ^
        "Invoke-WebRequest https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip -OutFile '%VCPKG_ZIP%'"
    powershell -Command ^
        "Expand-Archive '%VCPKG_ZIP%' -DestinationPath 'C:\'"
    if exist "C:\vcpkg-master" move "C:\vcpkg-master" "%VCPKG_DIR%"
    call "%VCPKG_DIR%\bootstrap-vcpkg.bat"
) else (
    echo vcpkg already installed.
)

:: -----------------------------
:: 3. Install vcpkg packages
:: -----------------------------
for %%p in (%VCPKG_PACKAGES%) do (
    "%VCPKG_DIR%\vcpkg.exe" install %%p
)

copy "C:\vcpkg\packages\glfw3_x64-windows\bin\glfw3.dll" ".\"
