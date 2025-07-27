#pragma once

#include <Components/Transform.h>
#include <MeshData.h>

namespace sf
{
	struct MeshCollider
	{
		const MeshData* meshData;
		float boundingSphereRadius;

		MeshCollider(const MeshData* meshData)
		{
			this->meshData = meshData;

			boundingSphereRadius = 0.0f;
			for (uint32_t j = 0; j < meshData->indexVector.size(); j++)
			{
				glm::vec3 vertexPos = *((glm::vec3*)meshData->vertexLayout.Access(meshData->vertexBuffer, BufferComponent::VertexPosition, meshData->indexVector[j + 0]));
				float length2 = glm::dot(vertexPos, vertexPos);
				boundingSphereRadius = length2 > boundingSphereRadius ? length2 : boundingSphereRadius;
			}
			boundingSphereRadius = glm::sqrt(boundingSphereRadius);
		}
	};
}