set GLSL_PATH=glslc.exe

%GLSL_PATH% shader.vert -o vert.spv
%GLSL_PATH% shader.frag -o frag.spv
%GLSL_PATH% shader.comp -o comp.spv
%GLSL_PATH% meshShader.vert -o meshVert.spv
%GLSL_PATH% meshShader.frag -o meshFrag.spv
%GLSL_PATH% lightShader.vert -o lightVert.spv
%GLSL_PATH% lightShader.frag -o lightFrag.spv
%GLSL_PATH% primitiveShader.vert -o primitiveVert.spv
%GLSL_PATH% primitiveShader.frag -o primitiveFrag.spv
%GLSL_PATH% stencil.vert -o stencilVert.spv
%GLSL_PATH% stencil.frag -o stencilFrag.spv
pause
