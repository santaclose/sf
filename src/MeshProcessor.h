#pragma once

#include <MeshData.h>
#include <string>

namespace sf {

	class MeshProcessor {
	public:
		static void ComputeNormals(MeshData& mesh, bool normalize = false);
		static void ComputeTangentSpace(MeshData& mesh);

		static void BakeAoToVertices(MeshData& mesh);

		static void GenerateMeshWithFunction(MeshData& mesh, void (*functionPointer)());
	};
}