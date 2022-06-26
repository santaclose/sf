#include <MeshData.h>

std::string sf::MeshData::vertexPositionAttr = "pos";
std::string sf::MeshData::vertexNormalAttr = "nor";
std::string sf::MeshData::vertexTangentAttr = "tan";
std::string sf::MeshData::vertexBitangentAttr = "bit";
std::string sf::MeshData::vertexUvsAttr = "uv";
std::string sf::MeshData::vertexColorAttr = "col";
std::string sf::MeshData::vertexAoAttr = "ao";

void sf::MeshData::ChangeVertexLayout(const sf::DataLayout& newLayout)
{
	if (this->vertexBuffer == nullptr)
	{
		this->vertexLayout = newLayout;
		return;
	}

	const std::vector<DataComponent>& oldComponents = this->vertexLayout.GetComponents();
	void* newVertexBuffer = malloc(newLayout.GetSize() * this->vertexCount);
	for (int i = 0; i < this->vertexCount; i++)
	{
		for (int j = 0; j < oldComponents.size(); j++)
		{
			const DataComponent* dataComponentInNewLayout = newLayout.GetComponent(oldComponents[j].name);
			if (dataComponentInNewLayout == nullptr) // new layout does not have this component
				continue;
			if (oldComponents[j].dataType != dataComponentInNewLayout->dataType)
				continue; // cannot use the data in this component

			uint32_t dataTypeSize = GetDataTypeSize(oldComponents[j].dataType);
			void* targetPointer = newLayout.Access(newVertexBuffer, oldComponents[j].name, i);
			void* sourcePointer = this->vertexLayout.Access(this->vertexBuffer, oldComponents[j].name, i);
			memcpy(targetPointer, sourcePointer, dataTypeSize);
		}
	}
	free(this->vertexBuffer);
	this->vertexLayout = newLayout;
	this->vertexBuffer = newVertexBuffer;
}