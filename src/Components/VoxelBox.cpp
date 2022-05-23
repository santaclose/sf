#include "VoxelBox.h"

int sf::VoxelBox::counter = 0;

sf::VoxelBox::VoxelBox(const VoxelBoxData* voxelBoxData)
{
	this->voxelBoxData = voxelBoxData;
	this->id = counter;
	counter++;
}
