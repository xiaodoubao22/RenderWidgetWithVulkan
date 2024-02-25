#version 450
#extension GL_ARB_separate_shader_objects : enable

// texture
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 pointOnWorld;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(texture(texSampler, texCoord).rgb, 1.0);
}