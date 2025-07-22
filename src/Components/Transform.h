#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace sf {

	struct Transform
	{
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		float scale = 1.0f;

		inline glm::mat4 ComputeMatrix() const
		{
			glm::mat4 outputMatrix = glm::translate(glm::mat4(1.0), position);
			glm::mat4 rotationMatrix = (glm::mat4)rotation;
			outputMatrix *= rotationMatrix;
			outputMatrix = glm::scale(outputMatrix, glm::vec3(scale, scale, scale));
			return outputMatrix;
		}

		inline glm::vec3 Forward() const
		{
			return rotation * glm::vec3(0, 0, -1);
		}

		inline glm::vec3 Right() const
		{
			return rotation * glm::vec3(1, 0, 0);
		}

		inline glm::vec3 Up() const
		{
			return rotation * glm::vec3(0, 1, 0);
		}

		inline void LookAt(const glm::vec3& target, const glm::vec3& up)
		{
			rotation = glm::quatLookAt(glm::normalize(target - position), up);
		}
	};
}