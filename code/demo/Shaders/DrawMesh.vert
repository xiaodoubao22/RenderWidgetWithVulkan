#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 loacalPosition;
layout(location = 1) in vec2 texCoordInVert;

layout(location = 0) out vec2 texCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(loacalPosition, 1.0);
    texCoord = texCoordInVert;
}