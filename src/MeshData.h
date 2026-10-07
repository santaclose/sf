#pragma once

#include <vector>
#include <string>
#include <BufferLayout.h>

namespace sf {

	struct MeshData
	{
		Buffer::Layout vertexBufferLayout;

		uint8_t* vertexBuffer = nullptr;
		uint32_t vertexCount = 0;
		uint32_t* indexBuffer = nullptr;
		uint32_t indexCount = 0;
		uint32_t* pieces = nullptr;
		uint32_t pieceCount = 0;
		uint8_t vertexCountPerPrimitive = 3;
		bool usesExternalVertexBuffer = false;

		MeshData() = default;
		void Create(
			Buffer::Layout vertexBufferLayout,
			uint8_t* vertexBuffer,
			uint32_t vertexCount,
			const std::vector<uint32_t>& indexVector,
			const std::vector<uint32_t>* pieceVector = nullptr,
			uint8_t vertexCountPerPrimitive = 3);
		void Destroy();

		void ChangeVertexBufferLayout(Buffer::Layout newLayout);
		bool Initialized() { return vertexBuffer != nullptr; }

		template<typename T>
		inline T* AccessVertexComponent(Buffer::Component component, uint32_t index)
		{
			return vertexBufferLayout.Access<T>(vertexBuffer, component, index);
		}

		template<typename T>
		inline const T* ReadVertexComponent(Buffer::Component component, uint32_t index) const
		{
			return vertexBufferLayout.Access<T>(vertexBuffer, component, index);
		}

		bool SaveToFile(const char* targetFile);
		bool LoadFromFile(const char* targetFile);
	};
}