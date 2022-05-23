#pragma once

#include <MeshData.h>
#include <string>

namespace sf {

	class MeshProcessor {
	public:
		static void ComputeNormals(MeshData& model, bool normalize = false);
		static void ComputeTangentSpace(MeshData& model);

		static void BakeAoToVertices(MeshData& model);
	};
}