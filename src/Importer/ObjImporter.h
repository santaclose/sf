#pragma once

#include <vector>
#include <string>

#include <MeshData.h>

namespace sf::ObjImporter
{
	int Load(const std::string& filePath);
	void Destroy(int id);
	void GenerateMeshData(int id, MeshData& mesh);
	void FreeMeshData(MeshData& mesh);
}