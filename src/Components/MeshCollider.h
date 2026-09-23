#pragma once

#include <Components/Transform.h>
#include <Components/SphereCollider.h>
#include <MeshData.h>

#include <glm/gtx/norm.hpp>

namespace sf
{
	struct MeshCollider
	{
		const MeshData* meshData;
		SphereCollider boundingSphere;

		MeshCollider(const MeshData* meshData)
		{
			this->meshData = meshData;

			glm::vec3 min(std::numeric_limits<float>::max());
			glm::vec3 max(-std::numeric_limits<float>::max());

			for (uint32_t i = 0; i < meshData->vertexCount; i++)
			{
				const glm::vec3* vertexPos = meshData->ReadVertexComponent<glm::vec3>(BufferComponent::Position, i);

				min = glm::min(min, *vertexPos);
				max = glm::max(max, *vertexPos);
			}

			boundingSphere.center = (min + max) * 0.5f;

			float maxDistanceSq = 0.0f;
			for (uint32_t i = 0; i < meshData->vertexCount; i++)
			{
				const glm::vec3* vertexPos = meshData->ReadVertexComponent<glm::vec3>(BufferComponent::Position, i);
				float distSq = glm::distance2(boundingSphere.center, *vertexPos);
				if (distSq > maxDistanceSq)
					maxDistanceSq = distSq;
			}

			boundingSphere.radius = glm::sqrt(maxDistanceSq);
		}
	};
}