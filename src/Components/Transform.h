#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

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

		static inline Transform FromMatrix(const glm::mat4& matrix)
		{
			glm::vec3 scale, translation, skew;
			glm::quat rotation;
			glm::vec4 perspective;
			bool succeeded = glm::decompose(matrix, scale, rotation, translation, skew, perspective);
			assert(succeeded);
			const float assertEpsilon = 0.001f;
			assert(glm::abs(skew.x) < assertEpsilon &&
				glm::abs(skew.y) < assertEpsilon &&
				glm::abs(skew.z) < assertEpsilon); // we don't deal with skew
			assert(glm::abs(perspective.x) < assertEpsilon &&
				glm::abs(perspective.y) < assertEpsilon &&
				glm::abs(perspective.z) < assertEpsilon &&
				glm::abs(perspective.w - 1.0f) < assertEpsilon); // we don't deal with perspective
			Transform output;
			output.position = translation;
			output.rotation = glm::normalize(rotation);
			output.SetScale(scale);
			return output;
		}

		inline void SetScale(const glm::vec3& newScale)
		{
			const float assertEpsilon = 0.001f;
			assert(glm::abs(newScale.x - newScale.y) < assertEpsilon &&
				glm::abs(newScale.x - newScale.z) < assertEpsilon); // we don't deal with non uniform scaaling
			scale = newScale.x;
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

		inline void Apply(const Transform& other)
		{
			position += rotation * (other.position * scale);
			rotation *= other.rotation;
			scale *= other.scale;
		}

		inline Transform Inverse()
		{
			Transform out;
			out.scale = 1.0f / scale;
			out.rotation = glm::conjugate(rotation);
			out.position = out.rotation * (-position * out.scale);
			return out;
		}
	};
}