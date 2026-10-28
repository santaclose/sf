#pragma once

#include <MeshProcessor.h>
#include <vector>

namespace sf
{
	struct LODSpecItem
	{
		uint32_t remeshRes = 0; /* 0 for no remesh, greater than 4 for remesh */
		float ratio;
		float error = 1e-2f;
		float maxDistance;
	};

	struct LODSpec
	{
		std::vector<LODSpecItem> items;
		inline uint32_t Count() { return items.size(); }
		inline void Clear() { items.clear(); }
		inline void Push(const LODSpecItem& lsi) { items.push_back(lsi); }
		bool SaveToFile(const char* targetFile);
		bool LoadFromFile(const char* targetFile);
	};

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
		void Create(const MeshData& meshData, const LODSpec& lodSpec);
		void Destroy();
	};
}