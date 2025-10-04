#pragma once

#include <Material.h>
#include <MeshData.h>
#include <SkeletonData.h>
#include <vector>

namespace sf {

	struct SkinnedMesh
	{
		const MeshData* meshData = nullptr;
		const SkeletonData* skeletonData = nullptr;
		std::vector<const Material*> materials;

		inline SkinnedMesh(const MeshData* meshData, const SkeletonData* skeletonData)
		{
			this->meshData = meshData;
			this->skeletonData = skeletonData;
			this->materials.resize(meshData->pieceCount, nullptr);
		}
		inline SkinnedMesh(const MeshData* meshData, const Material* material, const SkeletonData* skeletonData)
		{
			this->meshData = meshData;
			this->skeletonData = skeletonData;
			this->materials.resize(meshData->pieceCount, material);
		}
	};
}