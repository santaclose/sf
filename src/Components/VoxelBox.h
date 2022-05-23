#pragma once

#include <VoxelBoxData.h>

namespace sf {
	
	struct VoxelBox
	{
	private:
		static int counter;

	public:
		int id;
		const VoxelBoxData* voxelBoxData;

		VoxelBox(const VoxelBoxData* VoxelBoxData);
	};
}