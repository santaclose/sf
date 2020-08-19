#version 330 core

uniform sampler2D albedo;
uniform sampler2D normalMap;

//in vec3 normal;
in vec2 texCoord;
in mat3 TBN;
out vec4 color;

void main()
{
	vec3 normal = texture(normalMap, texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(TBN * normal);

	vec3 lightDir = vec3(1.0, -1.0, -1.0);
	float diff = max(dot(/*normalize(*/-normal/*)*/, normalize(lightDir)), 0.0);
	vec4 texColor = texture(albedo, texCoord);
	color = vec4(texColor.rgb * diff, 1.0);
}