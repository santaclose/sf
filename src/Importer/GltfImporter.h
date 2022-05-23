#pragma once

#include <vector>
#include <string>

#include <Renderer/Vertex.h>
#include <MeshData.h>

namespace sf::GltfImporter {

	int Load(const std::string& filePath);
	void Destroy(int id);
	void GenerateMeshData(int id, MeshData& meshData);
	void GetTexture(int id, int textureIndex, unsigned int& glId, int& width, int& height);
}