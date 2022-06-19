#pragma once

#include <vector>
#include <string>

#include <Renderer/Vertex.h>

namespace sf {

	struct MeshData
	{
		std::vector<Vertex> vertexVector;
		std::vector<unsigned int> indexVector;
		std::vector<unsigned int> pieces;
	};
}