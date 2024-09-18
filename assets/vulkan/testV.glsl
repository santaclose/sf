#version 450


layout(binding = 0) uniform SharedGpuData
{
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
} sgd;

layout(push_constant) uniform constants
{
	mat4 modelMatrix;
} pc;

layout(location = 0) in vec3 vPosition;
layout(location = 5) in vec2 vTexCoords;

layout(location = 0) out vec3 fragColor;

void main()
{
	fragColor = vec3(vTexCoords.x, vTexCoords.y, 0.0);
	gl_Position = sgd.cameraMatrix * pc.modelMatrix * vec4(vPosition, 1.0);
}