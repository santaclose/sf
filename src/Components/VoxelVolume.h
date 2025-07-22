#pragma once

#include <VoxelVolumeData.h>

namespace sf {

	struct VoxelVolume
	{
		const VoxelVolumeData* voxelVolumeData;
		VoxelVolume(const VoxelVolumeData* vv);
	};
}