
#define PARTICLE_INITIAL_POSITION vec3( \
		perParticleData[gl_InstanceID * 9 + 0], \
		perParticleData[gl_InstanceID * 9 + 1], \
		perParticleData[gl_InstanceID * 9 + 2])

#define PARTICLE_INITIAL_ROTATION vec4( \
		perParticleData[gl_InstanceID * 9 + 3], \
		perParticleData[gl_InstanceID * 9 + 4], \
		perParticleData[gl_InstanceID * 9 + 5], \
		perParticleData[gl_InstanceID * 9 + 6])

#define PARTICLE_INITIAL_SCALE perParticleData[gl_InstanceID * 9 + 7]

#define PARTICLE_LIFETIME (cycleTotalTime)
#define PARTICLE_TIME_ALIVE ((perParticleData[gl_InstanceID * 9 + 8] == -1.0) ? \
	-1.0 : \
	((cycleCurrentTime >= perParticleData[gl_InstanceID * 9 + 8]) ? \
		(cycleCurrentTime - perParticleData[gl_InstanceID * 9 + 8]) : \
		(cycleCurrentTime + (cycleTotalTime - perParticleData[gl_InstanceID * 9 + 8]))))