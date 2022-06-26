#version 430 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in float aAo;


layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	vec3 cameraPosition;
};

out float vertexAo;

void main()
{
	vertexAo = aAo;
	gl_Position = cameraMatrix * modelMatrix * vec4(aPosition, 1.0);
}