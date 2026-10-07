#include "LOD.h"

const sf::MeshData* sf::LODGroup::GetMeshDataForLOD(uint32_t lod) const
{
	assert(lod < lods.size());
	if (lods[lod].meshData == nullptr)
		return sourceMesh;
	return lods[lod].meshData;
}

void sf::LODGroup::Create(const MeshData& meshData, float* ratios, float* maxDistances, uint32_t lodCount, uint32_t* remeshRes)
{
	static Buffer::Layout postionNormal = Buffer::Layout({Buffer::Component::Position, Buffer::Component::Normal});
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
		if (remeshRes == nullptr || remeshRes[i] < 4)
		{
			if (ratios[i] == 1.0f)
			{
				lods[i] = { nullptr, maxDistances[i] };
			}
			else
			{
				MeshProcessor::Decimate(meshData, lodMeshes[i], ratios[i]);
				lods[i] = { &lodMeshes[i], maxDistances[i] };
			}
		}
		else
		{
			MeshData temp;
			MeshProcessor::Remesh(meshData, temp, remeshRes[i]);
			temp.ChangeVertexBufferLayout(postionNormal);
			MeshProcessor::ComputeNormals(temp);
			if (ratios[i] == 1.0f)
				lodMeshes[i] = temp;
			else
			{
				MeshProcessor::Decimate(temp, lodMeshes[i], ratios[i]);
				/* vertex buffer starts belonging to lodMeshes[i] */
				lodMeshes[i].usesExternalVertexBuffer = false;
			}

			lods[i] = { &lodMeshes[i], maxDistances[i] };
		}
	}

#ifdef SF_DEBUG
	for (uint32_t i = 0; i < lodCount + 1; i++)
		printf("[LOD] LOD %u: %u primitives, %f max distance\n", i, GetMeshDataForLOD(i)->indexCount / 3, lods[i].maxDistance);
#endif
}

void sf::LODGroup::Destroy()
{
	for (uint32_t i = 0; i < lods.size(); i++)
	{
		const LOD& lod = lods[i];
		if (lod.meshData != nullptr)
			lod.meshData->Destroy();
	}
	if (lodMeshes != nullptr)
		delete[] lodMeshes;

	lodMeshes = nullptr;
	sourceMesh = nullptr;
	lods.clear();
}