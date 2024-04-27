#version 450
#extension GL_ARB_separate_shader_objects : enable

const float PI = 3.14159265359;

// texture
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 pointOnWorld;

layout(location = 0) out vec4 outColor;

vec3 lightPose = vec3(0.0, 0.0, 1.0);
vec3 lightPower = vec3(23.47, 21.31, 20.79);

vec3 albedo = vec3(1.0);

void main() {
    vec3 wi = normalize(pointOnWorld.xyz - lightPose);
    float lightDist = distance(lightPose, pointOnWorld.xyz);
    vec3 lightIrradianceOnSp = lightPower / (4.0 * PI * lightDist * lightDist);
    vec3 fr = albedo / PI;

    vec3 outRadiance = fr * lightIrradianceOnSp * clamp(dot(-wi, normal), 0.0, 1.0);
    outColor = vec4(outRadiance, 1.0);
}