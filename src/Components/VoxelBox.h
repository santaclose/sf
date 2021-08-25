#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace sf {

	struct VoxelBox
	{
		std::vector<std::vector<std::vector<bool>>> mat;
		VoxelBox(unsigned int sizeX, unsigned int sizeY, unsigned int sizeZ);
	};
}