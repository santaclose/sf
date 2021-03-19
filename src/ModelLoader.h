#pragma once
#include <vector>
#include <string>
#include "Vertex.h"

class ModelLoader
{
public:
	static void LoadGltfFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor, bool isBinary);
};