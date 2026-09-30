#pragma once

#include <Material.h>
#include <LOD.h>
#include <vector>

namespace sf {
	
	struct Mesh
	{
		const LODGroup* lodGroup = nullptr;
		std::vector<const Material*> materials;
		bool usesLodGroup;

		inline Mesh(const LODGroup* lodGroup, const Material* material = nullptr)
		{
			assert(lodGroup != nullptr);
			assert(lodGroup->lods.size() > 0);
			this->lodGroup = lodGroup;
			/* If material uses task/mesh shaders, no need to have mesh data */
			this->materials.clear();
			this->materials.resize(lodGroup->sourceMesh->pieceCount, material);
			usesLodGroup = true;
		}
		inline Mesh(const MeshData* meshData, const Material* material = nullptr)
		{
			/* Use the lodGroup pointer just as storage */
			this->lodGroup = (LODGroup*) meshData;
			/* If material uses task/mesh shaders, no need to have mesh data */
			this->materials.clear();
			this->materials.resize(meshData == nullptr ? 1 : meshData->pieceCount, material);
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

			for (const LOD& lod : lodGroup->lods)
			{
				if (camDistance2 < lod.maxDistance * lod.maxDistance)
					return lod.meshData;
			}
			return lodGroup->lods[lodGroup->lods.size() - 1].meshData;
		}

		inline uint32_t GetCurrentLOD(float camDistance2) const
		{
			assert(usesLodGroup);
			uint32_t out = 0;
			for (const LOD& lod : lodGroup->lods)
			{
				if (camDistance2 < lod.maxDistance * lod.maxDistance)
					return out;
				out++;
			}
			return uint32_t(lodGroup->lods.size() - 1);
		}
	};
}