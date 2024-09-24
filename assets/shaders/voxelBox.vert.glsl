#version 460

layout(location = 4) out vec2 fTexCoords;

layout(location = 0) in vec3 vPosition;
layout(location = 5) in vec2 vTexCoords;

layout(binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

layout (binding = 1) buffer VoxelMatricesBuffer
{
	mat4 voxelMatrices[];
};

void main()
{
	fTexCoords = vTexCoords;
	gl_Position = cameraMatrix * modelMatrix * voxelMatrices[gl_InstanceID] * vec4(vPosition, 1.0);
}