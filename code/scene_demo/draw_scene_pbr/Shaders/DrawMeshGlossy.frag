#version 450
#extension GL_ARB_separate_shader_objects : enable

const float PI = 3.14159265359;
const float EPS = 0.0001;
const vec3 F0_BASE = vec3(0.04);
const float GAMA = 2.2;

layout(binding = 0) uniform UniformMvpMatrix {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} uMvp;

layout(binding = 1) uniform UniformMaterial {
    vec3 albedo;
    float roughness;
    float metallic;
} uMaterial;

layout(push_constant) uniform PushConsts {
    float roughness;
    float metallic;
    vec3 albedo;
    vec3 modelOffset;
} uConsts;

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 normalDir;
layout(location = 2) in vec4 pointOnWorld;

layout(location = 0) out vec4 outColor;

// light
vec3 lightPosList[4] = {
    vec3(10.0, 10.0, 10.0),
    vec3(10.0, 10.0, -10.0),
    vec3(10.0, -10.0, 10.0),
    vec3(10.0, -10.0, -10.0),
};
vec3 lightPowerList[4] = {
    vec3(5000.0, 5000.0, 5000.0),
    vec3(5000.0, 5000.0, 5000.0),
    vec3(5000.0, 5000.0, 5000.0),
    vec3(5000.0, 5000.0, 5000.0),
};
vec3 gAmbient = vec3(0.03);

float DistributionGGX(float roughness, vec3 normal, vec3 halfVector)
{
    float alpha = roughness * roughness;
    float alphaSquare = alpha * alpha;
    float dotNToH = clamp(dot(normal, halfVector), 0.0, 1.0);
    float denomTemp = dotNToH * dotNToH * (alphaSquare - 1.0) + 1.0;
    return alphaSquare / (PI * denomTemp * denomTemp);
}

vec3 FresnelSchlick(vec3 f0, float dotHalfToView)
{
    return f0 + (1.0 - f0) * pow(clamp(1.0 - dotHalfToView, 0.0, 1.0), 5);
}

float GeometyGGX(float roughness, float dotNtoV)
{
    float k = (roughness + 1) * (roughness + 1) * 0.125;
    return dotNtoV / (dotNtoV * (1.0 - k) + k);
}

void main()
{
    vec3 cameraPosOnWorld = uMvp.cameraPos;
    vec3 normal = normalize(normalDir);     // 法线插值后不再是单位相量，因此需要处理一下

    vec3 outRadiance = vec3(0);

    vec3 wo = normalize(cameraPosOnWorld - pointOnWorld.xyz);
    float dotNToWo = max(dot(normal, wo), 0.0);

    vec3 F0 = mix(F0_BASE, uConsts.albedo, uConsts.metallic);

    float G2 = GeometyGGX(uConsts.roughness, dotNToWo);

    for (int i = 0; i < 4; i++) {
        vec3 lightPose = lightPosList[i];
        vec3 lightPower = lightPowerList[i];

        vec3 wi = normalize(pointOnWorld.xyz - lightPose);
        float dotNToWi = max(dot(normal, -wi), 0.0);

        vec3 halfVector = normalize((-wi) + wo);

        float lightDist = distance(lightPose, pointOnWorld.xyz);
        vec3 lightIrradianceOnSp = lightPower / (4.0 * PI * lightDist * lightDist);

        vec3 FTerm = FresnelSchlick(F0, max(dot(halfVector, wo), 0.0));
        float DTerm = DistributionGGX(uConsts.roughness, normal, halfVector);
        float G1 = GeometyGGX(uConsts.roughness, dotNToWi);

        vec3 kS = FTerm;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - uConsts.metallic;
        
        vec3 fr = kD * uConsts.albedo / PI + FTerm * DTerm * G1 * G2 / (4.0 * dotNToWi * dotNToWo + EPS);

        outRadiance += fr * lightIrradianceOnSp * dotNToWi;
    }

    // ambient
    vec3 ambient = gAmbient * uConsts.albedo;
    vec3 color = ambient + outRadiance;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / GAMA)); 

    outColor = vec4(color, 1.0);
}