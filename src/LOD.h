#pragma once

#include <MeshProcessor.h>
#include <vector>

namespace sf
{
	struct LOD
	{
		const MeshData* meshData = nullptr;
		float maxDistance;
	};

	struct LODGroup
	{
		const MeshData* sourceMesh = nullptr;
		std::vector<LOD> lods;
		MeshData* lodMeshes = nullptr;
		/* Index buffer memory used by LOD meshes is managed by this class */

		void Create(const MeshData& meshData, float* ratios, float* maxDistances, uint32_t lodCount)
		{
			Destroy();

			printf("[LOD] Generating from mesh with %u primitives\n", meshData.indexCount / 3);
			sourceMesh = &meshData;

			assert(lodCount > 0);

			/* +1 for culling, new will initialize the data to 0s */
			lodMeshes = new MeshData[lodCount + 1];
			lods.resize(lodCount + 1);
			lods[lodCount] = { &lodMeshes[lodCount], 0.0f };

			for (uint32_t i = 0; i < lodCount; i++)
			{
				assert(ratios[i] > 0.0f);
				if (ratios[i] == 1.0f)
				{
					lods[i] = { &meshData, maxDistances[i] };
				}
				else
				{
					MeshProcessor::Decimate(meshData, lodMeshes[i], ratios[i]);
					lods[i] = { &lodMeshes[i], maxDistances[i] };
				}
			}

	#ifdef SF_DEBUG
			for (uint32_t i = 0; i < lodCount + 1; i++)
				printf("[LOD] LOD %u: %u primitives, %f max distance\n", i, lods[i].meshData->indexCount / 3, lods[i].maxDistance);
	#endif
		}

		void Destroy()
		{
			for (uint32_t i = 0; i < lods.size(); i++)
			{
				const LOD& lod = lods[i];
				if (lod.meshData == sourceMesh)
					continue;
				if (lod.meshData->indexCount == 0)
					continue;
				delete[] lod.meshData->indexBuffer;
			}
			if (lodMeshes != nullptr)
				delete[] lodMeshes;

			lodMeshes = nullptr;
			sourceMesh = nullptr;
			lods.clear();
		}
	};
}