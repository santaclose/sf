#pragma once

#include <vector>
#include <string>
#include <cassert>

#include <DataTypes.h>

namespace sf
{
	enum class BufferComponent
	{
		BoneWeights,
		BoneIndices,
		Rotation,

		FIRST_VEC3,
		Position = FIRST_VEC3,
		Normal,
		Tangent,
		Color,

		FIRST_VEC2,
		UV = FIRST_VEC2,

		FIRST_F32,
		AO = FIRST_F32,
		Scale,
		SpawnTime,
		MAX,
		INVALID = MAX
	};

	inline DataType GetBufferComponentDataType(BufferComponent component)
	{
		uint32_t componentU32 = (uint32_t) component;
		if (componentU32 < (uint32_t) BufferComponent::FIRST_VEC3)
			return DataType::vec4f32;
		if (componentU32 < (uint32_t) BufferComponent::FIRST_VEC2)
			return DataType::vec3f32;
		if (componentU32 < (uint32_t) BufferComponent::FIRST_F32)
			return DataType::vec2f32;
		assert(componentU32 < (uint32_t) BufferComponent::MAX);
		return DataType::f32;
	}

	static_assert((uint32_t) BufferComponent::MAX <= 32u);

	struct BufferComponentInfo
	{
		BufferComponent component;
		DataType dataType;
		uint32_t byteOffset;
	};

	/* Buffer layouts only accept one order per component combination, and
	 * the order is deduced from bitmask as specified in the BufferComponent enum */

	typedef uint32_t BufferLayoutBitmask;

	class BufferLayout
	{
	private:
		BufferLayoutBitmask bitmask = 0;
		uint32_t sizeInBytes = 0;
		BufferComponentInfo componentInfo[32u];

	public:
		void CreateFromBitmask(BufferLayoutBitmask bitmask);
		void CreateFromVector(const std::vector<BufferComponent>& components);
		void Destroy();

		BufferLayout(BufferLayoutBitmask bitmask)
		{
			CreateFromBitmask(bitmask);
		}
		BufferLayout(const std::vector<BufferComponent>& components)
		{
			CreateFromVector(components);
		}
		BufferLayout() = default;
		~BufferLayout() = default;

		static DataType GetComponentDataType(BufferComponent component);

		inline uint32_t GetSize() const
		{
			return this->sizeInBytes;
		}

		template <typename T>
		inline T* Access(uint8_t* buffer, BufferComponent component, uint32_t index) const
		{
			uint32_t componentU32 = (uint32_t) component;
			assert(componentU32 < (uint32_t) BufferComponent::MAX);
			return (T*)(buffer + (this->sizeInBytes * index) + this->componentInfo[componentU32].byteOffset);
		}

		inline const BufferComponentInfo* GetComponentInfo(BufferComponent component) const
		{
			uint32_t componentU32 = (uint32_t) component;
			assert(componentU32 < (uint32_t) BufferComponent::MAX);
			if (((1u << componentU32) & bitmask) == 0)
				return nullptr;
			return &componentInfo[componentU32];
		}

		inline std::vector<BufferComponentInfo> GetComponentInfos() const
		{
			std::vector<BufferComponentInfo> output;
			for (uint32_t componentU32 = 0; componentU32 < (uint32_t) BufferComponent::MAX; componentU32++)
			{
				if (((1u << componentU32) & (this->bitmask)) == 0)
					continue;
				output.push_back(componentInfo[componentU32]);
			}
			return output;
		}

		inline BufferLayoutBitmask GetBitmask() const
		{
			return this->bitmask;
		}
	};
}