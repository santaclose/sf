#include "BufferLayout.h"

#include <assert.h>

sf::DataType sf::BufferLayout::GetComponentDataType(BufferComponent component)
{
	switch (component)
	{
	case BufferComponent::VertexPosition:
	case BufferComponent::VertexNormal:
	case BufferComponent::VertexTangent:
	case BufferComponent::VertexColor:
	case BufferComponent::VoxelPosition:
	case BufferComponent::VoxelNormal:
	case BufferComponent::VoxelColor:
	case BufferComponent::ParticlePosition:
		return DataType::vec3f32;
	case BufferComponent::VertexUV:
	case BufferComponent::VoxelUV:
		return DataType::vec2f32;
	case BufferComponent::VertexBoneWeights:
	case BufferComponent::VertexBoneIndices:
	case BufferComponent::ParticleRotation:
		return DataType::vec4f32;
	case BufferComponent::VertexAO:
	case BufferComponent::ParticleScale:
	case BufferComponent::ParticleTime:
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