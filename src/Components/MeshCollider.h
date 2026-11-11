#pragma once

#include <Components/Transform.h>
#include <Components/SphereCollider.h>
#include <MeshData.h>

namespace sf
{
	struct MeshCollider
	{
		const MeshData* meshData;

		MeshCollider(const MeshData* meshData)
		{
			assert(meshData != nullptr);
			this->meshData = meshData;
		}

		inline const SphereCollider& GetBoundingSphere() const { return meshData->GetBoundingSphere(); }
	};
}