#version 460

layout(location = 4) out vec2 fTexCoords;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec3 vColor;
layout(location = 5) in vec2 vTexCoords;
layout(location = 6) in float vAmbientOcclusion;

layout(binding = 0) uniform SharedGpuData
{
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
} sgd;

layout(binding = 1) buffer VoxelMatricesBuffer
{
	mat4 voxelMatrices[];
};

layout(push_constant) uniform constants
{
	mat4 modelMatrix;
} pc;

void main()
{
	fTexCoords = vTexCoords;
	gl_Position = sgd.cameraMatrix * pc.modelMatrix * voxelMatrices[gl_InstanceIndex] * vec4(vPosition, 1.0);
}