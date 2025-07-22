#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <MeshData.h>

namespace sf {

	struct VoxelVolumeData
	{
		float voxelSize;
		glm::vec3 offset; // from center to min corner
		std::vector<void*> mat;
		glm::uvec3 voxelCountPerAxis;

		VoxelVolumeData() = default;
		~VoxelVolumeData() = default;

		void BuildEmpty(const glm::uvec3& voxelCountPerAxis, float voxelSize = 1.0f, const glm::vec3& offset = { 0.0f, 0.0f, 0.0f });
		void BuildFromMesh(const MeshData& meshData, float voxelSize);
		void* CastRay(const glm::vec3& origin, const glm::vec3& direction, bool avoidEarlyCollision = true, float* out_t = nullptr) const;

		inline glm::vec3 GetAABBMin() const
		{
			return offset;
		}
		inline glm::vec3 GetAABBMax() const
		{
			assert(voxelCountPerAxis.x > 0 && voxelCountPerAxis.y > 0 && voxelCountPerAxis.z > 0);
			return offset + glm::vec3(voxelCountPerAxis.x * voxelSize, voxelCountPerAxis.y * voxelSize, voxelCountPerAxis.z * voxelSize);
		}
		inline glm::vec3 GetVoxelCenterLocation(glm::uvec3 coords) const
		{
			return glm::vec3(
				offset.x + (voxelSize * (float)coords.x) + voxelSize / 2.0f,
				offset.y + (voxelSize * (float)coords.y) + voxelSize / 2.0f,
				offset.z + (voxelSize * (float)coords.z) + voxelSize / 2.0f);
		}
		inline void* GetVoxel(glm::uvec3 coords) const
		{
			return mat[coords.x * voxelCountPerAxis.y * voxelCountPerAxis.z + coords.y * voxelCountPerAxis.z + coords.z];
		}
		inline void SetVoxel(glm::uvec3 coords, void* value)
		{
			mat[coords.x * voxelCountPerAxis.y * voxelCountPerAxis.z + coords.y * voxelCountPerAxis.z + coords.z] = value;
		}
	};
}