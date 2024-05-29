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

layout(location = 0) in vec3 vsInLoacalPosition;
layout(location = 1) in vec2 vsInTexCoord;
layout(location = 2) in vec3 vsInNormal;
layout(location = 3) in vec3 vsInTangent;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec4 pointOnWorld;
layout(location = 3) out mat3 matTBN;

void main() {
    pointOnWorld = instanceMatrixM.model * vec4(vsInLoacalPosition, 1.0);
    gl_Position = globalMatrixVP.proj * globalMatrixVP.view * pointOnWorld;

    texCoord = vsInTexCoord;
    normal = (instanceMatrixM.model * vec4(vsInNormal, 1.0) - 
        instanceMatrixM.model * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    
    matTBN = mat3(vsInTangent, cross(normal, vsInTangent), normal);
}