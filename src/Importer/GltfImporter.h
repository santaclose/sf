#pragma once

#include <vector>
#include <string>

#include <MeshData.h>
#include <Bitmap.h>
#include <SkeletonData.h>

namespace sf::GltfImporter {

	int Load(const std::string& filePath);
	void Destroy(int id);

	bool GenerateMeshData(int id, const BufferLayout& vertexLayout, MeshData& meshData);
	bool GenerateBitmap(int id, int index, Bitmap& bitmap);
	bool GenerateSkeleton(int id, SkeletonData& skeleton, int index = 0);
}