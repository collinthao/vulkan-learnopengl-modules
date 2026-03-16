@echo off
setlocal

:: -----------------------------
:: 1. Define variables
:: -----------------------------
set SOURCE_FILE=main.cpp
set OUTPUT_FILE=main.exe
set RESPONSE_FILE=build.rsp
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
::echo /DNDEBUG
echo /I"%VCPKG_DIR%\packages\glm_x64-windows\include\glm"
echo /I"%VCPKG_DIR%\packages\glfw3_x64-windows\include"
echo /I"%VCPKG_DIR%\packages\vulkan-headers_x64-windows\include"
echo /I"%VCPKG_DIR%\packages\tinyobjloader_x64-windows\include"
echo /I"%VCPKG_DIR%\packages\stb_x64-windows\include"
echo /I"%VCPKG_DIR%\packages\assimp_x64-windows\include"
echo %SOURCE_FILE%
echo /link
:: Full paths to all library files
echo "%VCPKG_DIR%\packages\glfw3_x64-windows\lib\glfw3dll.lib"
echo "%VCPKG_DIR%\packages\vulkan-loader_x64-windows\lib\vulkan-1.lib"
echo "%VCPKG_DIR%\packages\spirv-tools_x64-windows\lib\SPIRV-Tools.lib"
echo "%VCPKG_DIR%\packages\spirv-tools_x64-windows\lib\SPIRV-Tools-diff.lib"
echo "%VCPKG_DIR%\packages\spirv-tools_x64-windows\lib\SPIRV-Tools-opt.lib"
echo "%VCPKG_DIR%\packages\glslang_x64-windows\lib\glslang.lib"
echo "%VCPKG_DIR%\packages\glslang_x64-windows\lib\GenericCodeGen.lib"
echo "%VCPKG_DIR%\packages\glslang_x64-windows\lib\MachineIndependent.lib"
echo "%VCPKG_DIR%\packages\glslang_x64-windows\lib\glslang-default-resource-limits.lib"
echo "%VCPKG_DIR%\packages\assimp_x64-windows\lib\assimp-vc143-mt.lib"
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

:: -----------------------------
:: 5. Copy runtime DLLs
:: -----------------------------
echo Copying runtime DLLs...

for %%dll in (
    glfw3.dll
    Vulkan-1.dll
    SPIRV-Tools.dll
    glslang.dll
    assimp-vc143-mt.dll
) do (
    for %%libdir in (
        glfw3_x64-windows
        vulkan-loader_x64-windows
        spirv-tools_x64-windows
        glslang_x64-windows
	assimp_x64-windows
    ) do (
        if exist "%VCPKG_DIR%\packages\%%libdir\bin\%%dll" copy "%VCPKG_DIR%\packages\%%libdir\bin\%%dll" "%BUILD_DIR%"
    )
)

:: -----------------------------
:: 6. Run the executable
:: -----------------------------
echo Running %OUTPUT_FILE%...
"%BUILD_DIR%\%OUTPUT_FILE%"

pause
