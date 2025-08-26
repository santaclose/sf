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
		std::vector<uint8_t> voxelBuffer;
		BufferLayout voxelBufferLayout;

		VoxelVolumeData() = default;
		~VoxelVolumeData() = default;

		void BuildEmpty(const glm::uvec3& voxelCountPerAxis, const BufferLayout* voxelBufferLayout = nullptr, float voxelSize = 1.0f, const glm::vec3& offset = { 0.0f, 0.0f, 0.0f });
		void BuildFromMesh(const MeshData& meshData, float voxelSize, const BufferLayout* voxelBufferLayout = nullptr);

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
		template <typename T>
		inline T* AccessVoxelComponent(BufferComponent component, const glm::uvec3& coords) const
		{
			if (map.find(coords) == map.end())
				return nullptr;
			return voxelBufferLayout.Access<T>((void*)voxelBuffer.data(), component, map.at(coords));
		}
		inline void* GetVoxel(glm::uvec3 coords) const
		{
			if (map.find(coords) == map.end())
				return nullptr;

			if (voxelBufferLayout.GetSize() == 0)
				return (void*) true;
			return (void*)&voxelBuffer[voxelBufferLayout.GetSize() * map.at(coords)];
		}
		inline bool CreateVoxel(glm::uvec3 coords)
		{
			if (map.find(coords) != map.end())
				return false;

			if (voxelBufferLayout.GetSize() == 0)
			{
				map[coords] = map.size();
				return true;
			}

			map[coords] = voxelBuffer.size() / voxelBufferLayout.GetSize();
			voxelBuffer.resize(voxelBuffer.size() + voxelBufferLayout.GetSize(), 0);
			return true;
		}

		inline uint32_t GetVoxelCount() const
		{
			assert(voxelBuffer.size() % voxelBufferLayout.GetSize() == 0);
			assert(map.size() == (uint32_t) voxelBuffer.size() / voxelBufferLayout.GetSize());
			return map.size();
		}
	};
}