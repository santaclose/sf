#pragma once

#include <MeshProcessor.h>
#include <vector>

namespace sf
{
	struct LOD
	{
		/* Source mesh is used when meshData is nullptr */
		MeshData* meshData = nullptr;
		float maxDistance;
	};

	struct LODGroup
	{
		const MeshData* sourceMesh = nullptr;
		std::vector<LOD> lods;
		MeshData* lodMeshes = nullptr;

		inline uint32_t GetLODCount() const
		{
			return lods.size();
		}

		const MeshData* GetMeshDataForLOD(uint32_t lod) const;
		void Create(const MeshData& meshData, float* ratios, float* maxDistances, uint32_t lodCount, uint32_t* remeshRes = nullptr);
		void Destroy();
	};
}