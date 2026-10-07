#include "VoxelVolumeData.h"

#include <Geometry/Geometry.h>
#include <Math.hpp>

void sf::VoxelVolumeData::BuildEmpty(const glm::uvec3& voxelCountPerAxis, Buffer::Layout voxelBufferLayout, float voxelSize, const glm::vec3& offset)
{
	this->voxelBufferLayout = voxelBufferLayout;
	this->voxelSize = voxelSize;
	this->offset = offset;
	this->voxelCountPerAxis = voxelCountPerAxis;
}

void sf::VoxelVolumeData::BuildFromMesh(const MeshData& mesh, float voxelSize, Buffer::Layout voxelBufferLayout)
{
	float voxelBoundingSphereRadius = glm::sqrt(voxelSize * voxelSize * 3) / 2.0f;

	this->voxelBufferLayout = voxelBufferLayout;
	const Buffer::LayoutInfo& layout = voxelBufferLayout.GetInfo();

	DataType positionDataType = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Position)->dataType;
	assert(positionDataType == DataType::vec3f32);
	assert(mesh.vertexCount > 0);
	assert(voxelSize > 0.0f);

	this->voxelSize = voxelSize;

	// compute mesh AABB
	glm::vec3 minP = *mesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, 0);
	glm::vec3 maxP = minP;
	for (int i = 1; i < mesh.vertexCount; i++)
	{
		const glm::vec3* posPtr = mesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, i);
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
	std::unordered_map<uint32_t, uint32_t> voxelCollisions;
	for (int indexI = 0; indexI < mesh.indexCount; indexI += 3)
	{
		uint32_t indexA = mesh.indexBuffer[indexI + 0];
		uint32_t indexB = mesh.indexBuffer[indexI + 1];
		uint32_t indexC = mesh.indexBuffer[indexI + 2];

		const glm::vec3* posPtrA = mesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, indexA);
		const glm::vec3* posPtrB = mesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, indexB);
		const glm::vec3* posPtrC = mesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, indexC);
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

					/* Approximate intersection using voxel bounding sphere */
					glm::vec3 voxelCenterOnTriangle = Geometry::ClosestPointPointTriangle(currentVoxelCenter, *posPtrA, *posPtrB, *posPtrC);
					bool shouldFill = glm::distance2(voxelCenterOnTriangle, currentVoxelCenter) < voxelBoundingSphereRadius * voxelBoundingSphereRadius;

					if (!shouldFill)
						continue;

					bool voxelExists = !CreateVoxel(currentVoxel);
					if (voxelBufferLayout == 0u)
						continue;
					if (voxelExists)
					{
						uint32_t existingVoxel = map[currentVoxel];
						if (voxelCollisions.find(existingVoxel) == voxelCollisions.end())
							voxelCollisions[existingVoxel] = 2;
						else
							voxelCollisions[existingVoxel]++;
					}
					for (const auto& [component, componentInfo] : layout.components)
					{
						if (component == Buffer::Component::Position)
						{
							glm::vec3* voxelPosPointer = AccessVoxelComponent<glm::vec3>(Buffer::Component::Position, currentVoxel);
							if (voxelPosPointer != nullptr)
								*voxelPosPointer = currentVoxelCenter;
							continue;
						}

						/* Source mesh doesn't have this component */
						if (mesh.vertexBufferLayout.GetComponentInfo(component) == nullptr)
							continue;

						/* We only support copying these */
						if (component != Buffer::Component::Normal &&
							component != Buffer::Component::Color &&
							component != Buffer::Component::UV)
							continue;

						glm::vec2 toBlendVec2[3];
						glm::vec3 toBlendVec3[3];
						glm::vec2 blendVec2Out;
						glm::vec3 blendVec3Out;
						glm::vec3 barycentricCoords = Geometry::Barycentric(voxelCenterOnTriangle, *posPtrA, *posPtrB, *posPtrC);

						switch (componentInfo.dataType)
						{
							case DataType::vec2f32:
								toBlendVec2[0] = *mesh.ReadVertexComponent<glm::vec2>(component, indexA);
								toBlendVec2[1] = *mesh.ReadVertexComponent<glm::vec2>(component, indexB);
								toBlendVec2[2] = *mesh.ReadVertexComponent<glm::vec2>(component, indexC);
								Math::WeightedBlend(toBlendVec2, &barycentricCoords.x, 3, blendVec2Out);
								*AccessVoxelComponent<glm::vec2>(component, currentVoxel) += blendVec2Out;
								break;
							case DataType::vec3f32:
								toBlendVec3[0] = *mesh.ReadVertexComponent<glm::vec3>(component, indexA);
								toBlendVec3[1] = *mesh.ReadVertexComponent<glm::vec3>(component, indexB);
								toBlendVec3[2] = *mesh.ReadVertexComponent<glm::vec3>(component, indexC);
								Math::WeightedBlend(toBlendVec3, &barycentricCoords.x, 3, blendVec3Out);
								*AccessVoxelComponent<glm::vec3>(component, currentVoxel) += blendVec3Out;
								break;
						}
					}
				}
	}

	for (const auto& pair : voxelCollisions)
	{
		for (const auto& [component, componentInfo] : layout.components)
		{
			if (component == Buffer::Component::Position)
				continue;
			switch (componentInfo.dataType)
			{
				case DataType::vec2f32:
					*(voxelBufferLayout.Access<glm::vec2>(voxelBuffer.data(), component, pair.first)) /= (float) pair.second;
					break;
				case DataType::vec3f32:
					if (component == Buffer::Component::Normal)
					{
						glm::vec3& temp = *(voxelBufferLayout.Access<glm::vec3>(voxelBuffer.data(), component, pair.first));
						temp = glm::normalize(temp);
					}
					else
						*(voxelBufferLayout.Access<glm::vec3>(voxelBuffer.data(), component, pair.first)) /= (float) pair.second;
					break;
			}
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
	Geometry::RayHit rh;
	if (!Geometry::IntersectRayAABB(origin, dir, bbmin, bbmax, &rh))
		return nullptr;

	/* Origin was inside aabb */
	if (rh.distance == 0.0f)
	{
		currentVoxel = {
			(origin.x - bbmin.x) / voxelSize,
			(origin.y - bbmin.y) / voxelSize,
			(origin.z - bbmin.z) / voxelSize };
	}
	else
	{
		/* Move origin to bounding box */
		currentVoxel = {
			(rh.point.x - bbmin.x) / voxelSize,
			(rh.point.y - bbmin.y) / voxelSize,
			(rh.point.z - bbmin.z) / voxelSize
		};
		if (currentVoxel.x < 0) currentVoxel.x = 0;
		if (currentVoxel.x > voxelCountPerAxis.x - 1) currentVoxel.x = voxelCountPerAxis.x - 1;
		if (currentVoxel.y < 0) currentVoxel.y = 0;
		if (currentVoxel.y > voxelCountPerAxis.y - 1) currentVoxel.y = voxelCountPerAxis.y - 1;
		if (currentVoxel.z < 0) currentVoxel.z = 0;
		if (currentVoxel.z > voxelCountPerAxis.z - 1) currentVoxel.z = voxelCountPerAxis.z - 1;
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
	}
}