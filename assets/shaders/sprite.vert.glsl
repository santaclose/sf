#version 460

layout(location = 4) out vec2 fTexCoords;

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vTexCoords;

layout(binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

void main()
{
	fTexCoords = vTexCoords;
	gl_Position = screenSpaceMatrix * vec4(vPosition, 0.0, 1.0);
}