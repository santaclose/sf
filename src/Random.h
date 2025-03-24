#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace sf::Random
{
	void SetSeed(uint32_t seed);
	float Float();
	int Int(int limit);
	glm::vec2 UnitVec2();
	glm::vec3 UnitVec3();
	glm::quat Rotation();

	glm::vec2 PointInCircle();
	glm::vec3 PointInSphere();
}