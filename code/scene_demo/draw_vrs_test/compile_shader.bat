@set glslc=C:\VulkanSDK\1.3.239.0\Bin

:: 设置环境变量
@set PATH=%glslc%;%PATH%

@set SHADER_SRC_DIR=.\Shaders
glslc %SHADER_SRC_DIR%\DrawMesh.vert -o .\Spirv\DrawMesh.vert.spv
glslc %SHADER_SRC_DIR%\DrawMeshGlossy.frag -o .\Spirv\DrawMeshGlossy.frag.spv

glslc %SHADER_SRC_DIR%\ScreenQuad.frag -o .\Spirv\ScreenQuad.frag.spv
glslc %SHADER_SRC_DIR%\ScreenQuad.vert -o .\Spirv\ScreenQuad.vert.spv
glslc %SHADER_SRC_DIR%\ScreenQuad.vert -o .\Spirv\ScreenQuad.vert.spv

glslc %SHADER_SRC_DIR%\pbr_width_texture.frag -o .\Spirv\pbr_width_texture.frag.spv
glslc %SHADER_SRC_DIR%\pbr_width_texture.vert -o .\Spirv\pbr_width_texture.vert.spv

glslc %SHADER_SRC_DIR%\draw_vrs_region.frag -o .\Spirv\draw_vrs_region.frag.spv
glslc %SHADER_SRC_DIR%\draw_vrs_region.vert -o .\Spirv\draw_vrs_region.vert.spv

glslc %SHADER_SRC_DIR%\draw_vrs_region.comp -o .\Spirv\draw_vrs_region.comp.spv

glslc %SHADER_SRC_DIR%\blend_vrs_image.frag -o .\Spirv\blend_vrs_image.frag.spv

pause