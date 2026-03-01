@echo off
:: Set Vulkan environment for overlays
set VK_INSTANCE_LAYERS=
set DISABLE_LAYER_NV_OPTIMUS_1=1
set DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1=1
set DISABLE_TWITCH_VULKAN_OVERLAY=1
set DISABLE_VULKAN_OBS_CAPTURE=1

:: Add GLFW DLL directory to PATH
set PATH=%PATH%;C:\vcpkg\packages\glfw3_x64-windows\bin

:: Set working directory to script location
cd /d "%~dp0"

:: Run your executable
start "" main.exe
