#pragma once

#include <VoxelVolumeData.h>

namespace sf {

	struct SparseVoxelOctree
	{
		uint32_t depth;
		std::vector<uint32_t> data;
		uint32_t CreateFromVoxelVolumeDataRec(const VoxelVolumeData& vvd, uint32_t currentDepth, const glm::uvec3& currentCorner);
		void CreateFromVoxelVolumeData(const VoxelVolumeData& vvd);

		uint32_t Sample(const glm::uvec3& coords);

	private:
		inline uint32_t WriteData(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g, uint32_t h)
		{
			if (data.size() == 0)
				data.resize(data.size() + 8);
			size_t currentPos = data.size() / 8;
			data.resize(data.size() + 8);
			data[data.size() - 8] = a;
			data[data.size() - 7] = b;
			data[data.size() - 6] = c;
			data[data.size() - 5] = d;
			data[data.size() - 4] = e;
			data[data.size() - 3] = f;
			data[data.size() - 2] = g;
			data[data.size() - 1] = h;
			assert(currentPos < 0xffffffff);
			return (uint32_t) currentPos;
		}

		inline uint32_t WriteDataRoot(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g, uint32_t h)
		{
			data[0] = a;
			data[1] = b;
			data[2] = c;
			data[3] = d;
			data[4] = e;
			data[5] = f;
			data[6] = g;
			data[7] = h;
			return 0;
		}

		inline uint32_t ReadData(uint32_t index, uint32_t corner)
		{
			return data[index * 8 + corner];
		}
	};
}