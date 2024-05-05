@set glslc=C:\VulkanSDK\1.3.239.0\Bin

:: 设置环境变量
@set PATH=%glslc%;%PATH%

@set SHADER_SRC_DIR=.\Shaders
glslc %SHADER_SRC_DIR%\DrawMesh.vert -o .\Spirv\DrawMesh.vert.spv
glslc %SHADER_SRC_DIR%\DrawMesh.frag -o .\Spirv\DrawMesh.frag.spv


pause