#pragma once

#include <vector>
#include <string>

#include <Renderer/Vertex.h>
#include <Components/Mesh.h>

namespace sf::ObjImporter
{
	int Load(const std::string& filePath);
	void Destroy(int id);
	void GetMesh(int id, Mesh& mesh);
	//void Texture(int id, int textureIndex, unsigned int& glId, int& width, int& height);
}