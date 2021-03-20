#version 430 core

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10

const vec3 Fdielectric = vec3(0.04);
const float PI = 3.141592;
const float Epsilon = 0.00001;

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

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D emissiveTexture;

uniform vec3 dLightDir[MAX_DIR_LIGHTS];
uniform vec3 dLightRad[MAX_DIR_LIGHTS];
uniform vec3 pLightPos[MAX_POINT_LIGHTS];
uniform vec3 pLightRad[MAX_POINT_LIGHTS];
uniform vec3 pLightRa[MAX_POINT_LIGHTS];

vec3 albedo, normalPixel, emissive;
float metalness, roughness;

float cosLo;
vec3 Lo, N, F0;

float ndfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 computeLightContribution(vec3 Li, vec3 Lradiance)
{
	vec3 Lh = normalize(Li + Lo);

	float cosLi = max(0.0, dot(N, Li));
	float cosLh = max(0.0, dot(N, Lh));

	vec3 F = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
	float D = ndfGGX(cosLh, roughness);
	float G = gaSchlickGGX(cosLi, cosLo, roughness);

	vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);
	vec3 diffuseBRDF = kd * albedo;

	vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);
	return (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
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

	Lo = normalize(camPos - worldPos);
	N = 2.0 * normalPixel - 1.0;
	N = normalize(TBN * N);
	cosLo = max(0.0, dot(N, Lo));
	vec3 Lr = 2.0 * cosLo * N - Lo;
	F0 = mix(Fdielectric, albedo, metalness);
	vec3 directLighting = vec3(0);

	for (int i = 0; i < MAX_DIR_LIGHTS; i++)
	{
		vec3 Li = normalize(-dLightDir[i]);
		vec3 Lradiance = dLightRad[i];
		directLighting += computeLightContribution(Li, Lradiance);
	}
	for (int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		vec3 Li = pLightPos[i] - worldPos;
		float dsq = dot(Li, Li);
		Li = normalize(Li);
		vec3 Lradiance = pLightRad[i] * pLightRa[i] / dsq;
		directLighting += computeLightContribution(Li, Lradiance);
	}

	color = vec4(directLighting, 1.0);
	color.rgb += emissive;
	color.rgb = color.rgb / (color.rgb + vec3(1.0));
	color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
}