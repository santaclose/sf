#pragma once

#include <vector>
#include <string>

#include <Vertex.h>
#include <MeshPiece.h>

namespace sf::GltfImporter {

	int Load(const std::string& filePath);
	void Destroy(int id);
	void GetMesh(int id, std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, std::vector<MeshPiece>& pieces);
	void GetTexture(int id, int textureIndex, unsigned int& glId, int& width, int& height);
}