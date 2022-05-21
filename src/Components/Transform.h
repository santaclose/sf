#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace sf {

	struct Transform
	{
		glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
		glm::fquat rotation = glm::fquat(1.0, 0.0, 0.0, 0.0);
		float scale = 1.0f;

		Transform() = default;

		glm::mat4 ComputeMatrix() const;

		glm::vec3 Forward() const;
		glm::vec3 Right() const;
		glm::vec3 Up() const;

		void LookAt(const glm::vec3& target, const glm::vec3& up);
	};
}