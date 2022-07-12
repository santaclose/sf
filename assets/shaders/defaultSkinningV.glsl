#version 430 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBitangent;
layout(location = 4) in vec3 aColor;
layout(location = 5) in vec2 aTextureCoord;
layout(location = 6) in float aAo;
layout(location = 7) in vec4 aBoneIndices;
layout(location = 8) in vec4 aBoneWeights;

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

layout (std430, binding = 1) buffer SkinningMatricesBuffer
{
	mat4 skinningMatrices[];
};

out vec3 worldPos;
out vec2 texCoord;
out mat3 TBN;
out float vertexAo;

uniform bool animate = false;

void main()
{
	texCoord = aTextureCoord;
	vertexAo = aAo;

	mat4 skinMat = animate ?
		aBoneWeights.x * skinningMatrices[int(aBoneIndices.x)] +
		aBoneWeights.y * skinningMatrices[int(aBoneIndices.y)] +
		aBoneWeights.z * skinningMatrices[int(aBoneIndices.z)] +
		aBoneWeights.w * skinningMatrices[int(aBoneIndices.w)] : mat4(1.0);

	vec3 T = normalize(vec3(modelMatrix * skinMat * vec4(aTangent, 0.0)));
	vec3 B = normalize(vec3(modelMatrix * skinMat * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(modelMatrix * skinMat * vec4(aNormal, 0.0)));
	TBN = mat3(T, B, N);

	// standard normal passing
	//normal = (modelMatrix * vec4(aNormal, 0.0)).xyz;
	// for allowing non uniform scaling
	//normal = (transpose(inverse(modelMatrix))* aNormal).xyz;

	worldPos = (modelMatrix * skinMat * vec4(aPosition, 1.0)).rgb;
	gl_Position = cameraMatrix * modelMatrix * skinMat * vec4(aPosition, 1.0);
}