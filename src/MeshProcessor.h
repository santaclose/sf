#pragma once

#include <MeshData.h>
#include <SkeletonData.h>
#include <VoxelVolumeData.h>
#include <string>

namespace sf {

	struct VertexAmbientOcclusionBakerConfig
	{
		int rayCount = 500;
		bool onlyCastRaysUpwards = true;
		float rayOriginOffset = 0.001f;
		float denoiseWeight = 0.2f;
		int denoisePasses = 2;
		bool autoConfigure = true;
			float rayDistance = 5.0f;
			float falloff = 6.0f;
	};

	class MeshProcessor {
	private:
		static float ComputeOcclusion(const std::vector<std::pair<bool, float>>& rayResults, float maxDistance, float falloff);
	public:
		static void ComputeNormals(MeshData& mesh, bool normalize = false);
		static void ComputeTangentSpace(MeshData& mesh);
		static void ComputeVertexAmbientOcclusion(MeshData& mesh, const VoxelVolumeData* voxelVolume = nullptr, const VertexAmbientOcclusionBakerConfig* config = nullptr);
		static void GenerateGrid(MeshData& mesh, uint32_t sizeX, uint32_t sizeY, uint32_t texResX, uint32_t texResY, float cellSize, bool useQuads = false);
		static void RemoveUnusedBones(MeshData& mesh, SkeletonData& skeleton);
	};
}