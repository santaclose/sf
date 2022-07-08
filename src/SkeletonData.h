#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace sf
{
	struct Bone
	{
		int32_t parent;
		glm::mat4 localMatrix;
		glm::mat4 invModelMatrix;
	};

	struct SkeletonData
	{
		std::vector<Bone> bones;
	};
}