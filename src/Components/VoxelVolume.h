#pragma once

#include <Material.h>
#include <VoxelVolumeData.h>

namespace sf {

	struct VoxelVolume
	{
		const VoxelVolumeData* voxelVolumeData;
		const Material* material;

		inline VoxelVolume(const VoxelVolumeData* vv)
		{
			this->voxelVolumeData = vv;
			this->material = nullptr;
		}

		inline VoxelVolume(const VoxelVolumeData* vv, const Material* material)
		{
			this->voxelVolumeData = vv;
			this->material = material;
		}
	};
}