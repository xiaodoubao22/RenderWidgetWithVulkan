#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform GlobalMatrixVP {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} globalMatrixVP;

layout(binding = 1) uniform InstanceMatrixM {
    mat4 model;
} instanceMatrixM;

layout(location = 0) in vec3 loacalPosition;
layout(location = 1) in vec2 texCoordInVert;
layout(location = 2) in vec3 normalInVert;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec4 pointOnWorld;

void main() {
    pointOnWorld = instanceMatrixM.model * vec4(loacalPosition, 1.0);
    gl_Position = globalMatrixVP.proj * globalMatrixVP.view * pointOnWorld;

    texCoord = texCoordInVert;
    normal = (instanceMatrixM.model * vec4(normalInVert, 1.0) - 
        instanceMatrixM.model * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
}