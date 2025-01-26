#pragma once

#include <Components/Transform.h>

namespace sf
{
	struct CapsuleCollider
	{
		float radius = 0.5f;
		glm::vec3 centerA = glm::vec3(0.0f, -0.5f, 0.0f);
		glm::vec3 centerB = glm::vec3(0.0f, 0.5f, 0.0f);
		inline CapsuleCollider ApplyTransform(const Transform& transform)
		{
			CapsuleCollider out;
			out.radius = this->radius * transform.scale;
			out.centerA = this->centerA * transform.scale;
			out.centerB = this->centerB * transform.scale;
			out.centerA = transform.rotation * out.centerA;
			out.centerB = transform.rotation * out.centerB;
			out.centerA += transform.position;
			out.centerB += transform.position;
			return out;
		}
	};
}