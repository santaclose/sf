#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <DataTypes.h>

namespace sf
{
	struct DataComponent
	{
		// Vertex attribute meant to be used as id for vertex buffers
		uint32_t id;
		DataType dataType;
		uint32_t byteOffset;
	};

	class DataLayout
	{
	private:
		uint32_t sizeInBytes = 0;
		std::vector<DataComponent> components;
		std::unordered_map<uint32_t, uint32_t> componentsById;

	public:
		DataLayout() = default;
		DataLayout(const std::vector<std::pair<uint32_t, DataType>>& components);
		DataLayout(const char* vertexShaderPath);
		uint32_t GetSize() const;

		void* Access(void* buffer, uint32_t componentName, uint32_t index) const;
		const DataComponent* GetComponent(uint32_t componentName) const;
		const std::vector<DataComponent>& GetComponents() const;
		friend bool operator== (const DataLayout& a, const DataLayout& b);
	};

	bool operator==(const DataLayout& a, const DataLayout& b);
}