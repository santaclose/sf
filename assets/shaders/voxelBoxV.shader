#version 430 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBitangent;
layout(location = 4) in vec2 aTextureCoord;
layout(location = 5) in vec2 aExtraData;

layout(std430, binding = 0) buffer SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	vec3 cameraPosition;
};

layout (std430, binding = 1) buffer VoxelMatricesBuffer
{
  mat4 voxelMatrices[];
};

out vec2 texCoord;

void main()
{
	texCoord = aTextureCoord;
	gl_Position = cameraMatrix * modelMatrix * voxelMatrices[gl_InstanceID] * vec4(aPosition, 1.0);
}