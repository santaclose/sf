#include "Math.h"

#define EPSILON 0.001f

glm::vec3 sf::operator/(const glm::vec3& v, const float& f)
{
	return glm::vec3(v.x / f, v.y / f, v.z / f);
}
glm::vec3 sf::operator*(const glm::vec3& v, const float& f)
{
	return glm::vec3(v.x * f, v.y * f, v.z * f);
}

namespace sf {

	const float Math::Pi = 3.14159265358979323846264;
}

bool sf::Math::RayTriIntersect(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float* out_t)
{
	glm::vec3 normal = glm::cross(v1 - v0, v2 - v0);
	float q = glm::dot(normal, dir);
	if (q == 0) return false;

	float d = -glm::dot(normal, v0);
	float t = -(glm::dot(normal, origin) + d) / q;
	if (t < 0) return false;

	glm::vec3 hit_point = origin + dir * t;

	glm::vec3 edge0 = v1 - v0;
	glm::vec3 VP0 = hit_point - v0;
	if (glm::dot(normal, glm::cross(edge0, VP0)) < 0)
		return false;

	glm::vec3 edge1 = v2 - v1;
	glm::vec3 VP1 = hit_point - v1;
	if (glm::dot(normal, glm::cross(edge1, VP1)) < 0)
		return false;

	glm::vec3 edge2 = v0 - v2;
	glm::vec3 VP2 = hit_point - v2;
	if (glm::dot(normal, glm::cross(edge2, VP2)) < 0)
		return false;

	if (out_t) *out_t = t;
	return true;
}