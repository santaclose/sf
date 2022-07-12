#include "SkinnedMesh.h"

sf::SkinnedMesh::SkinnedMesh(const MeshData* meshData, const SkeletonData* skeletonData) : Mesh(meshData)
{
	this->skeletonData = skeletonData;
}
