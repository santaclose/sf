#include "Random.h"

#include <Math.hpp>

void sf::Random::SetSeed(unsigned int seed)
{
	srand(seed);
}

float sf::Random::Float()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

int sf::Random::Int(int limit)
{
	return rand() % limit;
}

glm::vec3 sf::Random::UnitVec3()
{
	glm::vec3 rayDir;
	float phi = sf::Random::Float() * Math::Pi * 2.0f;
	float costheta = sf::Random::Float() * 2.0f - 1.0f;
	float theta = glm::acos(costheta);
	rayDir.x = glm::sin(theta) * glm::cos(phi);
	rayDir.y = glm::sin(theta) * glm::sin(phi);
	rayDir.z = glm::cos(theta);
	return rayDir;
}

glm::fquat sf::Random::Rotation()
{
	glm::fquat rot;
	float u = Float();
	float v = Float();
	float w = Float();

	rot.w = glm::sqrt(1 - u) * glm::sin(2 * Math::Pi * v);
	rot.x = glm::sqrt(1 - u) * glm::cos(2 * Math::Pi * v);
	rot.y = glm::sqrt(u) * glm::sin(2 * Math::Pi * w);
	rot.z = glm::sqrt(u) * glm::cos(2 * Math::Pi * w);

	return rot;
}
