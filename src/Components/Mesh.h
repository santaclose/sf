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
			/* If material uses task/mesh shaders, no need to have mesh data */
			this->materials.resize(meshData == nullptr ? 1 : meshData->pieceCount, material);
		}
	};
}