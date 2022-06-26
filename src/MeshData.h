#pragma once

#include <vector>
#include <string>
#include <DataLayout.h>

namespace sf {

	struct MeshData
	{
		static std::string vertexPositionAttr;
		static std::string vertexNormalAttr;
		static std::string vertexTangentAttr;
		static std::string vertexBitangentAttr;
		static std::string vertexUvsAttr;
		static std::string vertexColorAttr;
		static std::string vertexAoAttr;

		DataLayout vertexLayout = DataLayout({
			{"pos", DataType::vec3f32},
			{"nor", DataType::vec3f32},
			{"tan", DataType::vec3f32},
			{"bit", DataType::vec3f32},
			{"col", DataType::vec3f32},
			{"uv", DataType::vec2f32},
			{"ao", DataType::f32}
		});
		void* vertexBuffer = nullptr;
		uint32_t vertexCount = 0;

		std::vector<uint32_t> indexVector;
		std::vector<uint32_t> pieces;

		void ChangeVertexLayout(const DataLayout& newLayout);
	};

}