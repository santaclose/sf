#pragma once

#include <MeshData.h>
#include <SkeletonData.h>

#include <Components/Mesh.h>

namespace sf {

	struct SkinnedMesh : public Mesh
	{
	public:
		const SkeletonData* skeletonData = nullptr;
		SkinnedMesh(const MeshData* meshData, const SkeletonData* skeletonData);
	};
}