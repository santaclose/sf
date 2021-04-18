#version 330 core

//in vec3 normal;
in vec2 texCoord;
in mat3 TBN;
out vec4 color;

void main()
{
	vec3 normal = vec3(0.5, 0.5, 1.0);//texture(normalMap, texCoord).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(TBN * normal);

	vec3 lightDir = vec3(1.0, -1.0, -1.0);
	float diff = max(dot(/*normalize(*/-normal/*)*/, normalize(lightDir)), 0.0);
	vec4 texColor = vec4(1.0); //texture(albedo, texCoord);
	color = vec4(texColor.rgb * diff, 1.0);
}