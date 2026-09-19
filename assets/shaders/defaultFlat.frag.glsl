layout(location = 3) in vec3 fWorldPos;
layout(location = 4) in vec2 fTexCoords;

void main()
{
	vec3 normal = normalize(cross(dFdx(fWorldPos), dFdy(fWorldPos)));

	vec3 lightDir = vec3(1.0, -1.0, -1.0);
	float diff = max(dot(-normal, normalize(lightDir)), 0.0);
	vec4 texColor = vec4(1.0);
	OUT_COLOR = vec4(texColor.rgb * diff, 1.0);
}