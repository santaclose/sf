#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <DataTypes.h>

namespace sf
{
	struct DataComponent
	{
		std::string name;
		DataType dataType;
		uint32_t byteOffset;
	};

	class DataLayout
	{
	private:
		uint32_t sizeInBytes;
		std::vector<DataComponent> components;
		std::unordered_map<std::string, uint32_t> componentsByName;

	public:
		DataLayout(const std::vector<std::pair<std::string, DataType>>& components);
		uint32_t GetSize() const;

		void* Access(void* buffer, const std::string& componentName, uint32_t index) const;
		const DataComponent* GetComponent(const std::string& componentName) const;
		const std::vector<DataComponent>& GetComponents() const;
	};
}