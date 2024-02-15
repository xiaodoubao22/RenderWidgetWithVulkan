@set glslc=C:\VulkanSDK\1.3.239.0\Bin

:: 设置环境变量
@set PATH=%glslc%;%PATH%

@set DemoShaderDir=..\code\demo\Shaders
glslc %DemoShaderDir%\DrawTriangleTest.vert -o ..\spv_files\DrawTriangleTestVert.spv
glslc %DemoShaderDir%\DrawTriangleTest.frag -o ..\spv_files\DrawTriangleTestFrag.spv

glslc %DemoShaderDir%\DrawTextureTest.vert -o ..\spv_files\DrawTextureTestVert.spv
glslc %DemoShaderDir%\DrawTextureTest.frag -o ..\spv_files\DrawTextureTestFrag.spv

glslc %DemoShaderDir%\DrawMesh.vert -o ..\spv_files\DrawMesh.vert.spv
glslc %DemoShaderDir%\DrawMesh.frag -o ..\spv_files\DrawMesh.frag.spv
pause