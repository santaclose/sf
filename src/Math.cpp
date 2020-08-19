#include "Math.h"

#include <cmath>

glm::vec3 operator/(const glm::vec3& v, const float& f)
{
	return glm::vec3(v.x / f, v.y / f, v.z / f);
}
glm::vec3 operator*(const glm::vec3& v, const float& f)
{
	return glm::vec3(v.x * f, v.y * f, v.z * f);
}
float Math::Random()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

int Math::RandomInt(int limit)
{
	return rand() % limit;
}
