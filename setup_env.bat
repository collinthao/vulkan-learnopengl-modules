@echo off
setlocal EnableExtensions EnableDelayedExpansion

:: =================================================
:: 0. Vulkan environment hardening (IMPORTANT)
:: =================================================

echo Configuring Vulkan runtime environment...

:: Clear forced instance layers
set VK_INSTANCE_LAYERS=

:: Disable problematic implicit layers
set DISABLE_LAYER_NV_OPTIMUS_1=1
set DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1=1
set DISABLE_TWITCH_VULKAN_OVERLAY=1
set DISABLE_VULKAN_OBS_CAPTURE=1
set DISABLE_LAYER_OBS_HOOK_1=1

:: Silence loader spam
set VK_LOADER_DEBUG=none

:: Prefer discrete GPU
set VK_ICD_FILENAMES=
set __NV_PRIME_RENDER_OFFLOAD=1
set __GLX_VENDOR_LIBRARY_NAME=nvidia

echo Vulkan environment configured.
echo.

:: =================================================
:: 1. Define paths and files
:: =================================================

set VCPKG_DIR=C:\vcpkg
set VCPKG_ZIP=%TEMP%\vcpkg.zip
set SOURCE_FILE=main.cpp
set OUTPUT_FILE=main.exe

:: vcpkg packages
set VCPKG_PACKAGES=vulkan glm glfw3 tinyobjloader spirv-tools glslang stb assimp

:: =================================================
:: 2. Install vcpkg if missing
:: =================================================

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

echo.

:: =================================================
:: 3. Install vcpkg packages
:: =================================================

for %%p in (%VCPKG_PACKAGES%) do (
    "%VCPKG_DIR%\vcpkg.exe" install %%p
)

echo.

:: =================================================
:: 4. Copy required runtime DLLs
:: =================================================

copy "%VCPKG_DIR%\packages\glfw3_x64-windows\bin\glfw3.dll" ".\"

echo Setup complete.

