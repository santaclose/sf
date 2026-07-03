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

	inline std::vector<LOD> GenerateLODs(const MeshData& meshData, float* ratios, float* maxDistances, uint32_t lodCount)
	{
		printf("[LOD] Generating from mesh with %u indices\n", meshData.indexCount / 3);

		assert(lodCount > 0);

		/* +1 for culling */
		MeshData* lodMeshes = new MeshData[lodCount + 1];
		lodMeshes[lodCount].vertexCount = 0;
		lodMeshes[lodCount].indexCount = 0;
		std::vector<LOD> output;
		output.resize(lodCount + 1);
		output[lodCount] = { &lodMeshes[lodCount], 0.0f };

		for (uint32_t i = 0; i < lodCount; i++)
		{
			assert(ratios[i] > 0.0f);
			if (ratios[i] == 1.0f)
			{
				output[i] = { &meshData, maxDistances[i] };
			}
			else
			{
				MeshProcessor::Decimate(meshData, lodMeshes[i], ratios[i]);
				output[i] = { &lodMeshes[i], maxDistances[i] };
			}
		}

#ifdef SF_DEBUG
		for (uint32_t i = 0; i < lodCount + 1; i++)
			printf("[LOD] LOD %u: %u faces, %f max distance\n", i, output[i].meshData->indexCount / 3, output[i].maxDistance);
#endif
		return output;
	}
}