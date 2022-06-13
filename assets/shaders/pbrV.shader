#version 430 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBitangent;
layout(location = 4) in vec2 aTextureCoord;
layout(location = 5) in vec2 aExtraData;

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	vec3 cameraPosition;
};

out vec3 worldPos;
out vec2 texCoord;
out mat3 TBN;
out vec2 extraData;

void main()
{
	texCoord = aTextureCoord;

	vec3 T = normalize(vec3(modelMatrix * vec4(aTangent, 0.0)));
	vec3 B = normalize(vec3(modelMatrix * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(modelMatrix * vec4(aNormal, 0.0)));
	TBN = mat3(T, B, N);

	worldPos = (modelMatrix * vec4(aPosition, 1.0)).rgb;

	gl_Position = cameraMatrix * modelMatrix * vec4(aPosition, 1.0);
}