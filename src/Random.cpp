#include "Random.h"

#include <Math.hpp>

void sf::Random::SetSeed(uint32_t seed)
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

glm::quat sf::Random::Rotation()
{
	glm::quat rot;
	float u = Float();
	float v = Float();
	float w = Float();

	rot.w = glm::sqrt(1 - u) * glm::sin(2 * Math::Pi * v);
	rot.x = glm::sqrt(1 - u) * glm::cos(2 * Math::Pi * v);
	rot.y = glm::sqrt(u) * glm::sin(2 * Math::Pi * w);
	rot.z = glm::sqrt(u) * glm::cos(2 * Math::Pi * w);

	return rot;
}

glm::vec2 sf::Random::PointInCircle()
{
	float angle = Float() * 2.0f * glm::pi<float>();
	float radius = Float();
	float sep = glm::sqrt(radius);
	return glm::vec2(glm::cos(angle) * sep, glm::sin(angle) * sep);
}

glm::vec3 sf::Random::PointInSphere()
{
	glm::vec3 point = UnitVec3();
	float radius = Float();
	return point * glm::pow(radius, 1.0f / 3.0f);
}
