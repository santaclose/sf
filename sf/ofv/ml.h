#pragma once

#include <glad/glad.h>
#include "vl.h"
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "../Vertex.h"

namespace ofv
{
	namespace ml
	{
		struct faceS
		{
			std::vector<unsigned int> verts;
			::ofv::vl::vec normal;
		};

		void Initialize(bool smooth, std::vector<Vertex>* theVertices, std::vector<unsigned int>* theIndexList);

		//void generateFace(faceS& theFace);
		//inline ::ofv::vl::vec calcNormal(faceS& theFace);
		unsigned int vertex(float x, float y, float z, float u = 0.0f, float v = 0.0f);
		unsigned int vertex(::ofv::vl::vec& pos, float u = 0.0f, float v = 0.0f);
		unsigned int vertex(::ofv::vl::vec&& pos, float u = 0.0f, float v = 0.0f);
		void face(unsigned int* ids, int length);
		void face(unsigned int* ids, int length, bool invert);
		void face(unsigned int* ids, int length, int start);
		void face(unsigned int* ids, int length, int start, bool invert);
		void faceSeq(unsigned int* ids, int count, int vertsPerFace);
	}
}