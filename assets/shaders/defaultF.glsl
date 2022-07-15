#version 430 core

in vec2 fTexCoords;
in mat3 fTBN;

out vec4 outColor;

void main()
{
	vec3 normal = vec3(0.5, 0.5, 1.0);
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(fTBN * normal);

	vec3 lightDir = vec3(1.0, -1.0, -1.0);
	float diff = max(dot(-normal, normalize(lightDir)), 0.0);
	vec4 texColor = vec4(1.0);
	outColor = vec4(texColor.rgb * diff, 1.0);
}