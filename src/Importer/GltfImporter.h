#pragma once

#include <vector>
#include <string>

#include <Renderer/Vertex.h>
#include <MeshData.h>
#include <Renderer/Texture.h>

namespace sf::GltfImporter {

	int Load(const std::string& filePath);
	void Destroy(int id);
	void GenerateMeshData(int id, MeshData& meshData);
	void GenerateTexture(int id, int textureIndex, Texture& texture);
}