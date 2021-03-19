#pragma once
#include <vector>
#include <string>
#include "Vertex.h"

class ModelLoader
{
private:
	static void LoadFacetedObjFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor);
public:
	static void LoadObjFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor, bool smooth = true);
	static void LoadGltfFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath);
};