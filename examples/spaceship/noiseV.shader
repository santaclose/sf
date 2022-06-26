#version 430 core

layout(location = 0) in vec3 aPosition;

out vec2 screenPos;

layout(std140, binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	vec3 cameraPosition;
};

void main()
{
	gl_Position = cameraMatrix * modelMatrix * vec4(aPosition, 1.0);
	screenPos = gl_Position.xy;
}