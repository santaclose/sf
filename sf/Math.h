#pragma once
#include <glm/glm.hpp>

namespace Math
{
	float Random();
	int RandomInt(int limit);
}

glm::vec3 operator/(const glm::vec3& v, const float& f);
glm::vec3 operator*(const glm::vec3& v, const float& f);