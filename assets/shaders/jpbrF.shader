#version 430 core

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define PI 3.14159265359

out vec4 color;
in vec3 worldPos;
in vec2 texCoord;
in mat3 TBN;

uniform vec3 camPos;

uniform bool useAlbedoTexture = false;
uniform bool useNormalTexture = false;
uniform bool useMetalnessTexture = false;
uniform bool useRoughnessTexture = false;
uniform bool useEmissiveTexture = false;
uniform bool useAoTexture = false;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D aoTexture;

uniform vec3 dLightDir[MAX_DIR_LIGHTS];
uniform vec3 dLightRad[MAX_DIR_LIGHTS];
uniform vec3 pLightPos[MAX_POINT_LIGHTS];
uniform vec3 pLightRad[MAX_POINT_LIGHTS];
uniform float pLightRa[MAX_POINT_LIGHTS];

vec3 albedo, normalPixel, emissive;
float metalness, roughness, ao;

vec3 N, V, R, F0;

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------

vec3 computeLightContribution(vec3 L, vec3 radiance)
{
    // cook-torrance brdf
    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main()
{
    if (useAlbedoTexture)
        albedo = texture(albedoTexture, texCoord).rgb;
    else
        albedo = vec3(1.0);
    if (useNormalTexture)
        normalPixel = texture(normalTexture, texCoord).rgb;
    else
        normalPixel = vec3(0.5, 0.5, 1.0);
    if (useEmissiveTexture)
        emissive = texture(emissiveTexture, texCoord).rgb;
    else
        emissive = vec3(0.0);
    if (useMetalnessTexture)
        metalness = texture(metalnessTexture, texCoord).r;
    else
        metalness = 0.0;
    if (useRoughnessTexture)
        roughness = texture(roughnessTexture, texCoord).r;
    else
        roughness = 0.4;
    if (useAoTexture)
        ao = texture(aoTexture, texCoord).r;
    else
        ao = 1.0;

    // Get current fragment's normal and transform to world space.
    N = 2.0 * normalPixel - 1.0;
    N = normalize(TBN * N);

    V = normalize(camPos - worldPos);
    R = reflect(-V, N); 

    F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metalness);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < MAX_DIR_LIGHTS; ++i)
    {
        vec3 L = normalize(-dLightDir[i]);
        vec3 radiance = dLightRad[i];

        Lo += computeLightContribution(L, radiance);
    }
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        vec3 L = pLightPos[i] - worldPos;
        float dsq = dot(L, L);
        L = normalize(L);
        vec3 radiance = pLightRad[i] * pLightRa[i] / dsq;

        Lo += computeLightContribution(L, radiance);
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 temp = ambient + Lo;
    temp += emissive;

    temp = temp / (temp + vec3(1.0));
    temp = pow(temp, vec3(1.0 / 2.2));


    color = vec4(temp, 1.0);
}

