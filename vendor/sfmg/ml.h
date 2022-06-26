#pragma once

#include <glad/glad.h>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <vl.h>

#include <glm/glm.hpp>

namespace sfmg
{
	namespace ml
	{
		struct faceS
		{
			std::vector<uint32_t> verts;
			vl::vec normal;
		};

		struct Vertex
		{
			glm::vec3 position = { 0.0f, 0.0f, 0.0f };
			glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
			glm::vec2 textureCoord = { 0.0f, 0.0f };
		};

		void Initialize(bool smooth);
		void GetData(Vertex*& vertexBuffer, uint32_t& vertexCount, const uint32_t*& indexBuffer, uint32_t& indexCount);

		//void generateFace(faceS& theFace);
		//inline ::ofv::vl::vec calcNormal(faceS& theFace);
		uint32_t vertex(float x, float y, float z, float u = 0.0f, float v = 0.0f);
		uint32_t vertex(const vl::vec& pos, float u = 0.0f, float v = 0.0f);
		void face(uint32_t* ids, int length);
		void face(uint32_t* ids, int length, bool invert);
		void face(uint32_t* ids, int length, int start);
		void face(uint32_t* ids, int length, int start, bool invert);
		void faceSeq(uint32_t* ids, int count, int vertsPerFace);
	}
}