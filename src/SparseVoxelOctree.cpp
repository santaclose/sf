#include "SparseVoxelOctree.h"

#include <Math.hpp>

uint32_t sf::SparseVoxelOctree::CreateFromVoxelVolumeDataRec(const VoxelVolumeData& vvd, uint32_t currentDepth, const glm::uvec3& currentCorner)
{
	uint32_t voxelsPerUnit = 1u << (depth - currentDepth);
	if (voxelsPerUnit == 2u)
	{
		/* Base case */
		if ((vvd.GetVoxel(currentCorner + glm::uvec3(0u, 0u, 0u)) != nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(1u, 0u, 0u)) != nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(0u, 1u, 0u)) != nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(1u, 1u, 0u)) != nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(0u, 0u, 1u)) != nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(1u, 0u, 1u)) != nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(0u, 1u, 1u)) != nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(1u, 1u, 1u)) != nullptr))
			return 0xffffffff;
		if ((vvd.GetVoxel(currentCorner + glm::uvec3(0u, 0u, 0u)) == nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(1u, 0u, 0u)) == nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(0u, 1u, 0u)) == nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(1u, 1u, 0u)) == nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(0u, 0u, 1u)) == nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(1u, 0u, 1u)) == nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(0u, 1u, 1u)) == nullptr) &&
			(vvd.GetVoxel(currentCorner + glm::uvec3(1u, 1u, 1u)) == nullptr))
			return 0x00000000;

		return WriteData(
			vvd.GetVoxel(currentCorner + glm::uvec3(0u, 0u, 0u)) == nullptr ? 0x00000000 : 0xffffffff,
			vvd.GetVoxel(currentCorner + glm::uvec3(1u, 0u, 0u)) == nullptr ? 0x00000000 : 0xffffffff,
			vvd.GetVoxel(currentCorner + glm::uvec3(0u, 1u, 0u)) == nullptr ? 0x00000000 : 0xffffffff,
			vvd.GetVoxel(currentCorner + glm::uvec3(1u, 1u, 0u)) == nullptr ? 0x00000000 : 0xffffffff,
			vvd.GetVoxel(currentCorner + glm::uvec3(0u, 0u, 1u)) == nullptr ? 0x00000000 : 0xffffffff,
			vvd.GetVoxel(currentCorner + glm::uvec3(1u, 0u, 1u)) == nullptr ? 0x00000000 : 0xffffffff,
			vvd.GetVoxel(currentCorner + glm::uvec3(0u, 1u, 1u)) == nullptr ? 0x00000000 : 0xffffffff,
			vvd.GetVoxel(currentCorner + glm::uvec3(1u, 1u, 1u)) == nullptr ? 0x00000000 : 0xffffffff);
	}

	uint32_t a = CreateFromVoxelVolumeDataRec(vvd, currentDepth + 1u, { currentCorner.x, currentCorner.y, currentCorner.z });
	uint32_t b = CreateFromVoxelVolumeDataRec(vvd, currentDepth + 1u, { currentCorner.x + voxelsPerUnit / 2, currentCorner.y, currentCorner.z });
	uint32_t c = CreateFromVoxelVolumeDataRec(vvd, currentDepth + 1u, { currentCorner.x, currentCorner.y + voxelsPerUnit / 2, currentCorner.z });
	uint32_t d = CreateFromVoxelVolumeDataRec(vvd, currentDepth + 1u, { currentCorner.x + voxelsPerUnit / 2, currentCorner.y + voxelsPerUnit / 2, currentCorner.z });

	uint32_t e = CreateFromVoxelVolumeDataRec(vvd, currentDepth + 1u, { currentCorner.x, currentCorner.y, currentCorner.z + voxelsPerUnit / 2 });
	uint32_t f = CreateFromVoxelVolumeDataRec(vvd, currentDepth + 1u, { currentCorner.x + voxelsPerUnit / 2, currentCorner.y, currentCorner.z + voxelsPerUnit / 2 });
	uint32_t g = CreateFromVoxelVolumeDataRec(vvd, currentDepth + 1u, { currentCorner.x, currentCorner.y + voxelsPerUnit / 2, currentCorner.z + voxelsPerUnit / 2 });
	uint32_t h = CreateFromVoxelVolumeDataRec(vvd, currentDepth + 1u, { currentCorner.x + voxelsPerUnit / 2, currentCorner.y + voxelsPerUnit / 2, currentCorner.z + voxelsPerUnit / 2 });

	if ((a & b & c & d & e & f & g & h) == 0xffffffff)
		return 0xffffffff;
	if ((a | b | c | d | e | f | g | h) == 0x00000000)
		return 0x00000000;
	if (currentDepth == 0)
		return WriteDataRoot(a, b, c, d, e, f, g, h);
	return WriteData(a, b, c, d, e, f, g, h);
}

void sf::SparseVoxelOctree::CreateFromVoxelVolumeData(const VoxelVolumeData& vvd)
{
	uint32_t maxDimension = glm::max(vvd.voxelCountPerAxis.z, glm::max(vvd.voxelCountPerAxis.x, vvd.voxelCountPerAxis.y));
	depth = Math::CountTrailingZeroes(Math::NextPowerOf2(maxDimension));

	CreateFromVoxelVolumeDataRec(vvd, 0u, { 0u, 0u, 0u });
}

uint32_t sf::SparseVoxelOctree::Sample(const glm::uvec3& coords)
{
	glm::uvec3 localCoords = coords;
	uint32_t currentDepth = 0u;
	uint32_t currentDataIndex = 0u;
	while (currentDepth < depth)
	{
		uint32_t voxelsPerUnit = 1u << (depth - currentDepth);

		if (voxelsPerUnit == 2u)
		{
			/* Base case */
			uint32_t corner = localCoords.x | (localCoords.y << 1) | (localCoords.z << 2);
			return ReadData(currentDataIndex, corner);
		}

		uint32_t corner =
			(localCoords.x >= voxelsPerUnit / 2) |
			((localCoords.y >= voxelsPerUnit / 2) << 1) |
			((localCoords.z >= voxelsPerUnit / 2) << 2);
		uint32_t value = ReadData(currentDataIndex, corner);
		if (value == 0 || value == 0xffffffff)
			return value;

		localCoords.x %= voxelsPerUnit / 2;
		localCoords.y %= voxelsPerUnit / 2;
		localCoords.z %= voxelsPerUnit / 2;
		currentDataIndex = value;
		currentDepth++;
	}
	assert(false);
	return 0;
}