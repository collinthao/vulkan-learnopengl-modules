@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
:: Set include paths
set DIRECTORY=%~dp0
set MY_INCLUDES=%DIRECTORY%\Libraries\glm-1.0.2\glm
set LIB=%DIRECTORY%\Libraries\glfw-3.4.bin.WIN64\lib-vc2022;%LIB%;%DIRECTORY%\Libraries\VulkanSDK\1.4.335.0\Lib

:: Append your includes to the INCLUDE environment variable
set INCLUDE=%MY_INCLUDES%;%INCLUDE%;%DIRECTORY%\Libraries\glfw-3.4.bin.WIN64\include;%DIRECTORY%\Libraries\VulkanSDK\1.4.335.0\Include
