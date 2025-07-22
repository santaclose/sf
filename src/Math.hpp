#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace sf {

	glm::vec3 operator/(const glm::vec3& v, const float& f);
	glm::vec3 operator*(const glm::vec3& v, const float& f);

	namespace Math {

		extern const float Pi;

		int Mod(int a, int b);

		void WeightedBlend(const float* input, const float* weights, int count, float& output);
		void WeightedBlend(const glm::vec2* input, const float* weights, int count, glm::vec2& output);
		void WeightedBlend(const glm::vec3* input, const float* weights, int count, glm::vec3& output);
		void WeightedBlend(const glm::quat* input, const float* weights, int count, glm::quat& output);
	}
}