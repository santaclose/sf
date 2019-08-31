#version 330 core

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);
const float PI = 3.141592;
const float Epsilon = 0.00001;

out vec4 color;
in vec3 worldPos;
in vec2 texCoord;
in mat3 TBN;

uniform vec3 camPos;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
	// define lights
	bool lightType[3];
	lightType[0] = lightType[1] = true;
	lightType[2] = false;
	vec3 lightPos[3];
	lightPos[0] = vec3(-5.0, 0.0, 0.0);
	lightPos[1] = vec3(5.0, 0.0, 0.0);
	vec3 lightRadiance[3];
	lightRadiance[0] = vec3(1.0, 0.0, 1.0);
	lightRadiance[1] = vec3(1.0, 1.0, 1.0);
	lightRadiance[2] = vec3(1.0, 1.0, 1.0);
	float lightRadius[2];
	lightRadius[0] = 30.0;
	lightRadius[1] = 50000.0;
	// directional light
	lightPos[2] = vec3(1.0, -1.0, -1.0);

	// Sample input textures to get shading model params.
	vec3 albedo = texture(albedoTexture, texCoord).rgb;
	float metalness = texture(metalnessTexture, texCoord).r;
	float roughness = texture(roughnessTexture, texCoord).r;

	// Outgoing light direction (vector from world-space fragment position to the "eye").
	vec3 Lo = normalize(camPos - worldPos);

	// Get current fragment's normal and transform to world space.
	vec3 N = normalize(2.0 * texture(normalTexture, texCoord).rgb - 1.0);
	N = normalize(TBN * N);

	// Angle between surface normal and outgoing light direction.
	float cosLo = max(0.0, dot(N, Lo));

	// Specular reflection vector.
	vec3 Lr = 2.0 * cosLo * N - Lo;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
	vec3 F0 = mix(Fdielectric, albedo, metalness);

	// Direct lighting calculation for analytical lights.
	vec3 directLighting = vec3(0);

	for (int i = 0; i < 3; ++i) // for each light
	{
		vec3 Li;
		vec3 Lradiance;
		if (lightType[i] == true) {
			Li = normalize(lightPos[i] - worldPos); //-lights[i].direction;
			float d = distance(lightPos[i], worldPos);
			Lradiance = lightRadiance[i] * lightRadius[i] / d / d; //lights[i].radiance;
		}
		else
		{
			Li = normalize(-lightPos[i]); //-lights[i].direction;
			Lradiance = lightRadiance[i];
		}

		// Half-vector between Li and Lo.
		vec3 Lh = normalize(Li + Lo);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(N, Li));
		float cosLh = max(0.0, dot(N, Lh));

		// Calculate Fresnel term for direct lighting. 
		vec3 F = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		// Calculate normal distribution for specular BRDF.
		float D = ndfGGX(cosLh, roughness);
		// Calculate geometric attenuation for specular BRDF.
		float G = gaSchlickGGX(cosLi, cosLo, roughness);

		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
		vec3 diffuseBRDF = kd * albedo;

		// Cook-Torrance specular microfacet BRDF.
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
		directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	}

	// Final fragment color.
	color = vec4(directLighting, 1.0);
}