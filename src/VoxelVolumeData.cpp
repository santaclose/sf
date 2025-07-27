#include "VoxelVolumeData.h"

#include <Geometry.h>

void sf::VoxelVolumeData::BuildEmpty(const glm::uvec3& voxelCountPerAxis, const BufferLayout* voxelBufferLayout, float voxelSize, const glm::vec3& offset)
{
	if (voxelBufferLayout != nullptr)
		this->voxelBufferLayout = *voxelBufferLayout;
	this->voxelSize = voxelSize;
	this->offset = offset;
	this->voxelCountPerAxis = voxelCountPerAxis;
}

void sf::VoxelVolumeData::BuildFromMesh(const MeshData& mesh, float voxelSize, const BufferLayout* voxelBufferLayout)
{
	if (voxelBufferLayout != nullptr)
		this->voxelBufferLayout = *voxelBufferLayout;
	DataType positionDataType = mesh.vertexLayout.GetComponentInfo(BufferComponent::VertexPosition)->dataType;
	assert(positionDataType == DataType::vec3f32);
	assert(mesh.vertexCount > 0);
	assert(voxelSize > 0.0f);

	this->voxelSize = voxelSize;

	// compute mesh AABB
	glm::vec3 minP = *((glm::vec3*)mesh.vertexLayout.Access(mesh.vertexBuffer, BufferComponent::VertexPosition, 0));
	glm::vec3 maxP = minP;
	for (int i = 1; i < mesh.vertexCount; i++)
	{
		glm::vec3* posPtr = (glm::vec3*)mesh.vertexLayout.Access(mesh.vertexBuffer, BufferComponent::VertexPosition, i);
		minP.x = glm::min(minP.x, posPtr->x);
		minP.y = glm::min(minP.y, posPtr->y);
		minP.z = glm::min(minP.z, posPtr->z);
		maxP.x = glm::max(maxP.x, posPtr->x);
		maxP.y = glm::max(maxP.y, posPtr->y);
		maxP.z = glm::max(maxP.z, posPtr->z);
	}

	this->offset = minP;

	voxelCountPerAxis = {
		(uint32_t)glm::ceil((maxP.x - minP.x) / voxelSize),
		(uint32_t)glm::ceil((maxP.y - minP.y) / voxelSize),
		(uint32_t)glm::ceil((maxP.z - minP.z) / voxelSize)
	};

	// voxelize
	for (int indexI = 0; indexI < mesh.indexVector.size(); indexI += 3)
	{
		uint32_t indexA = mesh.indexVector[indexI + 0];
		uint32_t indexB = mesh.indexVector[indexI + 1];
		uint32_t indexC = mesh.indexVector[indexI + 2];

		glm::vec3* posPtrA = (glm::vec3*) mesh.vertexLayout.Access(mesh.vertexBuffer, BufferComponent::VertexPosition, indexA);
		glm::vec3* posPtrB = (glm::vec3*) mesh.vertexLayout.Access(mesh.vertexBuffer, BufferComponent::VertexPosition, indexB);
		glm::vec3* posPtrC = (glm::vec3*) mesh.vertexLayout.Access(mesh.vertexBuffer, BufferComponent::VertexPosition, indexC);
		glm::vec3 triNormal = glm::cross(
			*posPtrB - *posPtrA,
			*posPtrC - *posPtrA);

		glm::vec3 trianglebbmin = glm::min(glm::min(*posPtrA, *posPtrB), *posPtrC);
		glm::vec3 trianglebbmax = glm::max(glm::max(*posPtrA, *posPtrB), *posPtrC);
		glm::uvec3 minVoxelCoords = {
			glm::clamp((int)((trianglebbmin.x - minP.x) / voxelSize), 0, (int)(voxelCountPerAxis.x - 1)),
			glm::clamp((int)((trianglebbmin.y - minP.y) / voxelSize), 0, (int)(voxelCountPerAxis.y - 1)),
			glm::clamp((int)((trianglebbmin.z - minP.z) / voxelSize), 0, (int)(voxelCountPerAxis.z - 1))
		};
		glm::uvec3 maxVoxelCoords = {
			glm::clamp((int)((trianglebbmax.x - minP.x) / voxelSize) + 1, 0, (int)(voxelCountPerAxis.x - 1)),
			glm::clamp((int)((trianglebbmax.y - minP.y) / voxelSize) + 1, 0, (int)(voxelCountPerAxis.y - 1)),
			glm::clamp((int)((trianglebbmax.z - minP.z) / voxelSize) + 1, 0, (int)(voxelCountPerAxis.z - 1))
		};

		glm::uvec3 currentVoxel;
		for (currentVoxel.x = minVoxelCoords.x; currentVoxel.x < maxVoxelCoords.x; currentVoxel.x++)
			for (currentVoxel.y = minVoxelCoords.y; currentVoxel.y < maxVoxelCoords.y; currentVoxel.y++)
				for (currentVoxel.z = minVoxelCoords.z; currentVoxel.z < maxVoxelCoords.z; currentVoxel.z++)
				{
					glm::vec3 currentVoxelMin = offset + glm::vec3(currentVoxel) * voxelSize;
					glm::vec3 currentVoxelMax = currentVoxelMin + glm::vec3(voxelSize, voxelSize, voxelSize);
					glm::vec3 currentVoxelCenter = (currentVoxelMin + currentVoxelMax) / 2.0f;
					// approximation is good and fast
					bool shouldFill = glm::distance2(Geometry::ClosestPointPointTriangle(currentVoxelCenter, *posPtrA, *posPtrB, *posPtrC), currentVoxelCenter) < voxelSize * voxelSize;
					// bool shouldFill = Geometry::IntersectAABBTriangle(currentVoxelMin, currentVoxelMax, *posPtrA, *posPtrB, *posPtrC);

					if (!shouldFill)
						continue;

					SetVoxel(currentVoxel, &currentVoxelCenter);
				}
	}
}

// https://www.researchgate.net/publication/2611491_A_Fast_Voxel_Traversal_Algorithm_for_Ray_Tracing
void* sf::VoxelVolumeData::CastRay(const glm::vec3& origin, const glm::vec3& direction, bool avoidEarlyCollision, float* out_t) const
{
	glm::vec3 dir = glm::normalize(direction);
	if (dir.x == 0.0f) dir.x = 0.00000001f;
	if (dir.y == 0.0f) dir.y = 0.00000001f;
	if (dir.z == 0.0f) dir.z = 0.00000001f;

	glm::ivec3 step = {
		dir.x > 0.0f ? 1 : -1,
		dir.y > 0.0f ? 1 : -1,
		dir.z > 0.0f ? 1 : -1
	};

	glm::vec3 bbmin = GetAABBMin();
	glm::vec3 bbmax = GetAABBMax();

	glm::uvec3 currentVoxel;
	if (!Geometry::IntersectPointAABB(origin, bbmin, bbmax))
	{
		glm::vec3 point;
		// move origin to bounding box
		if (!Geometry::IntersectRayAABB(origin, dir, bbmin, bbmax, &point))
			return nullptr;

		currentVoxel = {
			(point.x - bbmin.x) / voxelSize,
			(point.y - bbmin.y) / voxelSize,
			(point.z - bbmin.z) / voxelSize
		};
		if (currentVoxel.x < 0) currentVoxel.x = 0;
		if (currentVoxel.x > voxelCountPerAxis.x - 1) currentVoxel.x = voxelCountPerAxis.x - 1;
		if (currentVoxel.y < 0) currentVoxel.y = 0;
		if (currentVoxel.y > voxelCountPerAxis.y - 1) currentVoxel.y = voxelCountPerAxis.y - 1;
		if (currentVoxel.z < 0) currentVoxel.z = 0;
		if (currentVoxel.z > voxelCountPerAxis.z - 1) currentVoxel.z = voxelCountPerAxis.z - 1;
	}
	else
	{
		currentVoxel = {
			(origin.x - bbmin.x) / voxelSize,
			(origin.y - bbmin.y) / voxelSize,
			(origin.z - bbmin.z) / voxelSize
		};
	}

	glm::vec3 currentVoxelCenter = GetVoxelCenterLocation(currentVoxel);
	glm::vec3 nextBoundaries = {
		currentVoxelCenter.x + (step.x * voxelSize / 2.0f),
		currentVoxelCenter.y + (step.y * voxelSize / 2.0f),
		currentVoxelCenter.z + (step.z * voxelSize / 2.0f)
	};

	glm::vec3 tMax = {
		(nextBoundaries.x - origin.x) / dir.x,
		(nextBoundaries.y - origin.y) / dir.y,
		(nextBoundaries.z - origin.z) / dir.z,
	};

	glm::vec3 tDelta{
		voxelSize / dir.x,
		voxelSize / dir.y,
		voxelSize / dir.z
	};

	bool inAir = !avoidEarlyCollision;
	while (true)
	{
		if (tMax.x < tMax.y)
		{
			if (tMax.x < tMax.z)
			{
				currentVoxel.x = currentVoxel.x + step.x;
				if (currentVoxel.x > voxelCountPerAxis.x - 1) return nullptr;
				tMax.x = tMax.x + glm::abs(tDelta.x);
			}
			else
			{
				currentVoxel.z = currentVoxel.z + step.z;
				if (currentVoxel.z > voxelCountPerAxis.z - 1) return nullptr;
				tMax.z = tMax.z + glm::abs(tDelta.z);
			}
		}
		else
		{
			if (tMax.y < tMax.z)
			{
				currentVoxel.y = currentVoxel.y + step.y;
				if (currentVoxel.y > voxelCountPerAxis.y - 1) return nullptr;
				tMax.y = tMax.y + glm::abs(tDelta.y);
			}
			else
			{
				currentVoxel.z = currentVoxel.z + step.z;
				if (currentVoxel.z > voxelCountPerAxis.z - 1) return nullptr;
				tMax.z = tMax.z + glm::abs(tDelta.z);
			}
		}
		if (!GetVoxel(currentVoxel) && !inAir)
			inAir = true;

		if (GetVoxel(currentVoxel) && inAir)
		{
			if (out_t != nullptr)
				*out_t = glm::distance(GetVoxelCenterLocation(currentVoxel), origin);
			return GetVoxel(currentVoxel);
		}

		// if (draw)
		// {
			// glm::vec3 voxelPos(currentVoxel);
			// voxelPos *= voxelSize;
			// voxelPos += offset;
			// SetVoxel(currentVoxel, &voxelPos);
		// }
	}
}