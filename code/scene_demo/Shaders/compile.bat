@set glslc=C:\VulkanSDK\1.3.239.0\Bin

:: 设置环境变量
@set PATH=%glslc%;%PATH%

@set SHADER_SRC_DIR=.\
glslc %SHADER_SRC_DIR%\DrawMesh.vert -o - | %{ $_.ToString("X2") } .\spv\DrawMesh.vert.spv 

pause