cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(RenderWidgetWithVulkan)

# [third_party]设置三方库包含路径
set(THIRD_PARTY ${CMAKE_SOURCE_DIR}/../third_party)
set(GLM_DIR ${THIRD_PARTY}/glm)
set(GLFW_DIR ${THIRD_PARTY}/glfw-3.3.8.bin.WIN64)
set(STB_DIR ${THIRD_PARTY}/stb)
find_package(Vulkan REQUIRED)
# 包含三方库
include_directories(${GLM_DIR} ${GLFW_DIR}/include ${STB_DIR})

# [RenderWidget]设置头文件路径
set(RENDER_WIDGET_INC_DIRS
    ${CMAKE_SOURCE_DIR}/framework/Inc
    ${CMAKE_SOURCE_DIR}/demo/Inc
    )
file(GLOB RENDER_WIDGET_INC_FILES
    ./framework/Inc/*.h ./framework/Inc/*.hpp
    ./demo/Inc/*.h ./demo/Inc/*.hpp
    )

# [RenderWidget]设置源文件路径
set(RENDER_WIDGET_SRC_DIRS
    ${CMAKE_SOURCE_DIR}/framework/Src
    ${CMAKE_SOURCE_DIR}/demo/Src
    )
file(GLOB RENDER_WIDGET_SRC_FILES
    ./framework/Src/*.c ./framework/Src/*.cpp
    ./demo/Src/*.c ./demo/Src/*.cpp
    )

# -----------render widget-----------
set(RENDER_WIDGET RenderWidget)
# 生成exe
add_executable(${RENDER_WIDGET} 
    ${RENDER_WIDGET_INC_FILES}
    ${RENDER_WIDGET_SRC_FILES}
    )
# 包含目录
target_include_directories(${RENDER_WIDGET} PUBLIC 
    ${RENDER_WIDGET_INC_DIRS}
    ${Vulkan_INCLUDE_DIRS}
    )
# [third_party]链接静态库
target_link_libraries(${RENDER_WIDGET} 
    ${GLFW_DIR}/lib-vc2022/glfw3.lib
    Vulkan::Vulkan
    )
# 设置VS工程目录
set(ALL_FILES
    ${RENDER_WIDGET_INC_DIRS}
    ${RENDER_WIDGET_INC_FILES}
    ${RENDER_WIDGET_SRC_DIRS}
    ${RENDER_WIDGET_SRC_FILES}
    )
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${ALL_FILES})

# [vs]设置启动项目
set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${RENDER_WIDGET})



