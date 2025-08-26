#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <DataTypes.h>

namespace sf
{
	enum class BufferComponent
	{
		VertexPosition,
		VertexNormal,
		VertexTangent,
		VertexColor,
		VertexUV,
		VertexAO,
		VertexBoneWeights,
		VertexBoneIndices,
		VoxelPosition,
		VoxelNormal,
		VoxelColor,
		VoxelUV,
		ParticlePosition,
		ParticleRotation,
		ParticleScale,
		ParticleSpawnTime
	};

	struct BufferComponentInfo
	{
		BufferComponent component;
		DataType dataType;
		uint32_t byteOffset;
	};

	class BufferLayout
	{
	private:
		uint32_t sizeInBytes = 0;
		std::vector<BufferComponentInfo> componentInfos;
		std::unordered_map<BufferComponent, uint32_t> componentMap;

	public:
		BufferLayout(const std::vector<BufferComponent>& components);
		BufferLayout() = default;
		~BufferLayout() = default;

		static DataType GetComponentDataType(BufferComponent component);

		inline uint32_t GetSize() const
		{
			return this->sizeInBytes;
		}

		template <typename T>
		inline T* Access(void* buffer, BufferComponent component, uint32_t index) const
		{
			return (T*)(((uint8_t*)buffer) + (this->sizeInBytes * index) + this->componentInfos[this->componentMap.at(component)].byteOffset);
		}

		inline const BufferComponentInfo* GetComponentInfo(BufferComponent component) const
		{
			if (this->componentMap.find(component) == this->componentMap.end())
				return nullptr;
			return &(this->componentInfos[this->componentMap.at(component)]);
		}

		inline const std::vector<BufferComponentInfo>& GetComponentInfos() const
		{
			return this->componentInfos;
		}
	};
}