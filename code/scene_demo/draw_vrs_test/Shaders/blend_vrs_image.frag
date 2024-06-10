#version 450
#extension GL_ARB_separate_shader_objects : enable

// texture
layout(binding = 0) uniform usampler2D vrsImage;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

uint shadingRate1x1 = 0x00;
uint shadingRate1x2 = 0x01;
uint shadingRate2x1 = 0x04;
uint shadingRate2x2 = 0x05;
uint shadingRate2x4 = 0x06;
uint shadingRate4x2 = 0x09;
uint shadingRate4x4 = 0x0A;

void main() {

    uint shadingRate = texture(vrsImage, fragTexCoord).r;

    vec3 color = vec3(0.0);

    if (shadingRate == shadingRate1x1) {
        color = vec3(0.0, 0.0, 0.0);
    } else if (shadingRate == shadingRate1x2) {
        color = vec3(0.0, 0.0, 1.0);
    } else if (shadingRate == shadingRate2x1) {
        color = vec3(0.0, 0.0, 0.7);
    } else if (shadingRate == shadingRate2x2) {
        color = vec3(0.0, 1.0, 0.0);
    } else if (shadingRate == shadingRate2x4) {
        color = vec3(1.0, 1.0, 0.0);
    } else if (shadingRate == shadingRate4x2) {
        color = vec3(1.0, 0.7, 0.0);
    } else if (shadingRate == shadingRate4x4) {
        color = vec3(1.0, 0.0, 0.0);
    }
    
    outColor = vec4(color, 0.3);
}