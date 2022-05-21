#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <Components/Mesh.h>

namespace sf {

	struct VoxelBox
	{
		float voxelSize;
		glm::vec3 offset;
		std::vector<std::vector<std::vector<bool>>> mat;
		VoxelBox(unsigned int voxelCountX, unsigned int voxelCountY, unsigned int voxelCountZ, float voxelSize = 1.0f, const glm::vec3& offset = { 0.0f, 0.0f, 0.0f });
		VoxelBox(const Mesh& mesh, float voxelSize);

		bool CastRay(const glm::vec3& origin, const glm::vec3& direction, bool avoidEarlyCollision = true, float* out_t = nullptr, bool draw = false);

		inline glm::vec3 GetAABBMin() const
		{
			return offset;
		}
		inline glm::vec3 GetAABBMax() const
		{
			assert(mat.size() > 0 && mat[0].size() > 0 && mat[0][0].size() > 0);
			return offset + glm::vec3(mat.size() * voxelSize, mat[0].size() * voxelSize, mat[0][0].size() * voxelSize);
		}
		inline glm::vec3 GetVoxelCenterLocation(glm::uvec3 coords) const
		{
			return glm::vec3(
				offset.x + (voxelSize * (float)coords.x) + voxelSize / 2.0f,
				offset.y + (voxelSize * (float)coords.y) + voxelSize / 2.0f,
				offset.z + (voxelSize * (float)coords.z) + voxelSize / 2.0f);
		}
	};
}