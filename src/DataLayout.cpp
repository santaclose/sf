#include "DataLayout.h"

#include <assert.h>
#include <iostream>

#include <FileUtils.h>
#include <VertexAttributes.h>

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

sf::DataLayout::DataLayout(const char* vertexShaderPath)
{
	// Restricted to 10 descriptor sets ID MISSIN
	this->components.clear();
	this->sizeInBytes = 0;
	const static std::string vertexAttributeString = "layout(location = ";
	std::string shaderContents;
	FileUtils::ReadFileAsString(std::string(vertexShaderPath) + ".glsl", shaderContents);

	for (const char* c = shaderContents.data(); *c != '\0'; c++)
	{
		if (strncmp(c, vertexAttributeString.c_str(), vertexAttributeString.size()) == 0)
		{
			for (; *c != '\n' && strncmp(c, "in", 2) != 0; c++);
			if (*c == '\n')
				continue;
			// we found a vertex attribute

			this->components.emplace_back();
			this->components.back().byteOffset = this->sizeInBytes;

			c += 3;
			int len;
			for (len = 0; *(c + len) != ' '; len++);
			std::string dataTypeSubstring = shaderContents.substr(c - shaderContents.data(), len);

			c += len + 1;
			for (len = 0; *(c + len) != ';'; len++);
			std::string attributeKindSubstring = shaderContents.substr(c - shaderContents.data(), len);

			if (attributeKindSubstring.compare("vPosition") == 0)
				this->components.back().id = VertexAttribute::Position;
			else if (attributeKindSubstring.compare("vNormal") == 0)
				this->components.back().id = VertexAttribute::Normal;
			else if (attributeKindSubstring.compare("vTangent") == 0)
				this->components.back().id = VertexAttribute::Tangent;
			else if (attributeKindSubstring.compare("vBitangent") == 0)
				this->components.back().id = VertexAttribute::Bitangent;
			else if (attributeKindSubstring.compare("vColor") == 0)
				this->components.back().id = VertexAttribute::Color;
			else if (attributeKindSubstring.compare("vTexCoords") == 0)
				this->components.back().id = VertexAttribute::TexCoords;
			else if (attributeKindSubstring.compare("vAmbientOcclusion") == 0)
				this->components.back().id = VertexAttribute::AmbientOcclusion;
			else if (attributeKindSubstring.compare("vBoneWeights") == 0)
				this->components.back().id = VertexAttribute::BoneWeights;
			else if (attributeKindSubstring.compare("vBoneIndices") == 0)
				this->components.back().id = VertexAttribute::BoneIndices;
			else
				assert(false); // failed to parse vertex attribute

			if (dataTypeSubstring.compare("vec3") == 0)
				this->components.back().dataType = DataType::vec3f32;
			else if (dataTypeSubstring.compare("vec2") == 0)
				this->components.back().dataType = DataType::vec2f32;
			else if (dataTypeSubstring.compare("vec4") == 0)
				this->components.back().dataType = DataType::vec4f32;
			else if (dataTypeSubstring.compare("float") == 0)
				this->components.back().dataType = DataType::f32;
			else if (dataTypeSubstring.compare("int") == 0)
				this->components.back().dataType = DataType::i32;
			else if (dataTypeSubstring.compare("mat3") == 0)
				this->components.back().dataType = DataType::mat3f32;
			else if (dataTypeSubstring.compare("mat2") == 0)
				this->components.back().dataType = DataType::mat2f32;
			else if (dataTypeSubstring.compare("mat4") == 0)
				this->components.back().dataType = DataType::mat4f32;
			else
				assert(false); // failed to parse vertex attribute data type

			this->sizeInBytes += GetDataTypeSize(this->components.back().dataType);
		}
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

bool sf::operator==(const DataLayout& a, const DataLayout& b)
{
	if (a.sizeInBytes != b.sizeInBytes || a.components.size() != b.components.size())
		return false;
	for (int i = 0; i < a.components.size(); i++)
		if (a.components[i].id != b.components[i].id ||
			a.components[i].dataType != b.components[i].dataType ||
			a.components[i].byteOffset != b.components[i].byteOffset)
			return false;
	return true;
}
