#pragma once

#include <MeshData.h>

namespace sf {
	
	struct Mesh
	{
	private:
		static int counter;

	public:
		int id;
		const MeshData* meshData;

		Mesh(const MeshData* meshData);
	};
}