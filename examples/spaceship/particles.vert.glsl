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

uniform float PARTICLE_CYCLE_TIME;
uniform float PARTICLE_LIFETIME;

void main()
{
	vec4 particleRotation = PARTICLES_LOAD_ROTATION;
	vec3 particlePosition = PARTICLES_LOAD_POSITION;
	float particleScale = PARTICLES_LOAD_SCALE;
	float particleSpawnTime = PARTICLES_LOAD_SPAWN_TIME;
	float particleTimeAlive = PARTICLE_TIME_ALIVE(particleSpawnTime);

	float timeRatio = particleTimeAlive / PARTICLE_LIFETIME;

	particleOpacity = 1.0 - timeRatio;
	particleOpacity *= 0.2f;
	particleUV = VA_UV;

	gl_Position = cameraMatrix * vec4(particlePosition + QuatRotateVector(VA_Position * particleScale, particleRotation), 1.0);
}