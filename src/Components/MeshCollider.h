#pragma once

#include <Components/Transform.h>
#include <Components/SphereCollider.h>
#include <MeshData.h>
#include <MeshProcessor.h>

#include <glm/gtx/norm.hpp>

namespace sf
{
	struct MeshCollider
	{
		const MeshData* meshData;
		SphereCollider boundingSphere;

		MeshCollider(const MeshData* meshData)
		{
			assert(meshData != nullptr);
			this->meshData = meshData;
			MeshProcessor::ComputeMeshBoundingSphere(*meshData, this->boundingSphere);
		}
	};
}