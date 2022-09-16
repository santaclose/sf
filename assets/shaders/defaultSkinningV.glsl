#version 430 core

layout(location = 0) in vec4 vBoneIndices;
layout(location = 1) in vec4 vBoneWeights;
layout(location = 2) in vec3 vPosition;
layout(location = 3) in vec3 vNormal;
layout(location = 4) in vec3 vTangent;
layout(location = 5) in vec3 vBitangent;
layout(location = 6) in vec3 vColor;
layout(location = 7) in vec2 vTexCoords;
layout(location = 8) in float vAo;

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

out mat3 fTBN;
out vec3 fWorldPos;
out vec2 fTexCoords;
out float fVertexAo;

uniform bool animate = false;

void main()
{
	fTexCoords = vTexCoords;
	fVertexAo = vAo;

	mat4 skinMat = animate ?
		vBoneWeights.x * skinningMatrices[int(vBoneIndices.x)] +
		vBoneWeights.y * skinningMatrices[int(vBoneIndices.y)] +
		vBoneWeights.z * skinningMatrices[int(vBoneIndices.z)] +
		vBoneWeights.w * skinningMatrices[int(vBoneIndices.w)] : mat4(1.0);

	vec3 T = normalize(vec3(modelMatrix * skinMat * vec4(vTangent, 0.0)));
	vec3 B = normalize(vec3(modelMatrix * skinMat * vec4(vBitangent, 0.0)));
	vec3 N = normalize(vec3(modelMatrix * skinMat * vec4(vNormal, 0.0)));
	fTBN = mat3(T, B, N);

	// standard normal passing
	//normal = (modelMatrix * vec4(vNormal, 0.0)).xyz;
	// for allowing non uniform scaling
	//normal = (transpose(inverse(modelMatrix))* vNormal).xyz;

	fWorldPos = (modelMatrix * skinMat * vec4(vPosition, 1.0)).rgb;
	gl_Position = cameraMatrix * modelMatrix * skinMat * vec4(vPosition, 1.0);
}