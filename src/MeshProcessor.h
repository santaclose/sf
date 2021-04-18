#pragma once

#include <Mesh.h>
#include <VoxelModel.h>

namespace sf {

	class MeshProcessor {
	public:
		static void ComputeNormals(Mesh& model, bool normalize = false);
		static void ComputeTangentSpace(Mesh& model);

		static void BakeAoToVertices(Mesh& model);
	};
}