#include <MeshData.h>

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
			const DataComponent* dataComponentInNewLayout = newLayout.GetComponent(oldComponents[j].id);
			if (dataComponentInNewLayout == nullptr) // new layout does not have this component
				continue;
			if (oldComponents[j].dataType != dataComponentInNewLayout->dataType)
				continue; // cannot use the data in this component

			uint32_t dataTypeSize = GetDataTypeSize(oldComponents[j].dataType);
			void* targetPointer = newLayout.Access(newVertexBuffer, oldComponents[j].id, i);
			void* sourcePointer = this->vertexLayout.Access(this->vertexBuffer, oldComponents[j].id, i);
			memcpy(targetPointer, sourcePointer, dataTypeSize);
		}
	}
	free(this->vertexBuffer);
	this->vertexLayout = newLayout;
	this->vertexBuffer = newVertexBuffer;
}