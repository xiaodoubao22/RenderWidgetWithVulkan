#version 450
#extension GL_ARB_separate_shader_objects : enable

// texture
layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(texture(texSampler, fragTexCoord).rgb, 1.0);
}