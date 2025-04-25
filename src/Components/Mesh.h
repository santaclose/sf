#pragma once

#include <MeshData.h>
#include <vector>

namespace sf {
	
	struct Mesh
	{
		const MeshData* meshData;
		std::vector<uint32_t> materials;

		Mesh(const MeshData* meshData);
		Mesh(const MeshData* meshData, uint32_t material);
	};
}