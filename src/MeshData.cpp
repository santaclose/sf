#include "MeshData.h"

#include <cstring>
#include <cassert>
#include <fstream>

void sf::MeshData::Create(
	const BufferLayout* vertexBufferLayout,
	uint8_t* vertexBuffer,
	uint32_t vertexCount,
	const std::vector<uint32_t>& indexVector,
	const std::vector<uint32_t>* pieceVector,
	uint8_t vertexCountPerPrimitive)
{
	assert((vertexCount == 0 && indexCount == 0) ||
		(vertexBufferLayout != nullptr && vertexBuffer != nullptr && indexVector.size() > 0));
	if (vertexBufferLayout != nullptr)
		this->vertexBufferLayout = *vertexBufferLayout;
	this->vertexBuffer = vertexBuffer;
	this->vertexCount = vertexCount;

	this->indexBuffer = new uint32_t[indexVector.size()];
	this->indexCount = indexVector.size();
	memcpy(this->indexBuffer, indexVector.data(), indexVector.size() * sizeof(uint32_t));

	if (pieceVector != nullptr)
	{
		this->pieces = new uint32_t[pieceVector->size()];
		this->pieceCount = pieceVector->size();
		memcpy(this->pieces, pieceVector->data(), pieceVector->size() * sizeof(uint32_t));
	}
	else
	{
		this->pieces = nullptr;
		this->pieceCount = 1;
	}

	this->vertexCountPerPrimitive = vertexCountPerPrimitive;
	this->usesExternalVertexBuffer = false;
}

void sf::MeshData::Destroy()
{
	if (vertexBuffer != nullptr && !usesExternalVertexBuffer)
		delete[] vertexBuffer;
	if (indexBuffer != nullptr)
		delete[] indexBuffer;
	if (pieces != nullptr)
		delete[] pieces;
	*this = MeshData();
}

void sf::MeshData::ChangeVertexBufferLayout(const sf::BufferLayout& newLayout)
{
	if (this->vertexBuffer == nullptr)
	{
		this->vertexBufferLayout = newLayout;
		return;
	}

	std::vector<BufferComponentInfo> oldComponents = this->vertexBufferLayout.GetComponentInfos();
	uint8_t* newVertexBuffer = new uint8_t[newLayout.GetSize() * this->vertexCount];
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
	delete[] this->vertexBuffer;
	this->vertexBufferLayout = newLayout;
	this->vertexBuffer = newVertexBuffer;
}

bool sf::MeshData::SaveToFile(const char* targetFile)
{
	std::ofstream file;
	file.open(targetFile, std::ios::trunc | std::ios::binary);
	if (!file)
		return false;

	// Vertex layout
	BufferLayoutBitmask bitmask = vertexBufferLayout.GetBitmask();
	file.write((char*) &bitmask, sizeof(bitmask));

	// Vertices per primitive
	file.write((char*) &vertexCountPerPrimitive, sizeof(vertexCountPerPrimitive));

	// Indices
	file.write((char*) &indexCount, sizeof(indexCount));
	file.write((char*) indexBuffer, indexCount * sizeof(uint32_t));

	// Pieces
	file.write((char*) &pieceCount, sizeof(pieceCount));
	if (pieceCount > 1)
		file.write((char*) pieces, pieceCount * sizeof(uint32_t));

	// Vertices
	file.write((char*) &vertexCount, sizeof(vertexCount));
	uint32_t vertexBufferSizeInBytes = vertexBufferLayout.GetSize() * vertexCount;
	file.write((char*) vertexBuffer, vertexBufferSizeInBytes);

	file.close();

	return true;
}

bool sf::MeshData::LoadFromFile(const char* targetFile)
{
	assert(pieces == nullptr && vertexBuffer == nullptr && indexBuffer == nullptr);
	std::ifstream file;
	file.open(targetFile, std::ios::binary);
	if (!file)
		return false;

	// Vertex layout
	BufferLayoutBitmask bitmask;
	file.read((char*) &bitmask, sizeof(bitmask));
	vertexBufferLayout.Destroy();
	vertexBufferLayout.CreateFromBitmask(bitmask);

	// Vertices per primitive
	file.read((char*) &vertexCountPerPrimitive, sizeof(vertexCountPerPrimitive));

	// Indices
	file.read((char*) &indexCount, sizeof(indexCount));
	indexBuffer = new uint32_t[indexCount];
	file.read((char*) indexBuffer, indexCount * sizeof(uint32_t));

	// Pieces
	file.read((char*) &pieceCount, sizeof(pieceCount));
	if (pieceCount > 1)
	{
		pieces = new uint32_t[pieceCount];
		file.read((char*) pieces, pieceCount * sizeof(uint32_t));
	}
	else
		pieces = nullptr;

	// Vertices
	file.read((char*) &vertexCount, sizeof(vertexCount));
	uint32_t vertexBufferSizeInBytes = vertexBufferLayout.GetSize() * vertexCount;
	vertexBuffer = new uint8_t[vertexBufferSizeInBytes];
	file.read((char*) vertexBuffer, vertexBufferSizeInBytes);

	file.close();

	return true;
}