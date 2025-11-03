#include "BufferLayout.h"

#include <assert.h>

sf::DataType sf::BufferLayout::GetComponentDataType(BufferComponent component)
{
	switch (component)
	{
	case BufferComponent::Position:
	case BufferComponent::Normal:
	case BufferComponent::Tangent:
	case BufferComponent::Color:
		return DataType::vec3f32;
	case BufferComponent::UV:
		return DataType::vec2f32;
	case BufferComponent::BoneWeights:
	case BufferComponent::BoneIndices:
	case BufferComponent::Rotation:
		return DataType::vec4f32;
	case BufferComponent::AO:
	case BufferComponent::Scale:
	case BufferComponent::SpawnTime:
		return DataType::f32;
	default:
		assert(false);
	}
}

sf::BufferLayout::BufferLayout(const std::vector<BufferComponent>& components)
{
	this->componentInfos.resize(components.size());
	this->sizeInBytes = 0;
	uint32_t i = 0;
	for (BufferComponent component : components)
	{
		DataType componentDataType = GetComponentDataType(component);
		this->componentMap[component] = i;
		this->componentInfos[i].component = component;
		this->componentInfos[i].dataType = componentDataType;
		this->componentInfos[i].byteOffset = this->sizeInBytes;
		this->sizeInBytes += GetDataTypeSize(componentDataType);
		i++;
	}
}