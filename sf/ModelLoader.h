#pragma once
#include <vector>
#include <string>
#include "Vertex.h"

class ModelLoader
{
public:
	static void LoadOBJFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor);
	static void LoadFacetedOBJFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor);

	static void LoadGLTF(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath);
};