#pragma once

#include <vector>
#include <string>

#include <MeshData.h>
#include <Bitmap.h>
#include <SkeletonData.h>

namespace sf::GltfImporter {

	int Load(const std::string& filePath);
	void Destroy(int id);

	void GenerateMeshData(int id, MeshData& meshData);
	bool GenerateBitmap(int id, int index, Bitmap& bitmap);
	bool GenerateSkeleton(int id, SkeletonData& skeleton, int index = 0);
}