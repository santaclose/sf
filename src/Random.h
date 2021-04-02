#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace sf::Random
{
	void SetSeed(unsigned int seed);
	float Float();
	int Int(int limit);
	glm::vec3 UnitVec3();
	glm::fquat Rotation();
}