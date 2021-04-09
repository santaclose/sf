#pragma once

#include <Model.h>
#include <VoxelModel.h>

namespace sf {

	class ModelProcessor {
		static float ComputeOcclusion(const std::vector<std::pair<bool, float>>& rayResults, float maxDistance, float falloff);

	public:
		static void ComputeNormals(Model& model, bool normalize = false);
		static void ComputeTangentSpace(Model& model);

		static void BakeAoToVertices(Model& model,
			int rayCount = 30, bool onlyCastRaysUpwards = false,
			const VoxelModel* voxelized = nullptr,
			float rayOriginOffset = 0.001f,
			float rayDistance = 5.0f, float falloff = 6.0f,
			float denoiseWeight = 0.3f, int denoisePasses = 1);
	};
}