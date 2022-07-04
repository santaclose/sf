#pragma once

#include <vector>
#include <string>
#include <DataLayout.h>

namespace sf {

	struct MeshData
	{
		enum VertexAttribute
		{
			Position = 0,
			Normal = 1,
			Tangent = 2,
			Bitangent = 3,
			Color = 4,
			UV = 5,
			AO = 6
		};
		static std::string vertexPositionAttr;
		static std::string vertexNormalAttr;
		static std::string vertexTangentAttr;
		static std::string vertexBitangentAttr;
		static std::string vertexUvsAttr;
		static std::string vertexColorAttr;
		static std::string vertexAoAttr;

		DataLayout vertexLayout = DataLayout({
			{VertexAttribute::Position, DataType::vec3f32},
			{VertexAttribute::Normal, DataType::vec3f32},
			{VertexAttribute::Tangent, DataType::vec3f32},
			{VertexAttribute::Bitangent, DataType::vec3f32},
			{VertexAttribute::Color, DataType::vec3f32},
			{VertexAttribute::UV, DataType::vec2f32},
			{VertexAttribute::AO, DataType::f32}
		});
		void* vertexBuffer = nullptr;
		uint32_t vertexCount = 0;

		std::vector<uint32_t> indexVector;
		std::vector<uint32_t> pieces;

		void ChangeVertexLayout(const DataLayout& newLayout);
	};

}