#version 450
#define FRAGMENT
#ifdef VERTEX
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec3 vColor;
layout(location = 5) in vec2 vTexCoords;
layout(location = 6) in float vAo;
layout(location = 7) in vec4 vBoneIndices;
layout(location = 8) in vec4 vBoneWeights;
#endif

layout(binding = 0) uniform SharedGpuData
{
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	mat4 skyboxMatrix;
	vec3 cameraPosition;
} sgd;

layout(std140, set = 0, binding = 1) readonly buffer ParticleMatrices
{
	mat4 particleMatrices[];
} pm;

layout( push_constant ) uniform constants
{
	mat4 modelMatrix;
	int vertexShaderId;
	int fragmentShaderId;
} pc;

#define VA_POSITION vPosition
#define VA_NORMAL vNormal
#define VA_TANGENT vTangent
#define VA_BITANGENT vBitangent
#define VA_COLOR vColor
#define VA_TEX_COORDS vTexCoords
#define VA_AMBIENT_OCCLUSION vAo
#define VA_BONE_INDICES vBoneIndices
#define VA_BONE_WEIGHTS vBoneWeights

#define OBJECT_MATRIX pc.modelMatrix
#define CAMERA_MATRIX sgd.cameraMatrix
#define CAMERA_POSITION sgd.cameraPosition
#define SCREEN_SPACE_MATRIX sgd.screenSpaceMatrix
#define SKYBOX_MATRIX sgd.skyboxMatrix

#define VERTEX_SHADER_ID pc.vertexShaderId
#define FRAGMENT_SHADER_ID pc.fragmentShaderId

#define PARTICLE_MATRICES pm.particleMatrices
#define SKINNING_MATRICES pm.particleMatrices

#define INSTANCE_INDEX gl_InstanceIndex

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define PI 3.14159265359
layout(set = 0, binding = 2) uniform UniformBlock {
	float pLightRa[MAX_POINT_LIGHTS];
	vec3 dLightDir[MAX_DIR_LIGHTS];
	vec3 dLightRad[MAX_DIR_LIGHTS];
	float exposure; //= 5.0;
	bool useRoughnessTexture; //= false;
	vec3 pLightRad[MAX_POINT_LIGHTS];
	vec3 color; //= vec3(0.0, 0.0, 0.0);
	bool useExposure; //= false;
	bool useVertexAo; //= false;
	bool useMetalnessTexture; //= false;
	bool useAoTexture; //= false;
	bool useAlbedoTexture; //= false;
	bool useNormalTexture; //= false;
	bool useEmissiveTexture; //= false;
	vec3 pLightPos[MAX_POINT_LIGHTS];
};
layout(binding = 3) uniform samplerCube irradianceMap;
layout(binding = 4) uniform sampler2D aoTexture;
layout(binding = 5) uniform sampler2D albedoTexture;
layout(binding = 6) uniform sampler2D metalnessTexture;
layout(binding = 7) uniform sampler2D normalTexture;
layout(binding = 8) uniform samplerCube skybox;
layout(binding = 9) uniform sampler2D brdfLUT;
layout(binding = 10) uniform sampler2D emissiveTexture;
layout(binding = 11) uniform sampler2D roughnessTexture;
layout(binding = 12) uniform sampler2D bitmap;
layout(binding = 13) uniform samplerCube prefilterMap;
layout(location = 0) in VertexOut {
	float fVertexAofloat;
	vec2 fTexCoordsvec2;
	vec3 fWorldPosvec3;
	mat3 fTBNmat3;
	vec3 fTexCoordsvec3;
};
layout(location = 0) out vec4 outColorvec4;




void main0()
{
	vec3 normal = vec3(0.5, 0.5, 1.0);
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(fTBNmat3 * normal);

	vec3 lightDir = vec3(1.0, -1.0, -1.0);
	float diff = max(dot(-normal, normalize(lightDir)), 0.0);
	vec4 texColor = vec4(1.0);
	outColorvec4 = vec4(texColor.rgb * diff, 1.0);
}






// IBL

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

void main1()
{
	albedo = useAlbedoTexture ? pow(texture(albedoTexture, fTexCoordsvec2).rgb, vec3(2.2)) : vec3(1.0);
	emissive = useEmissiveTexture ? pow(texture(emissiveTexture, fTexCoordsvec2).rgb, vec3(2.2)) : vec3(0.0);

	normalPixel = useNormalTexture ? texture(normalTexture, fTexCoordsvec2).rgb : vec3(0.5, 0.5, 1.0);
	metalness = useMetalnessTexture ? texture(metalnessTexture, fTexCoordsvec2).r : 0.0;
	roughness = useRoughnessTexture ? texture(roughnessTexture, fTexCoordsvec2).r : 0.4;
	ao = useAoTexture ? texture(aoTexture, fTexCoordsvec2).r : 1.0;
	ao = useVertexAo ? min(fVertexAofloat, ao) : ao;

	// Get current fragment's normal and transform to world space.
	N = 2.0 * normalPixel - 1.0;
	N = normalize(fTBNmat3 * N);

	V = normalize(CAMERA_POSITION - fWorldPosvec3);
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
		vec3 L = pLightPos[i] - fWorldPosvec3;
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

	outColorvec4 = vec4(temp, 1.0);
}



void main2()
{
	outColorvec4 = texture(skybox, fTexCoordsvec3);
}



const float gamma = 2.2;

void main3()
{
	if (useExposure)
	{
		vec3 hdrColor = texture(skybox, fTexCoordsvec3).rgb;

		// exposure tone mapping
		vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
		// gamma correction 
		mapped = pow(mapped, vec3(1.0 / gamma));

		outColorvec4 = vec4(mapped, 1.0);
	}
	else
	{
		vec3 envColor = textureLod(skybox, fTexCoordsvec3, 0.0).rgb;

		// HDR tonemap and gamma correct
		envColor = envColor / (envColor + vec3(1.0));
		envColor = pow(envColor, vec3(1.0 / 2.2));

		outColorvec4 = vec4(envColor, 1.0);
	}
}



void main4()
{
	outColorvec4 = vec4(color, 1.0);
}



void main5()
{
	outColorvec4 = texture(bitmap, fTexCoordsvec2);
}


void main6()
{
	outColorvec4 = vec4(fTexCoordsvec2.x, fTexCoordsvec2.y, 0.0, 1.0);
}



void main7()
{
	outColorvec4 = vec4(vec3(pow(fVertexAofloat, 1.0 / 2.2)), 1.0);
}
void main()
{
#ifdef VERTEX
	switch (VERTEX_SHADER_ID)
#else
	switch (FRAGMENT_SHADER_ID)
#endif
	{
		case 0: main0(); break;
		case 1: main1(); break;
		case 2: main2(); break;
		case 3: main3(); break;
		case 4: main4(); break;
		case 5: main5(); break;
		case 6: main6(); break;
		case 7: main7(); break;
	}
}
// __sh__ assets/shaders/defaultF.glsl
// __sh__ assets/shaders/pbrF.glsl
// __sh__ assets/shaders/skyboxF.glsl
// __sh__ assets/shaders/skyboxHdrF.glsl
// __sh__ assets/shaders/solidColorF.glsl
// __sh__ assets/shaders/spriteF.glsl
// __sh__ assets/shaders/uvF.glsl
// __sh__ assets/shaders/vertexAoF.glsl