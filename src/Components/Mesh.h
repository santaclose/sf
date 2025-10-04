#pragma once

#include <Material.h>
#include <MeshData.h>
#include <vector>

namespace sf {
	
	struct Mesh
	{
		const MeshData* meshData = nullptr;
		std::vector<const Material*> materials;

		inline Mesh(const MeshData* meshData)
		{
			this->meshData = meshData;
			this->materials.clear();
			this->materials.resize(meshData->pieceCount, nullptr);
		}
		inline Mesh(const MeshData* meshData, const Material* material)
		{
			this->meshData = meshData;
			this->materials.clear();
			this->materials.resize(meshData->pieceCount, material);
		}
	};
}