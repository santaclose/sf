#pragma once
#include <glm/glm.hpp>

namespace Math
{
	float Random();
}

glm::vec3 operator/(const glm::vec3& v, const float& f);
glm::vec3 operator*(const glm::vec3& v, const float& f);