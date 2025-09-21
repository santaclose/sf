#include <MeshData.h>
#include <cstring>
#include <cassert>
#include <fstream>

void sf::MeshData::ChangeVertexBufferLayout(const sf::BufferLayout& newLayout)
{
	if (this->vertexBuffer == nullptr)
	{
		this->vertexBufferLayout = newLayout;
		return;
	}

	const std::vector<BufferComponentInfo>& oldComponents = this->vertexBufferLayout.GetComponentInfos();
	void* newVertexBuffer = malloc(newLayout.GetSize() * this->vertexCount);
	for (int i = 0; i < this->vertexCount; i++)
	{
		for (int j = 0; j < oldComponents.size(); j++)
		{
			const BufferComponentInfo* dataComponentInNewLayout = newLayout.GetComponentInfo(oldComponents[j].component);
			if (dataComponentInNewLayout == nullptr) // new layout does not have this component
				continue;
			if (oldComponents[j].dataType != dataComponentInNewLayout->dataType)
				continue; // cannot use the data in this component

			uint32_t dataTypeSize = GetDataTypeSize(oldComponents[j].dataType);
			void* targetPointer = newLayout.Access<void>(newVertexBuffer, oldComponents[j].component, i);
			void* sourcePointer = this->vertexBufferLayout.Access<void>(this->vertexBuffer, oldComponents[j].component, i);
			memcpy(targetPointer, sourcePointer, dataTypeSize);
		}
	}
	free(this->vertexBuffer);
	this->vertexBufferLayout = newLayout;
	this->vertexBuffer = newVertexBuffer;
}


void sf::MeshData::SaveToFile(const char* targetFile)
{
	std::ofstream file;
	file.open(targetFile, std::ios::trunc | std::ios::binary);

	// Vertex layout
	uint32_t componentCount = vertexBufferLayout.GetComponentInfos().size();
	file.write((char*) &componentCount, sizeof(componentCount));
	for (BufferComponentInfo comp : vertexBufferLayout.GetComponentInfos())
		file.write((char*) &comp.component, sizeof(comp.component));

	// Indices
	file.write((char*) &indexCount, sizeof(indexCount));
	file.write((char*) indexBuffer, indexCount * sizeof(uint32_t));

	// Pieces
	file.write((char*) &pieceCount, sizeof(pieceCount));
	file.write((char*) pieces, pieceCount * sizeof(uint32_t));

	// Vertices
	file.write((char*) &vertexCount, sizeof(vertexCount));
	uint32_t vertexBufferSizeInBytes = vertexBufferLayout.GetSize() * vertexCount;
	file.write((char*) vertexBuffer, vertexBufferSizeInBytes);

	file.close();
}

bool sf::MeshData::LoadFromFile(const char* targetFile)
{
	assert(pieces == nullptr && vertexBuffer == nullptr && indexBuffer == nullptr);
	std::ifstream file;
	file.open(targetFile, std::ios::binary);
	if (!file)
		return false;

	// Vertex layout
	uint32_t componentCount;
	file.read((char*)&componentCount, sizeof(componentCount));
	std::vector<BufferComponent> components;
	components.resize(componentCount);
	file.read((char*) components.data(), componentCount * sizeof(BufferComponent));
	vertexBufferLayout = BufferLayout(components);

	// Indices
	file.read((char*) &indexCount, sizeof(indexCount));
	indexBuffer = new uint32_t[indexCount];
	file.read((char*) indexBuffer, indexCount * sizeof(uint32_t));

	// Pieces
	file.read((char*) &pieceCount, sizeof(pieceCount));
	pieces = new uint32_t[pieceCount];
	file.read((char*) pieces, pieceCount * sizeof(uint32_t));

	// Vertices
	file.read((char*) &vertexCount, sizeof(vertexCount));
	uint32_t vertexBufferSizeInBytes = vertexBufferLayout.GetSize() * vertexCount;
	vertexBuffer = new char[vertexBufferSizeInBytes];
	file.read((char*) vertexBuffer, vertexBufferSizeInBytes);

	file.close();

	return true;
}