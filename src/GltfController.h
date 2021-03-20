#pragma once
#include <vector>
#include <string>
#include "Vertex.h"

namespace GltfController
{
	int Load(const std::string& filePath);
	void Destroy(int id);
	void Model(int id, int meshIndex, std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector);
	void Texture(int id, int textureIndex, unsigned int& glId, int& width, int& height);
};