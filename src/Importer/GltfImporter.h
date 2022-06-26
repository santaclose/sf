#pragma once

#include <vector>
#include <string>

#include <MeshData.h>
#include <Bitmap.h>

namespace sf::GltfImporter {

	int Load(const std::string& filePath);
	void Destroy(int id);
	void GenerateMeshData(int id, MeshData& meshData);
	void GenerateBitmap(int id, int index, Bitmap& bitmap);
}