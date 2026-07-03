#pragma once

#include <Material.h>
#include <LOD.h>
#include <SkeletonData.h>
#include <vector>

namespace sf {

	struct SkinnedMesh
	{
		std::vector<LOD> lods;
		const SkeletonData* skeletonData = nullptr;
		std::vector<const Material*> materials;

		inline SkinnedMesh(const std::vector<LOD>& lods, const SkeletonData* skeletonData, const Material* material = nullptr)
		{
			assert(lods.size() > 0);
			this->lods = lods;
			this->materials.clear();
			this->materials.resize(lods[0].meshData->pieceCount, material);
			this->skeletonData = skeletonData;
		}
		inline SkinnedMesh(const MeshData* meshData, const SkeletonData* skeletonData, const Material* material = nullptr)
		{
			this->lods.resize(1);
			this->lods[0].meshData = meshData;
			this->materials.clear();
			this->materials.resize(meshData->pieceCount, material);
			this->skeletonData = skeletonData;
		}

		inline const MeshData* GetMeshData(float camDistance2)
		{
			for (LOD& lod : lods)
			{
				if (camDistance2 < lod.maxDistance * lod.maxDistance)
					return lod.meshData;
			}
			return lods[lods.size() - 1].meshData;
		}

		inline uint32_t GetCurrentLOD(float camDistance2)
		{
			uint32_t out = 0;
			for (LOD& lod : lods)
			{
				if (camDistance2 < lod.maxDistance * lod.maxDistance)
					return out;
				out++;
			}
			return uint32_t(lods.size() - 1);
		}
	};
}