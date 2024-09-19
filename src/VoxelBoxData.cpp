#include "VoxelBoxData.h"

#include <Math.hpp>

sf::VoxelBoxData::VoxelBoxData(uint32_t voxelCountX, uint32_t voxelCountY, uint32_t voxelCountZ, float voxelSize, const glm::vec3& offset)
{
	this->voxelSize = voxelSize;
	this->offset = offset;

	mat.resize(voxelCountX);
	for (auto& vec : mat)
	{
		vec.resize(voxelCountY);
		for (auto& deeperVec : vec)
			deeperVec.resize(voxelCountZ);
	}
}

sf::VoxelBoxData::VoxelBoxData(const MeshData& mesh, float voxelSize)
{
	DataType positionDataType = mesh.vertexLayout.GetComponent(VertexAttribute::Position)->dataType;
	assert(positionDataType == DataType::vec3f32);

	const float RASTERIZE_MAX_DISTANCE = 0.7f;

	this->voxelSize = voxelSize;

	glm::vec3 minP = *((glm::vec3*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, 0));
	glm::vec3 maxP = minP;

	for (int i = 1; i < mesh.vertexCount; i++)
	{
		glm::vec3* posPtr = (glm::vec3*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, i);
		minP.x = glm::min(minP.x, posPtr->x);
		minP.y = glm::min(minP.y, posPtr->y);
		minP.z = glm::min(minP.z, posPtr->z);
		maxP.x = glm::max(maxP.x, posPtr->x);
		maxP.y = glm::max(maxP.y, posPtr->y);
		maxP.z = glm::max(maxP.z, posPtr->z);
	}

	this->offset = minP;

	glm::uvec3 voxelCount = {
		(uint32_t)glm::ceil((maxP.x - minP.x) / voxelSize),
		(uint32_t)glm::ceil((maxP.y - minP.y) / voxelSize),
		(uint32_t)glm::ceil((maxP.z - minP.z) / voxelSize)
	};

	// allocate matrix
	mat.resize(voxelCount.x);
	for (auto& vector : mat)
	{
		vector.resize(voxelCount.y);
		for (auto& subvector : vector)
			subvector.resize(voxelCount.z);
	}

	// rasterize
	for (int indexI = 0; indexI < mesh.indexVector.size(); indexI += 3)
	{
		uint32_t indexA = mesh.indexVector[indexI + 0];
		uint32_t indexB = mesh.indexVector[indexI + 1];
		uint32_t indexC = mesh.indexVector[indexI + 2];

		glm::vec3* posPtrA = (glm::vec3*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, indexA);
		glm::vec3* posPtrB = (glm::vec3*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, indexB);
		glm::vec3* posPtrC = (glm::vec3*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, indexC);

		glm::vec3 triNormal = glm::cross(
			*posPtrB - *posPtrA,
			*posPtrC - *posPtrA);

		glm::vec3 trianglebbmin = {
			glm::min(glm::min(posPtrA->x, posPtrB->x), posPtrC->x),
			glm::min(glm::min(posPtrA->y, posPtrB->y), posPtrC->y),
			glm::min(glm::min(posPtrA->z, posPtrB->z), posPtrC->z)
		};
		glm::vec3 trianglebbmax = {
			glm::max(glm::max(posPtrA->x, posPtrB->x), posPtrC->x),
			glm::max(glm::max(posPtrA->y, posPtrB->y), posPtrC->y),
			glm::max(glm::max(posPtrA->z, posPtrB->z), posPtrC->z)
		};
		glm::uvec3 minVoxelCoords = {
			glm::clamp((int)((trianglebbmin.x - minP.x) / voxelSize), 0, (int)(mat.size() - 1)),
			glm::clamp((int)((trianglebbmin.y - minP.y) / voxelSize), 0, (int)(mat[0].size() - 1)),
			glm::clamp((int)((trianglebbmin.z - minP.z) / voxelSize), 0, (int)(mat[0][0].size() - 1))
		};
		glm::uvec3 maxVoxelCoords = {
			glm::clamp((int)((trianglebbmax.x - minP.x) / voxelSize), 0, (int)(mat.size() - 1)),
			glm::clamp((int)((trianglebbmax.y - minP.y) / voxelSize), 0, (int)(mat[0].size() - 1)),
			glm::clamp((int)((trianglebbmax.z - minP.z) / voxelSize), 0, (int)(mat[0][0].size() - 1))
		};

		std::vector<std::vector<bool>> xyMat;
		std::vector<std::vector<bool>> xzMat;
		std::vector<std::vector<bool>> zyMat;
		std::vector<std::vector<std::vector<bool>>> xyzMat;

		xyMat.resize(maxVoxelCoords.x - minVoxelCoords.x + 1);
		for (auto& vec : xyMat)
			vec.resize(maxVoxelCoords.y - minVoxelCoords.y + 1);
		xzMat.resize(maxVoxelCoords.x - minVoxelCoords.x + 1);
		for (auto& vec : xzMat)
			vec.resize(maxVoxelCoords.z - minVoxelCoords.z + 1);
		zyMat.resize(maxVoxelCoords.z - minVoxelCoords.z + 1);
		for (auto& vec : zyMat)
			vec.resize(maxVoxelCoords.y - minVoxelCoords.y + 1);

		for (int i = minVoxelCoords.x; i <= maxVoxelCoords.x; i++)
		{
			for (int j = minVoxelCoords.y; j <= maxVoxelCoords.y; j++)
			{
				glm::vec2 voxelCenter = glm::vec2(minP.x, minP.y) + glm::vec2(i * voxelSize + voxelSize / 2.0f, j * voxelSize + voxelSize / 2.0f);
				glm::vec2 tri0 = glm::vec2(posPtrA->x, posPtrA->y);
				glm::vec2 tri1 = glm::vec2(posPtrB->x, posPtrB->y);
				glm::vec2 tri2 = glm::vec2(posPtrC->x, posPtrC->y);
				xyMat[i - minVoxelCoords.x][j - minVoxelCoords.y] = Math::TriPointDistance2D(voxelCenter, tri0, tri1, tri2) < voxelSize * RASTERIZE_MAX_DISTANCE;
			}
		}

		for (int i = minVoxelCoords.x; i <= maxVoxelCoords.x; i++)
		{
			for (int j = minVoxelCoords.z; j <= maxVoxelCoords.z; j++)
			{
				glm::vec2 voxelCenter = glm::vec2(minP.x, minP.z) + glm::vec2(i * voxelSize + voxelSize / 2.0f, j * voxelSize + voxelSize / 2.0f);
				glm::vec2 tri0 = glm::vec2(posPtrA->x, posPtrA->z);
				glm::vec2 tri1 = glm::vec2(posPtrB->x, posPtrB->z);
				glm::vec2 tri2 = glm::vec2(posPtrC->x, posPtrC->z);
				xzMat[i - minVoxelCoords.x][j - minVoxelCoords.z] = Math::TriPointDistance2D(voxelCenter, tri0, tri1, tri2) < voxelSize * RASTERIZE_MAX_DISTANCE;
			}
		}

		for (int i = minVoxelCoords.z; i <= maxVoxelCoords.z; i++)
		{
			for (int j = minVoxelCoords.y; j <= maxVoxelCoords.y; j++)
			{
				glm::vec2 voxelCenter = glm::vec2(minP.z, minP.y) + glm::vec2(i * voxelSize + voxelSize / 2.0f, j * voxelSize + voxelSize / 2.0f);
				glm::vec2 tri0 = glm::vec2(posPtrA->z, posPtrA->y);
				glm::vec2 tri1 = glm::vec2(posPtrB->z, posPtrB->y);
				glm::vec2 tri2 = glm::vec2(posPtrC->z, posPtrC->y);
				zyMat[i - minVoxelCoords.z][j - minVoxelCoords.y] = Math::TriPointDistance2D(voxelCenter, tri0, tri1, tri2) < voxelSize * RASTERIZE_MAX_DISTANCE;
			}
		}

		for (int i = minVoxelCoords.x; i <= maxVoxelCoords.x; i++)
		{
			for (int j = minVoxelCoords.y; j <= maxVoxelCoords.y; j++)
			{
				for (int k = minVoxelCoords.z; k <= maxVoxelCoords.z; k++)
				{
					glm::vec3 voxelCenter =
						glm::vec3(minP.x, minP.y, minP.z) +
						glm::vec3(i * voxelSize + voxelSize / 2.0f, j * voxelSize + voxelSize / 2.0f, k * voxelSize + voxelSize / 2.0f);

					bool voxelValueForCurrentTri =
						xyMat[i - minVoxelCoords.x][j - minVoxelCoords.y] &&
						xzMat[i - minVoxelCoords.x][k - minVoxelCoords.z] &&
						zyMat[k - minVoxelCoords.z][j - minVoxelCoords.y] &&
						glm::abs(Math::PlanePointDistance(triNormal, *posPtrA, voxelCenter)) < voxelSize * RASTERIZE_MAX_DISTANCE;

					mat[i][j][k] = mat[i][j][k] || voxelValueForCurrentTri;
				}
			}
		}
	}
}

// https://www.researchgate.net/publication/2611491_A_Fast_Voxel_Traversal_Algorithm_for_Ray_Tracing
bool sf::VoxelBoxData::CastRay(const glm::vec3& origin, const glm::vec3& direction, bool avoidEarlyCollision, float* out_t, bool draw)
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
	if (!Math::PointInsideAABB(origin, bbmin, bbmax))
	{
		glm::vec3 point;
		// move origin to bounding box
		if (!Math::RayAABBIntersect(origin, dir, bbmin, bbmax, &point))
			return false;

		currentVoxel = {
			(point.x - bbmin.x) / voxelSize,
			(point.y - bbmin.y) / voxelSize,
			(point.z - bbmin.z) / voxelSize
		};
		if (currentVoxel.x < 0) currentVoxel.x = 0;
		if (currentVoxel.x > mat.size() - 1) currentVoxel.x = mat.size() - 1;
		if (currentVoxel.y < 0) currentVoxel.y = 0;
		if (currentVoxel.y > mat[0].size() - 1) currentVoxel.y = mat[0].size() - 1;
		if (currentVoxel.z < 0) currentVoxel.z = 0;
		if (currentVoxel.z > mat[0][0].size() - 1) currentVoxel.z = mat[0][0].size() - 1;
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
				if (currentVoxel.x > mat.size() - 1) return false;
				tMax.x = tMax.x + glm::abs(tDelta.x);
			}
			else
			{
				currentVoxel.z = currentVoxel.z + step.z;
				if (currentVoxel.z > mat[0][0].size() - 1) return false;
				tMax.z = tMax.z + glm::abs(tDelta.z);
			}
		}
		else
		{
			if (tMax.y < tMax.z)
			{
				currentVoxel.y = currentVoxel.y + step.y;
				if (currentVoxel.y > mat[0].size() - 1) return false;
				tMax.y = tMax.y + glm::abs(tDelta.y);
			}
			else
			{
				currentVoxel.z = currentVoxel.z + step.z;
				if (currentVoxel.z > mat[0][0].size() - 1) return false;
				tMax.z = tMax.z + glm::abs(tDelta.z);
			}
		}
		if (!mat[currentVoxel.x][currentVoxel.y][currentVoxel.z] && !inAir)
			inAir = true;

		if (mat[currentVoxel.x][currentVoxel.y][currentVoxel.z] && inAir)
		{
			if (out_t != nullptr)
				*out_t = glm::distance(GetVoxelCenterLocation(currentVoxel), origin);
			return true;
		}

		if (draw)
			mat[currentVoxel.x][currentVoxel.y][currentVoxel.z] = true;
	}
}