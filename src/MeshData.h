#pragma once

#include <vector>
#include <string>
#include <BufferLayout.h>

namespace sf {

	struct MeshData
	{
		BufferLayout vertexLayout = BufferLayout({
			BufferComponent::VertexPosition,
			BufferComponent::VertexNormal,
			BufferComponent::VertexTangent,
			BufferComponent::VertexColor,
			BufferComponent::VertexUV,
			BufferComponent::VertexAO
		});

		void* vertexBuffer = nullptr;
		uint32_t vertexCount = 0;

		std::vector<uint32_t> indexVector;
		std::vector<uint32_t> pieces;

		void ChangeVertexLayout(const BufferLayout& newLayout);
		bool Initialized() { return vertexBuffer != nullptr; }

		void SaveToFile(const char* targetFile);
		bool LoadFromFile(const char* targetFile);
	};
}