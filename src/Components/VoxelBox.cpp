#include "VoxelBox.h"

sf::VoxelBox::VoxelBox(unsigned int sizeX, unsigned int sizeY, unsigned int sizeZ)
{
	mat.resize(sizeX);
	for (auto& vec : mat)
	{
		vec.resize(sizeY);
		for (auto& deeperVec : vec)
			deeperVec.resize(sizeZ);
	}
}
