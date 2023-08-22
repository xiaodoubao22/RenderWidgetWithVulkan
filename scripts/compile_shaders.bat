@set glslc=C:\VulkanSDK\1.3.239.0\Bin

:: 设置环境变量
@set PATH=%glslc%;%PATH%

@set ShaderDir=..\code\Shaders
glslc %ShaderDir%\DrawTriangleTest.vert -o ..\spv_files\DrawTriangleTestVert.spv
glslc %ShaderDir%\DrawTriangleTest.frag -o ..\spv_files\DrawTriangleTestFrag.spv
pause