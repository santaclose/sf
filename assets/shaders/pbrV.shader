#version 430 core

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBitangent;
layout(location = 4) in vec2 aTextureCoord;

out vec3 worldPos;
out vec2 texCoord;
out mat3 TBN;

uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

void main()
{
	texCoord = aTextureCoord;

	vec3 T = normalize(vec3(modelMatrix * vec4(aTangent, 0.0)));
	vec3 B = normalize(vec3(modelMatrix * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(modelMatrix * vec4(aNormal, 0.0)));
	TBN = mat3(T, B, N);

	worldPos = (modelMatrix * aPosition).rgb;

	gl_Position = cameraMatrix * modelMatrix * aPosition;
}