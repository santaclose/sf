layout(location = 0) in mat3 fTBN;
layout(location = 4) in vec2 fTexCoords;

void main()
{
	vec3 normal = vec3(0.5, 0.5, 1.0);
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(fTBN * normal);

	vec3 lightDir = vec3(1.0, -1.0, -1.0);
	float diff = max(dot(-normal, normalize(lightDir)), 0.0);
	vec4 texColor = vec4(1.0);
	OUT_COLOR = vec4(texColor.rgb * diff, 1.0);
}