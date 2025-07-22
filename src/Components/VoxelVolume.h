#pragma once

#include <VoxelVolumeData.h>

namespace sf {

	struct VoxelVolume
	{
		const VoxelVolumeData* voxelVolumeData;
		inline VoxelVolume(const VoxelVolumeData* vv)
		{
			this->voxelVolumeData = vv;
		}
	};
}