#include "BufferLayout.h"

#include <unordered_map>

const sf::Buffer::LayoutInfo& sf::Buffer::Layout::GetInfo() const
{
	static std::unordered_map<uint32_t, LayoutInfo> infoCache;
	if (infoCache.find(bitmask) == infoCache.end())
	{
		infoCache[bitmask].sizeInBytes = 0;
		for (uint32_t componentU32 = 0; componentU32 < (uint32_t) Component::MAX; componentU32++)
		{
			if (((1u << componentU32) & (this->bitmask)) == 0)
				continue;
			Component component = (Component) componentU32;
			DataType componentDataType = GetComponentDataType(component);
			infoCache[bitmask].components[component].dataType = componentDataType;
			infoCache[bitmask].components[component].byteOffset = infoCache[bitmask].sizeInBytes;
			infoCache[bitmask].sizeInBytes += GetDataTypeSize(componentDataType);
		}
	}
	return infoCache[bitmask];
}