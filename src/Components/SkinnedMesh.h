#pragma once

#include <MeshData.h>
#include <SkeletonData.h>
#include <vector>

namespace sf {

	struct SkinnedMesh
	{
		const MeshData* meshData = nullptr;
		const SkeletonData* skeletonData = nullptr;
		std::vector<uint32_t> materials;

		inline SkinnedMesh(const MeshData* meshData, const SkeletonData* skeletonData)
		{
			this->meshData = meshData;
			this->skeletonData = skeletonData;
			this->materials.resize(meshData->pieces.size(), ~0U);
		}
		inline SkinnedMesh(const MeshData* meshData, uint32_t material, const SkeletonData* skeletonData)
		{
			this->meshData = meshData;
			this->skeletonData = skeletonData;
			this->materials.resize(meshData->pieces.size(), material);
		}
	};
}