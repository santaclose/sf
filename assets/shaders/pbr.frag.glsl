#version 460

#include <assets/shaders/shared.h>

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define PI 3.14159265359

layout(location = 0) out vec4 outColor;

layout(location = 0) in mat3 fTBN;
layout(location = 3) in vec3 fWorldPos;
layout(location = 4) in vec2 fTexCoords;
layout(location = 5) in float fVertexAo;

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	float cameraPositionX;
	float cameraPositionY;
	float cameraPositionZ;
	float windowSizeX;
	float windowSizeY;
};

uniform bool useVertexAo = false;

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

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

vec3 albedo, normalPixel, emissive;
float metalness, roughness, ao;

vec3 N, V, R, F0;

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
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
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
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
	albedo = useAlbedoTexture ? pow(texture(albedoTexture, fTexCoords).rgb, vec3(2.2)) : vec3(1.0);
	emissive = useEmissiveTexture ? pow(texture(emissiveTexture, fTexCoords).rgb, vec3(2.2)) : vec3(0.0);

	normalPixel = useNormalTexture ? texture(normalTexture, fTexCoords).rgb : vec3(0.5, 0.5, 1.0);
	metalness = useMetalnessTexture ? texture(metalnessTexture, fTexCoords).r : 0.0;
	roughness = useRoughnessTexture ? texture(roughnessTexture, fTexCoords).r : 0.4;
	ao = useAoTexture ? texture(aoTexture, fTexCoords).r : 1.0;
	ao = useVertexAo ? min(fVertexAo, ao) : ao;

	// Get current fragment's normal and transform to world space.
	N = 2.0 * normalPixel - 1.0;
	N = normalize(fTBN * N);

	V = normalize(CAMERA_POSITION_VEC - fWorldPos);
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
		vec3 L = pLightPos[i] - fWorldPos;
		float dsq = dot(L, L);
		L = normalize(L);
		vec3 radiance = pLightRad[i] * pLightRa[i] / dsq;

		Lo += computeLightContribution(L, radiance);
	}

	// ambient lighting (we now use IBL as the ambient term)
	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metalness;

	vec3 irradiance = texture(irradianceMap, N).rgb;
	vec3 diffuse = irradiance * albedo;

	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
	int prefilterTextureLevels = textureQueryLevels(prefilterMap);
	vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * prefilterTextureLevels).rgb;
	vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

	vec3 ambient = (kD * diffuse + specular) * ao;

	vec3 temp = ambient + Lo;
	temp += emissive;

	// HDR tonemapping
	temp = temp / (temp + vec3(1.0));
	// gamma correct
	temp = pow(temp, vec3(1.0 / 2.2));

	outColor = vec4(temp, 1.0);
}