#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fTexCoords;

uniform samplerCube skybox;
uniform bool useExposure = false;
uniform float exposure = 5.0;

const float gamma = 2.2;

void main()
{
	if (useExposure)
	{
		vec3 hdrColor = texture(skybox, fTexCoords).rgb;

		// exposure tone mapping
		vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
		// gamma correction 
		mapped = pow(mapped, vec3(1.0 / gamma));

		outColor = vec4(mapped, 1.0);
	}
	else
	{
		vec3 envColor = textureLod(skybox, fTexCoords, 0.0).rgb;

		// HDR tonemap and gamma correct
		envColor = envColor / (envColor + vec3(1.0));
		envColor = pow(envColor, vec3(1.0 / 2.2));

		outColor = vec4(envColor, 1.0);
	}
}