#pragma once

#include <vector>
#include <string>
#include <BufferLayout.h>

namespace sf {

	struct MeshData
	{
		void* vertexBuffer = nullptr;
		BufferLayout vertexBufferLayout = BufferLayout({
			BufferComponent::VertexPosition,
			BufferComponent::VertexNormal,
			BufferComponent::VertexTangent,
			BufferComponent::VertexColor,
			BufferComponent::VertexUV,
			BufferComponent::VertexAO
		});

		uint32_t vertexCount = 0;

		std::vector<uint32_t> indexVector;
		std::vector<uint32_t> pieces;

		void ChangeVertexBufferLayout(const BufferLayout& newLayout);
		bool Initialized() { return vertexBuffer != nullptr; }

		inline void* AccessVertexComponent(BufferComponent component, uint32_t index) const
		{
			return vertexBufferLayout.Access(vertexBuffer, component, index);
		}

		void SaveToFile(const char* targetFile);
		bool LoadFromFile(const char* targetFile);
	};
}