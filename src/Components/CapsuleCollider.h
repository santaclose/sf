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
		inline CapsuleCollider ApplyTransformInverse(const Transform& transform)
		{
			CapsuleCollider out;
			glm::quat conj = glm::conjugate(transform.rotation);
			out.centerA = this->centerA - transform.position;
			out.centerB = this->centerB - transform.position;
			out.centerA = conj * out.centerA;
			out.centerB = conj * out.centerB;
			out.centerA = out.centerA / transform.scale;
			out.centerB = out.centerB / transform.scale;
			out.radius = this->radius / transform.scale;
			return out;
		}
	};
}

#define GET_WORLD_SPACE_CAPSULE_COLLIDER(x) (x.GetComponent<CapsuleCollider>().ApplyTransform(x.GetComponent<Transform>()))