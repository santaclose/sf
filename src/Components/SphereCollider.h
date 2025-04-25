#pragma once

#include <Components/Transform.h>

namespace sf
{
	struct SphereCollider
	{
		float radius = 0.5f;
		glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
		inline SphereCollider ApplyTransform(const Transform& transform)
		{
			SphereCollider out;
			out.radius = this->radius * transform.scale;
			out.center = this->center * transform.scale;
			out.center = transform.rotation * out.center;
			out.center += transform.position;
			return out;
		}
		inline SphereCollider ApplyTransformInverse(const Transform& transform)
		{
			SphereCollider out;
			out.center = this->center - transform.position;
			out.center = glm::conjugate(transform.rotation) * out.center;
			out.center = out.center / transform.scale;
			out.radius = this->radius / transform.scale;
			return out;
		}
	};
}