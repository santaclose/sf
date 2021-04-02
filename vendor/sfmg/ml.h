#pragma once

#include <glad/glad.h>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <vl.h>
#include "../../src/Vertex.h"

namespace sfmg
{
	namespace ml
	{
		struct faceS
		{
			std::vector<unsigned int> verts;
			vl::vec normal;
		};

		void Initialize(bool smooth, std::vector<sf::Vertex>* theVertices, std::vector<unsigned int>* theIndexList);

		//void generateFace(faceS& theFace);
		//inline ::ofv::vl::vec calcNormal(faceS& theFace);
		unsigned int vertex(float x, float y, float z, float u = 0.0f, float v = 0.0f);
		unsigned int vertex(vl::vec& pos, float u = 0.0f, float v = 0.0f);
		unsigned int vertex(vl::vec&& pos, float u = 0.0f, float v = 0.0f);
		void face(unsigned int* ids, int length);
		void face(unsigned int* ids, int length, bool invert);
		void face(unsigned int* ids, int length, int start);
		void face(unsigned int* ids, int length, int start, bool invert);
		void faceSeq(unsigned int* ids, int count, int vertsPerFace);
	}
}