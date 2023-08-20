@set glslc=C:\VulkanSDK\1.3.239.0\Bin

:: 设置环境变量
@set PATH=%glslc%;%PATH%

@set ShaderDir=..\code\Shaders
glslc %ShaderDir%\shader.vert -o ..\spvFiles\vert.spv
glslc %ShaderDir%\shader.frag -o ..\spvFiles\frag.spv
pause