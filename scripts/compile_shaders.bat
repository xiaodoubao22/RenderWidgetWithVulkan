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
glslc %DemoShaderDir%\DrawMeshDefuse.frag -o ..\spv_files\DrawMeshDefuse.frag.spv
glslc %DemoShaderDir%\DrawMeshGlossy.frag -o ..\spv_files\DrawMeshGlossy.frag.spv

glslc %DemoShaderDir%\ScreenQuad.vert -o ..\spv_files\ScreenQuad.vert.spv
glslc %DemoShaderDir%\ScreenQuad.frag -o ..\spv_files\ScreenQuad.frag.spv
pause