#version 450
#extension GL_ARB_separate_shader_objects : enable

// texture
// layout(binding = 0) uniform sampler2D texSampler;

// in
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

// out
layout(location = 0) out vec4 outColor;

void main() {
    // vec3 textureColor = texture(texSampler, fragTexCoord).rgb;
    // outColor = vec4(textureColor, 1.0);
    outColor = vec4(fragTexCoord, 0.0, 1.0);
    // outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
