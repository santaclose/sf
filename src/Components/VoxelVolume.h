#pragma once

#include <VoxelVolumeData.h>

namespace sf {

	struct VoxelVolume
	{
		const VoxelVolumeData* voxelVolumeData;
		uint32_t material;

		inline VoxelVolume(const VoxelVolumeData* vv)
		{
			this->voxelVolumeData = vv;
			this->material = ~0U;
		}

		inline VoxelVolume(const VoxelVolumeData* vv, uint32_t material)
		{
			this->voxelVolumeData = vv;
			this->material = material;
		}
	};
}