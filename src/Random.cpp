#include "Random.h"
#include <cmath>

void Random::SetSeed(unsigned int seed)
{
	srand(seed);
}

float Random::Float()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

int Random::Int(int limit)
{
	return rand() % limit;
}
