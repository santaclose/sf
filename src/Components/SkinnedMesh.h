#pragma once

#include <Material.h>
#include <LOD.h>
#include <SkeletonData.h>
#include <vector>

namespace sf {

	struct SkinnedMesh
	{
		const LODGroup* lodGroup = nullptr;
		const SkeletonData* skeletonData = nullptr;
		std::vector<const Material*> materials;
		bool usesLodGroup;

		inline SkinnedMesh(const LODGroup* lodGroup, const SkeletonData* skeletonData, const Material* material = nullptr)
		{
			assert(lodGroup != nullptr);
			assert(lodGroup->lods.size() > 0);
			this->lodGroup = lodGroup;
			this->materials.clear();
			this->materials.resize(lodGroup->sourceMesh->pieceCount, material);
			this->skeletonData = skeletonData;
			usesLodGroup = true;
		}
		inline SkinnedMesh(const MeshData* meshData, const SkeletonData* skeletonData, const Material* material = nullptr)
		{
			/* Use the lodGroup pointer just as storage */
			this->lodGroup = (LODGroup*) meshData;
			this->materials.clear();
			this->materials.resize(meshData->pieceCount, material);
			this->skeletonData = skeletonData;
			usesLodGroup = false;
		}

		inline const MeshData* GetSourceMeshData() const
		{
			if (!usesLodGroup)
				return (const MeshData*) lodGroup;
			return lodGroup->sourceMesh;
		}

		inline const MeshData* GetMeshData(float camDistance2) const
		{
			if (!usesLodGroup)
				return (const MeshData*) lodGroup;

			for (uint32_t i = 0; i < lodGroup->GetLODCount(); i++)
			{
				const LOD& lod = lodGroup->lods[i];
				if (camDistance2 < lod.maxDistance * lod.maxDistance)
					return lodGroup->GetMeshDataForLOD(i);
			}
			return lodGroup->GetMeshDataForLOD(lodGroup->GetLODCount() - 1);
		}

		inline uint32_t GetCurrentLOD(float camDistance2) const
		{
			assert(usesLodGroup);
			for (uint32_t i = 0; i < lodGroup->GetLODCount(); i++)
			{
				const LOD& lod = lodGroup->lods[i];
				if (camDistance2 < lod.maxDistance * lod.maxDistance)
					return i;
			}
			return lodGroup->GetLODCount() - 1;
		}
	};
}