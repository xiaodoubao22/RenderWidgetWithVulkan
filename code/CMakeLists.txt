cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)

project(RenderWidgetWithVulkan)

# [third_party]设置三方库包含路径
set(THIRD_PARTY ${CMAKE_SOURCE_DIR}/../third_party)
set(GLM_DIR ${THIRD_PARTY}/glm)
set(GLFW_DIR ${THIRD_PARTY}/glfw-3.3.8.bin.WIN64)
set(STB_DIR ${THIRD_PARTY}/stb)
set(TINY_OBJ_LOADER_DIR ${THIRD_PARTY}/tinyobjloader)
set(VMA_DIR ${THIRD_PARTY}/VulkanMemoryAllocator)
find_package(Vulkan REQUIRED)
# 包含三方库
include_directories(${GLM_DIR} ${GLFW_DIR}/include ${STB_DIR} ${TINY_OBJ_LOADER_DIR} ${VMA_DIR})

# [framework]设置头文件路径
set(FRAMEWORK_INC_DIRS ${CMAKE_SOURCE_DIR}/framework/Inc)
file(GLOB FRAMEWORK_INC_FILES ./framework/Inc/*.h ./framework/Inc/*.hpp)

# [framework]设置源文件路径
set(FRAMEWORK_SRC_DIRS ${CMAKE_SOURCE_DIR}/framework/Src)
file(GLOB FRAMEWORK_SRC_FILES ./framework/Src/*.c ./framework/Src/*.cpp)

set(SCENE_DEMO_DIR ${CMAKE_SOURCE_DIR}/scene_demo)

set(PROJ_NAME_LIST
    draw_rotate_quad
    draw_scene_test
    draw_scene_pbr
    draw_texture_msaa
    )

foreach(PROJ_NAME IN LISTS PROJ_NAME_LIST)
    set(SCENE_DEMO_INC_DIRS ${SCENE_DEMO_DIR}/${PROJ_NAME}/Inc)
    file(GLOB SCENE_DEMO_INC_FILES ${SCENE_DEMO_INC_DIRS}/*.h ${SCENE_DEMO_INC_DIRS}/*.hpp)
    set(SCENE_DEMO_SRC_DIRS ${SCENE_DEMO_DIR}/${PROJ_NAME}/Src)
    file(GLOB SCENE_DEMO_SRC_FILES ${SCENE_DEMO_SRC_DIRS}/*.c ${SCENE_DEMO_SRC_DIRS}/*.cpp)
    # 生成exe
    add_executable(${PROJ_NAME}
        ${FRAMEWORK_INC_FILES}
        ${FRAMEWORK_SRC_FILES}
        ${SCENE_DEMO_INC_FILES}
        ${SCENE_DEMO_SRC_FILES}
        )
    # 包含目录
    target_include_directories(${PROJ_NAME} PUBLIC 
        ${FRAMEWORK_INC_DIRS}
        ${SCENE_DEMO_INC_DIRS}
        ${Vulkan_INCLUDE_DIRS}
        )
    # [third_party]链接静态库
    target_link_libraries(${PROJ_NAME} 
        ${GLFW_DIR}/lib-vc2022/glfw3.lib
        Vulkan::Vulkan
        )
endforeach()

# 设置VS工程目录
file(GLOB ALL_DEMO_FILES
    ${SCENE_DEMO_DIR}/*/Inc/*.h ${SCENE_DEMO_DIR}/*/Inc/*.hpp
    ${SCENE_DEMO_DIR}/*/Src/*.c ${SCENE_DEMO_DIR}/*/Src/*.cpp
    )
set(ALL_FILES
    ${FRAMEWORK_INC_FILES}
    ${FRAMEWORK_SRC_FILES}
    ${ALL_DEMO_FILES}
    )
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${ALL_FILES})

# [vs]设置启动项目
set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT draw_scene_pbr)



