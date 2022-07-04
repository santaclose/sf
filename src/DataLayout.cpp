#include "DataLayout.h"

#include <assert.h>

sf::DataLayout::DataLayout(const std::vector<std::pair<uint32_t, DataType>>& components)
{
	this->components.resize(components.size());
	this->sizeInBytes = 0;
	uint32_t i = 0;
	for (const auto& dc : components)
	{
		this->componentsById[dc.first] = i;
		this->components[i].id = dc.first;
		this->components[i].dataType = dc.second;
		this->components[i].byteOffset = this->sizeInBytes;
		this->sizeInBytes += GetDataTypeSize(dc.second);
		i++;
	}
}

uint32_t sf::DataLayout::GetSize() const
{
	return this->sizeInBytes;
}

void* sf::DataLayout::Access(void* buffer, uint32_t componentName, uint32_t index) const
{
	return (((uint8_t*)buffer) + (this->sizeInBytes * index) + this->components[this->componentsById.at(componentName)].byteOffset);
}

const sf::DataComponent* sf::DataLayout::GetComponent(uint32_t componentName) const
{
	if (this->componentsById.find(componentName) == this->componentsById.end())
		return nullptr;
	return &(this->components[this->componentsById.at(componentName)]);
}

const std::vector<sf::DataComponent>& sf::DataLayout::GetComponents() const
{
	return this->components;
}