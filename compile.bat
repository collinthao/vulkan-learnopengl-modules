@echo off

cl /EHsc /std:c++20 /MD main.cpp /link user32.lib gdi32.lib shell32.lib ole32.lib winmm.lib glfw3.lib vulkan-1.lib SPIRVd.lib SPIRV-Toolsd.lib
SPIRV-Tools-diffd.lib SPIRV-Tools-optd.lib glslangd.lib GenericCodeGend.lib MachineIndependent.lib glslang-default-resource-limitsd.lib
