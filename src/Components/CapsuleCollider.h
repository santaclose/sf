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
			out.centerA = transform.ApplyToPoint(centerA);
			out.centerB = transform.ApplyToPoint(centerB);
			return out;
		}
	};
}