#pragma once

#include <vector>
#include <string>
#include <BufferLayout.h>

namespace sf {

	struct MeshData
	{
		BufferLayout vertexBufferLayout;

		void* vertexBuffer = nullptr;
		uint32_t vertexCount = 0;
		uint32_t* indexBuffer = nullptr;
		uint32_t indexCount = 0;
		uint32_t* pieces = nullptr;
		uint32_t pieceCount = 0;

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