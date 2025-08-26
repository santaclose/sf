#include <assets/shaders/particle.h>
#include <assets/shaders/quaternion.h>

layout(location = 0) out vec2 particleUV;
layout(location = 1) out float particleOpacity;

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

layout (std430, binding = 1) buffer _PARTICLE_BUFFER
{
	float PARTICLE_BUFFER[];
};

uniform float PARTICLE_CYCLE_TIME;
uniform float PARTICLE_LIFETIME;

void main()
{
	vec4 particleRotation = LOAD_PARTICLE_ROTATION;
	vec3 particlePosition = LOAD_PARTICLE_POSITION;
	float particleScale = LOAD_PARTICLE_SCALE;
	float particleSpawnTime = LOAD_PARTICLE_SPAWN_TIME;
	float particleTimeAlive = PARTICLE_TIME_ALIVE(particleSpawnTime);

	float timeRatio = particleTimeAlive / PARTICLE_LIFETIME;

	particleOpacity = 1.0 - timeRatio;
	particleOpacity *= 0.2f;
	particleUV = VA_UV;

	gl_Position = cameraMatrix * vec4(particlePosition + QuatRotateVector(VA_Position * particleScale, particleRotation), 1.0);
}