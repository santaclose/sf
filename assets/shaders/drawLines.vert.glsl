#version 460

layout(location = 0) out vec3 fColor;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;

layout(std140, binding = 0) uniform SharedGpuData
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
	fColor = vColor;
	gl_Position = cameraMatrix * vec4(vPosition, 1.0);
}