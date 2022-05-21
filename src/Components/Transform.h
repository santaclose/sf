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

		mutable bool matrixUpdatePending = true;
		mutable glm::mat4 matrix = glm::mat4(1.0f);

		void UpdateTransformMatrix() const;

		Transform() = default;

		void SetPosition(const glm::vec3& newPosition);
		void SetPosition(float x, float y, float z);
		void SetRotation(const glm::fquat& newRotation);
		void SetRotation(const glm::vec3& newRotation); // from euler angles
		void SetRotation(float x, float y, float z); // from euler angles
		void SetScale(float newScale);

		const glm::vec3& GetPosition() const;
		const glm::fquat& GetRotation() const;
		float GetScale() const;
		const glm::mat4& GetMatrix() const;
		glm::mat4 ComputeMatrix() const;

		glm::vec3 Forward() const;
		glm::vec3 Right() const;
		glm::vec3 Up() const;

		void LookAt(const glm::vec3& target, const glm::vec3& up);
	};
}