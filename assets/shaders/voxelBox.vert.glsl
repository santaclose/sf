#version 460

#include <assets/shaders/particle.h>

layout(location = 4) out vec2 fTexCoords;

layout(location = 0) in vec3 vPosition;
layout(location = 5) in vec2 vTexCoords;

layout(binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	mat4 screenSpaceMatrix;
	vec3 cameraPosition;
};

layout (std430, binding = 1) buffer PerParticleBuffer
{
	float perParticleData[];
};

void main()
{
	fTexCoords = vTexCoords;
	gl_Position = cameraMatrix * vec4(PARTICLE_INITIAL_POSITION + vPosition * PARTICLE_INITIAL_SCALE, 1.0);
}