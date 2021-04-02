#pragma once
#include <glm/glm.hpp>

namespace sf {

	glm::vec3 operator/(const glm::vec3& v, const float& f);
	glm::vec3 operator*(const glm::vec3& v, const float& f);

	namespace Math {

		extern const float Pi;

		bool RayTriIntersect(const glm::vec3& origin,
			const glm::vec3& dir,
			const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
			float* out_t = nullptr);
	}
}