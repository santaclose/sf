#pragma once

#include <Components/Transform.h>

namespace sf
{
	struct BoxCollider
	{
		glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		inline BoxCollider ApplyTransform(const Transform& transform)
		{
			BoxCollider out;
			out.size = this->size * transform.scale;
			out.center = this->center * transform.scale;
			out.center = transform.rotation * out.center;
			out.orientation = transform.rotation * this->orientation;
			out.center += transform.position;
			return out;
		}
		inline BoxCollider ApplyTransformInverse(const Transform& transform)
		{
			BoxCollider out;
			glm::quat conj = glm::conjugate(transform.rotation);
			out.center = this->center - transform.position;
			out.center = conj * out.center;
			out.center = out.center / transform.scale;
			out.orientation = conj * this->orientation;
			out.size = this->size / transform.scale;
			return out;
		}
	};
}