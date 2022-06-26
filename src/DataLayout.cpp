#include "DataLayout.h"

#include <assert.h>

sf::DataLayout::DataLayout(const std::vector<std::pair<std::string, DataType>>& components)
{
	this->components.resize(components.size());
	this->sizeInBytes = 0;
	uint32_t i = 0;
	for (const auto& dc : components)
	{
		this->componentsByName[dc.first] = i;
		this->components[i].name = dc.first;
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

void* sf::DataLayout::Access(void* buffer, const std::string& componentName, uint32_t index) const
{
	return (((uint8_t*)buffer) + (this->sizeInBytes * index) + this->components[this->componentsByName.at(componentName)].byteOffset);
}

const sf::DataComponent* sf::DataLayout::GetComponent(const std::string& componentName) const
{
	if (this->componentsByName.find(componentName) == this->componentsByName.end())
		return nullptr;
	return &(this->components[this->componentsByName.at(componentName)]);
}

const std::vector<sf::DataComponent>& sf::DataLayout::GetComponents() const
{
	return this->components;
}