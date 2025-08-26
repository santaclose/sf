#pragma once

#include <vector>
#include <string>
#include <BufferLayout.h>

namespace sf {

	struct MeshData
	{
		void* vertexBuffer = nullptr;
		BufferLayout vertexBufferLayout;

		uint32_t vertexCount = 0;

		std::vector<uint32_t> indexVector;
		std::vector<uint32_t> pieces;

		MeshData() = default;
		inline MeshData(const BufferLayout& newLayout)
		{
			vertexBufferLayout = newLayout;
		}

		void ChangeVertexBufferLayout(const BufferLayout& newLayout);
		bool Initialized() { return vertexBuffer != nullptr; }

		template<typename T>
		inline T* AccessVertexComponent(BufferComponent component, uint32_t index) const
		{
			return vertexBufferLayout.Access<T>(vertexBuffer, component, index);
		}

		void SaveToFile(const char* targetFile);
		bool LoadFromFile(const char* targetFile);
	};
}