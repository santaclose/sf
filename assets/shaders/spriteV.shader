#version 430 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTextureCoord;

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

out vec2 texCoord;

void main()
{
	texCoord = aTextureCoord;
	gl_Position = screenSpaceMatrix * vec4(aPosition, 0.0, 1.0);
}