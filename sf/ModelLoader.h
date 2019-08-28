#pragma once
#include <vector>
#include <string>
#include "Vertex.h"

namespace ModelLoader
{
	void LoadOBJFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor);
	void LoadFacetedOBJFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor);
}