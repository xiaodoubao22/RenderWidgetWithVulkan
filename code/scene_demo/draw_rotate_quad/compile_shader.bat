@set glslc=C:\VulkanSDK\1.3.239.0\Bin

:: 设置环境变量
@set PATH=%glslc%;%PATH%

@set SHADER_SRC_DIR=.\Shaders
glslc %SHADER_SRC_DIR%\DrawTriangleTest.vert -o .\Spirv\DrawTriangleTest.vert.spv
glslc %SHADER_SRC_DIR%\DrawTriangleTest.frag -o .\Spirv\DrawTriangleTest.frag.spv

pause