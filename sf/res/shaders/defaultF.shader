#version 330 core

uniform sampler2D albedo;
uniform sampler2D normalMap;

in vec3 normal;
in vec2 texCoord;
out vec4 color;

void main()
{
	// obtain normal from normal map in range [0,1]
	//vec3 texNormal = texture(normalMap, texCoord).rgb;
	// transform normal vector to range [-1,1]
	//texNormal = texNormal * 2.0 - 1.0;

	vec3 finalNormal = /*texNormal + */normal;

	vec3 lightDir = vec3(1.0, -1.0, -1.0);
	float diff = max(dot(normalize(-finalNormal), normalize(lightDir)), 0.0);
	vec4 texColor = texture(albedo, texCoord);
	color = vec4(texColor.rgb * diff, 1.0);
}