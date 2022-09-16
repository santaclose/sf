#version 430 core

layout(location = 0) in vec3 vPosition;
layout(location = 5) in vec2 vTexCoords;

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

layout (std430, binding = 1) buffer VoxelMatricesBuffer
{
	mat4 voxelMatrices[];
};

out vec2 fTexCoords;

void main()
{
	fTexCoords = vTexCoords;
	gl_Position = cameraMatrix * modelMatrix * voxelMatrices[gl_InstanceID] * vec4(vPosition, 1.0);
}