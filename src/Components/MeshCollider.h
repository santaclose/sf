#pragma once

#include <Components/Transform.h>
#include <Components/SphereCollider.h>
#include <MeshData.h>

namespace sf
{
	struct MeshCollider
	{
		const MeshData* meshData;
		SphereCollider boundingSphere;

		MeshCollider(const MeshData* meshData)
		{
			this->meshData = meshData;

			glm::vec3 min, max;
			for (uint32_t j = 0; j < meshData->indexCount; j++)
			{
				glm::vec3* vertexPos = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 0]);
				if (j == 0)
				{
					min = max = *vertexPos;
					continue;
				}
				if (vertexPos->x > max.x)
					max.x = vertexPos->x;
				if (vertexPos->y > max.y)
					max.y = vertexPos->y;
				if (vertexPos->z > max.z)
					max.z = vertexPos->z;
				if (vertexPos->x < min.x)
					min.x = vertexPos->x;
				if (vertexPos->y < min.y)
					min.y = vertexPos->y;
				if (vertexPos->z < min.z)
					min.z = vertexPos->z;
			}
			boundingSphere.center = (min + max) * 0.5f;
			boundingSphere.radius = glm::length(boundingSphere.center - max);
		}
	};
}