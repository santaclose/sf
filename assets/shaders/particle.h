#define PARTICLE_TIME_ALIVE(spawnTime) ((spawnTime == -1.0) ? \
	-1.0 : \
	((PARTICLE_CYCLE_TIME >= spawnTime) ? \
		(PARTICLE_CYCLE_TIME - spawnTime) : \
		(PARTICLE_CYCLE_TIME + (PARTICLE_LIFETIME - spawnTime))))