#pragma once

#include <glm/glm.hpp>
#include <unordered_set>
#include <MeshData.h>
#include <Hash.h>

#include <cstring>

namespace sf {

	struct VoxelVolumeData
	{
		float voxelSize;
		glm::vec3 offset; // from center to min corner
		glm::uvec3 voxelCountPerAxis;

		std::unordered_map<glm::uvec3, uint32_t, Hash::UVec3Hash> map;
		std::vector<uint8_t> perVoxelData;
		DataLayout voxelLayout;

		VoxelVolumeData() = default;
		~VoxelVolumeData() = default;

		void BuildEmpty(const glm::uvec3& voxelCountPerAxis, const DataLayout& voxelLayout, float voxelSize = 1.0f, const glm::vec3& offset = { 0.0f, 0.0f, 0.0f });
		void BuildFromMesh(const MeshData& meshData, float voxelSize, const DataLayout& voxelLayout);

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
			if (map.find(coords) == map.end())
				return nullptr;

			return (void*)&perVoxelData[voxelLayout.GetSize() * map.at(coords)];
		}
		inline void SetVoxel(glm::uvec3 coords, void* value)
		{
			if (map.find(coords) == map.end())
			{
				map[coords] = perVoxelData.size() / voxelLayout.GetSize();
				perVoxelData.resize(perVoxelData.size() + voxelLayout.GetSize());
			}
			memcpy(&perVoxelData[voxelLayout.GetSize() * map[coords]], value, voxelLayout.GetSize());
		}
		inline uint32_t GetVoxelCount() const
		{
			assert(perVoxelData.size() % voxelLayout.GetSize() == 0);
			assert(map.size() == (uint32_t) perVoxelData.size() / voxelLayout.GetSize());
			return map.size();
		}
	};
}