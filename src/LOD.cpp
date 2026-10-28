#include "LOD.h"

#include <fstream>
#include <sstream>

bool sf::LODSpec::SaveToFile(const char* targetFile)
{
	std::ofstream file;
	file.open(targetFile, std::ios::trunc);
	if (!file)
		return false;

	for (const LODSpecItem& lsi : items)
	{
		file << std::to_string(lsi.remeshRes) << " ";
		file << std::to_string(lsi.ratio) << " ";
		file << std::to_string(lsi.error) << " ";
		file << std::to_string(lsi.maxDistance) << "\n";
	}

	file.close();
	return true;
}

bool sf::LODSpec::LoadFromFile(const char* targetFile)
{
	std::ifstream file;
	file.open(targetFile);
	if (!file)
		return false;

	items.clear();
	std::string line;
	while (std::getline(file, line))
	{
		if (line.length() == 0)
			continue;
		items.emplace_back();
		std::stringstream ss(line);
		ss >> items.back().remeshRes;
		ss >> items.back().ratio;
		ss >> items.back().error;
		ss >> items.back().maxDistance;
	}

	file.close();
	return true;
}

const sf::MeshData* sf::LODGroup::GetMeshDataForLOD(uint32_t lod) const
{
	assert(lod < lods.size());
	if (lods[lod].meshData == nullptr)
		return sourceMesh;
	return lods[lod].meshData;
}

void sf::LODGroup::Create(const MeshData& meshData, const LODSpec& lodSpec)
{
	Destroy();

	printf("[LOD] Generating from mesh with %u primitives\n", meshData.indexCount / 3);
	sourceMesh = &meshData;

	uint32_t lodCount = lodSpec.items.size();
	assert(lodCount > 0);

	/* +1 for culling, new will initialize the data to 0s */
	lodMeshes = new MeshData[lodCount + 1];
	lods.resize(lodCount + 1);
	lods[lodCount] = { &lodMeshes[lodCount], 0.0f };

	for (uint32_t i = 0; i < lodCount; i++)
	{
		const LODSpecItem& lsi = lodSpec.items[i];
		assert(lsi.ratio > 0.0f && lsi.ratio <= 1.0f);

		if (lsi.remeshRes < 4)
		{
			if (lsi.ratio == 1.0f)
			{
				lods[i] = { nullptr, lsi.maxDistance };
			}
			else
			{
				MeshProcessor::Decimate(meshData, lodMeshes[i], lsi.ratio, lsi.error);
				lods[i] = { &lodMeshes[i], lsi.maxDistance };
			}
		}
		else
		{
			MeshData temp;
			MeshProcessor::Remesh(meshData, temp, lsi.remeshRes);
			temp.ChangeVertexBufferLayout(meshData.vertexBufferLayout);
			if (lsi.ratio == 1.0f)
				lodMeshes[i] = temp;
			else
			{
				MeshProcessor::Decimate(temp, lodMeshes[i], lsi.ratio, lsi.error);
				/* vertex buffer starts belonging to lodMeshes[i] */
				lodMeshes[i].usesExternalVertexBuffer = false;
			}

			MeshProcessor::TransferVertexAttributesFromClosest(meshData, lodMeshes[i]);

			lods[i] = { &lodMeshes[i], lsi.maxDistance };
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