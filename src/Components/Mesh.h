#pragma once

#include <Material.h>
#include <LOD.h>
#include <vector>

namespace sf {
	
	struct Mesh
	{
		std::vector<LOD> lods;
		std::vector<const Material*> materials;

		inline Mesh(const std::vector<LOD>& lods, const Material* material = nullptr)
		{
			assert(lods.size() > 0);
			this->lods = lods;
			/* If material uses task/mesh shaders, no need to have mesh data */
			this->materials.clear();
			this->materials.resize(lods[0].meshData->pieceCount, material);
		}
		inline Mesh(const MeshData* meshData, const Material* material = nullptr)
		{
			this->lods.resize(1);
			this->lods[0].meshData = meshData;
			/* If material uses task/mesh shaders, no need to have mesh data */
			this->materials.clear();
			this->materials.resize(meshData == nullptr ? 1 : meshData->pieceCount, material);
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