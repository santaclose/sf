#version 430 core

layout(location = 0) in vec3 vPosition;

out vec2 fScreenPos;

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

void main()
{
	gl_Position = cameraMatrix * modelMatrix * vec4(vPosition, 1.0);
	fScreenPos = gl_Position.xy;
}