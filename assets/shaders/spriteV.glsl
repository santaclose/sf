#version 430 core

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vTextureCoords;

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

out vec2 fTextureCoords;

void main()
{
	fTextureCoords = vTextureCoords;
	gl_Position = screenSpaceMatrix * vec4(vPosition, 0.0, 1.0);
}