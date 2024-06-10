#version 450
#extension GL_ARB_separate_shader_objects : enable

const float PI = 3.14159265359;
const float EPS = 0.0001;
const vec3 F0_BASE = vec3(0.04);
const float GAMA = 2.2;

layout(binding = 0) uniform GlobalMatrixVP {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} globalMatrixVP;

layout(binding = 10) uniform sampler2D texRoughness;
layout(binding = 11) uniform sampler2D texMatallic;
layout(binding = 12) uniform sampler2D texAlbedo;
layout(binding = 13) uniform sampler2D texNormal;

// in
layout(location = 0) in VERT_OUT {
    vec2 texCoord;
    vec4 pointOnWorld;
    mat3 matTBN;
} fragIn;

// out
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
    // sample texture
    float sampleRoughness = texture(texRoughness, fragIn.texCoord).x;
    float sampleMetallic = texture(texMatallic, fragIn.texCoord).x;
    vec3 sampleAlbedo = pow(texture(texAlbedo, fragIn.texCoord).rgb, vec3(2.2));
    vec3 sampleNormal = texture(texNormal, fragIn.texCoord).xyz;
    sampleNormal = normalize(sampleNormal * 2.0 - 1.0);
    sampleNormal = normalize(fragIn.matTBN * sampleNormal);

    vec3 cameraPosOnWorld = globalMatrixVP.cameraPos;
    
    vec3 outRadiance = vec3(0);

    vec3 wo = normalize(cameraPosOnWorld - fragIn.pointOnWorld.xyz);
    float dotNToWo = max(dot(sampleNormal, wo), 0.0);

    vec3 F0 = mix(F0_BASE, sampleAlbedo, sampleMetallic);

    float G2 = GeometyGGX(sampleRoughness, dotNToWo);

    for (int i = 0; i < 4; i++) {
        vec3 lightPose = lightPosList[i];
        vec3 lightPower = lightPowerList[i];

        vec3 wi = normalize(fragIn.pointOnWorld.xyz - lightPose);
        float dotNToWi = max(dot(sampleNormal, -wi), 0.0);

        vec3 halfVector = normalize((-wi) + wo);

        float lightDist = distance(lightPose, fragIn.pointOnWorld.xyz);
        vec3 lightIrradianceOnSp = lightPower / (4.0 * PI * lightDist * lightDist);

        vec3 FTerm = FresnelSchlick(F0, max(dot(halfVector, wo), 0.0));
        float DTerm = DistributionGGX(sampleRoughness, sampleNormal, halfVector);
        float G1 = GeometyGGX(sampleRoughness, dotNToWi);

        vec3 kS = FTerm;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - sampleMetallic;

        vec3 fr = kD * sampleAlbedo / PI + FTerm * DTerm * G1 * G2 / (4.0 * dotNToWi * dotNToWo + EPS);

        outRadiance += fr * lightIrradianceOnSp * dotNToWi;
    }

    // ambient
    vec3 ambient = gAmbient * sampleAlbedo;
    vec3 color = ambient + outRadiance;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / GAMA)); 

    outColor = vec4(color, 1.0);
}