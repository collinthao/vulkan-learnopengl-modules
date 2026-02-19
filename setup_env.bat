@echo off
:: Set include paths
set MY_INCLUDES=C:\Users\Collin\Projects\vulkan-learnopengl-modules\Libraries\glm-1.0.2\glm
set LIB=C:\Users\Collin\Projects\vulkan-learnopengl-modules\Libraries\glfw-3.4.bin.WIN64\lib-vc2015;%LIB%;C:\VulkanSDK\1.4.341.1\Lib
:: Add Visual Studio compiler to PATH (change VS version if needed)
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

:: Append your includes to the INCLUDE environment variable
set INCLUDE=%MY_INCLUDES%;%INCLUDE%;C:\Users\Collin\Projects\vulkan-learnopengl-modules\Libraries\glfw-3.4.bin.WIN64/include;C:\VulkanSDK\1.4.341.1\Include;C:\Users\Collin\Projects\vulkan-learnopengl-modules\Libraries\stb_master;C:\Users\Collin\Projects\vulkan-learnopengl-modules\Libraries\tiny_obj_loader

