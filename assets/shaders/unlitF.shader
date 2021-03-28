#version 330 core

out vec4 color;
in vec3 worldPos;
in vec2 texCoord;
in mat3 TBN;

uniform vec3 camPos;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;

uniform vec3 pLightPos[2] = vec3[](vec3(5.0, 0.0, 0.0), vec3(-5.0, 0.0, 0.0));
uniform vec3 pLightRad[2] = vec3[](vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 1.0));
uniform float pLightRa[2] = float[](30.0, 30.0);

void main()
{
	vec3 albedo = texture(albedoTexture, texCoord).rgb;
	color.rgb = albedo;
	color.a = camPos.r / camPos.r;
}