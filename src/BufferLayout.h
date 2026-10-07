#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <map>

#include <DataTypes.h>

namespace sf::Buffer
{
	enum class Component
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

	inline DataType GetComponentDataType(Component component)
	{
		uint32_t componentU32 = (uint32_t) component;
		if (componentU32 < (uint32_t) Component::FIRST_VEC3)
			return DataType::vec4f32;
		if (componentU32 < (uint32_t) Component::FIRST_VEC2)
			return DataType::vec3f32;
		if (componentU32 < (uint32_t) Component::FIRST_F32)
			return DataType::vec2f32;
		assert(componentU32 < (uint32_t) Component::MAX);
		return DataType::f32;
	}

	static_assert((uint32_t) Component::MAX <= 32u);

	struct ComponentInfo
	{
		DataType dataType;
		uint32_t byteOffset;
	};

	struct LayoutInfo
	{
		uint32_t sizeInBytes = 0;
		std::map<Component, ComponentInfo> components;
		inline uint32_t GetSize() const
		{
			return sizeInBytes;
		}
		inline const ComponentInfo* GetComponentInfo(Component component) const
		{
			if (components.find(component) == components.end())
				return nullptr;
			return &components.at(component);
		}
		template <typename T>
		inline T* Access(uint8_t* buffer, Component component, uint32_t index) const
		{
			return (T*)(buffer + (sizeInBytes * index) + components.at(component).byteOffset);
		}
	};

	struct Layout
	{
		uint32_t bitmask = 0u;
		inline Layout() = default;
		inline Layout(const std::vector<Component>& components)
		{
			for (const Component& c : components)
				bitmask |= (1u << ((uint32_t) c));
		}
		inline Layout(uint32_t bm)
		{
			bitmask = bm;
		}
		Layout& operator=(uint32_t bm)
		{
			bitmask = bm;
			return *this;
		}
		bool operator==(uint32_t bm) const
		{
			return bitmask == bm;
		}
		inline uint32_t GetBitmask() const
		{
			return bitmask;
		}
		inline uint32_t GetSize() const
		{
			return GetInfo().GetSize();
		}
		const LayoutInfo& GetInfo() const;
		inline const ComponentInfo* GetComponentInfo(Component component) const
		{
			return GetInfo().GetComponentInfo(component);
		}
		template <typename T>
		inline T* Access(uint8_t* buffer, Component component, uint32_t index) const
		{
			return GetInfo().Access<T>(buffer, component, index);
		}
	};
}