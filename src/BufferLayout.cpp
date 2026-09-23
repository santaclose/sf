#include "BufferLayout.h"

void sf::BufferLayout::CreateFromBitmask(BufferLayoutBitmask bitmask)
{
	this->bitmask = bitmask;

	this->sizeInBytes = 0;
	for (uint32_t componentU32 = 0; componentU32 < (uint32_t) BufferComponent::MAX; componentU32++)
	{
		if (((1u << componentU32) & (this->bitmask)) == 0)
		{
			this->componentInfo[componentU32].component = BufferComponent::INVALID;
			continue;
		}

		BufferComponent component = (BufferComponent) componentU32;
		DataType componentDataType = GetBufferComponentDataType(component);

		this->componentInfo[componentU32].component = component;
		this->componentInfo[componentU32].dataType = componentDataType;
		this->componentInfo[componentU32].byteOffset = this->sizeInBytes;
		this->sizeInBytes += GetDataTypeSize(componentDataType);
	}
}

/* The order of components doesn't matter in the argument */
void sf::BufferLayout::CreateFromVector(const std::vector<BufferComponent>& components)
{
	BufferLayoutBitmask bitmask = (BufferLayoutBitmask) 0u;
	for (BufferComponent component : components)
	{
		uint32_t componentU32 = (uint32_t) component;
		assert(componentU32 < (uint32_t) BufferComponent::MAX);
		bitmask |= (1u << componentU32);
	}
	CreateFromBitmask(bitmask);
}

void sf::BufferLayout::Destroy()
{
	*this = BufferLayout();
}