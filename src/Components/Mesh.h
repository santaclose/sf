#pragma once

#include <MeshData.h>
#include <vector>

namespace sf {
	
	struct Mesh
	{
		const MeshData* meshData = nullptr;
		std::vector<uint32_t> materials;

		inline Mesh(const MeshData* meshData)
		{
			this->meshData = meshData;
			this->materials.clear();
			this->materials.resize(meshData->pieceCount, ~0U);
		}
		inline Mesh(const MeshData* meshData, uint32_t material)
		{
			this->meshData = meshData;
			this->materials.clear();
			this->materials.resize(meshData->pieceCount, material);
		}
	};
}