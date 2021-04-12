#pragma once

#include <Model.h>
#include <VoxelModel.h>

namespace sf {

	class ModelProcessor {
	public:
		static void ComputeNormals(Model& model, bool normalize = false);
		static void ComputeTangentSpace(Model& model);

		static void BakeAoToVertices(Model& model);
	};
}