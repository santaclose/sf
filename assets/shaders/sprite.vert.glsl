#version 460

#include <assets/shaders/shared.h>

layout(location = 4) out vec2 fTexCoords;

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vTexCoords;

layout(binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	float cameraPositionX;
	float cameraPositionY;
	float cameraPositionZ;
	float windowSizeX;
	float windowSizeY;
};

void main()
{
	fTexCoords = vTexCoords;
	gl_Position = PIXEL_SPACE_TO_GL_SPACE(vPosition);
}