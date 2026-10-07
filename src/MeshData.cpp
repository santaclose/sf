#include "MeshData.h"

#include <cstring>
#include <cassert>
#include <fstream>

void sf::MeshData::Create(
	Buffer::Layout vertexBufferLayout,
	uint8_t* vertexBuffer,
	uint32_t vertexCount,
	const std::vector<uint32_t>& indexVector,
	const std::vector<uint32_t>* pieceVector,
	uint8_t vertexCountPerPrimitive)
{
	assert((vertexCount == 0 && indexCount == 0) || (vertexBuffer != nullptr && indexVector.size() > 0));
	this->vertexBufferLayout = vertexBufferLayout;
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

void sf::MeshData::ChangeVertexBufferLayout(Buffer::Layout newLayout)
{
	if (this->vertexBuffer == nullptr)
	{
		this->vertexBufferLayout = newLayout;
		return;
	}

	const Buffer::LayoutInfo& layoutToChange = this->vertexBufferLayout.GetInfo();
	const Buffer::LayoutInfo& layout = newLayout.GetInfo();

	uint8_t* newVertexBuffer = new uint8_t[layout.GetSize() * this->vertexCount];
	for (int i = 0; i < this->vertexCount; i++)
	{
		for (const auto& [component, componentInfo] : layoutToChange.components)
		{
			const Buffer::ComponentInfo* newComponentInfo = layout.GetComponentInfo(component);
			if (newComponentInfo == nullptr)
				/* New layout does not have this component */
				continue;

			uint32_t dataTypeSize = GetDataTypeSize(componentInfo.dataType);
			uint8_t* targetPointer = layout.Access<uint8_t>(newVertexBuffer, component, i);
			uint8_t* sourcePointer = layoutToChange.Access<uint8_t>(this->vertexBuffer, component, i);
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
	file.write((char*) &vertexBufferLayout.bitmask, sizeof(vertexBufferLayout.bitmask));

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
	file.read((char*) &vertexBufferLayout.bitmask, sizeof(vertexBufferLayout.bitmask));

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