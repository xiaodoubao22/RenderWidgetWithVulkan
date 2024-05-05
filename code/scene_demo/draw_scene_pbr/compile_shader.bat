@set glslc=C:\VulkanSDK\1.3.239.0\Bin

:: 设置环境变量
@set PATH=%glslc%;%PATH%

@set SHADER_SRC_DIR=.\Shaders
glslc %SHADER_SRC_DIR%\DrawMesh.vert -o .\Spirv\DrawMesh.vert.spv
glslc %SHADER_SRC_DIR%\DrawMeshGlossy.frag -o .\Spirv\DrawMeshGlossy.frag.spv

glslc %SHADER_SRC_DIR%\ScreenQuad.frag -o .\Spirv\ScreenQuad.frag.spv
glslc %SHADER_SRC_DIR%\ScreenQuad.vert -o .\Spirv\ScreenQuad.vert.spv

pause