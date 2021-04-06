#include "VoxelModel.h"

#include <iostream>

#include <Math.hpp>

#define RASTERIZE_MAX_DISTANCE 0.7

void sf::VoxelModel::CreateFromModel(const Model& model, float voxelSize)
{
	assert(model.m_vertexVector.size() > 0);

	m_voxelSize = voxelSize;
	m_position = model.m_position;

	glm::vec3 minP = model.m_vertexVector[0].position;
	glm::vec3 maxP = minP;

	for (int i = 1; i < model.m_vertexVector.size(); i++)
	{
		minP.x = glm::min(minP.x, model.m_vertexVector[i].position.x);
		minP.y = glm::min(minP.y, model.m_vertexVector[i].position.y);
		minP.z = glm::min(minP.z, model.m_vertexVector[i].position.z);
		maxP.x = glm::max(maxP.x, model.m_vertexVector[i].position.x);
		maxP.y = glm::max(maxP.y, model.m_vertexVector[i].position.y);
		maxP.z = glm::max(maxP.z, model.m_vertexVector[i].position.z);
	}

	m_minPos = minP;

	glm::uvec3 voxelCount = {
		(unsigned int)glm::ceil((maxP.x - minP.x) / m_voxelSize),
		(unsigned int)glm::ceil((maxP.y - minP.y) / m_voxelSize),
		(unsigned int)glm::ceil((maxP.z - minP.z) / m_voxelSize)
	};

	// allocate matrix
	m_mat.resize(voxelCount.x);
	for (auto& vector : m_mat)
	{
		vector.resize(voxelCount.y);
		for (auto& subvector : vector)
			subvector.resize(voxelCount.z);
	}

	// rasterize
	for (int indexI = 0; indexI < model.m_indexVector.size(); indexI += 3)
	{
		unsigned int indexA = model.m_indexVector[indexI + 0];
		unsigned int indexB = model.m_indexVector[indexI + 1];
		unsigned int indexC = model.m_indexVector[indexI + 2];

		glm::vec3 triNormal = glm::cross(
			model.m_vertexVector[indexB].position - model.m_vertexVector[indexA].position,
			model.m_vertexVector[indexC].position - model.m_vertexVector[indexA].position);

		glm::vec3 trianglebbmin = {
			glm::min(glm::min(model.m_vertexVector[indexA].position.x, model.m_vertexVector[indexB].position.x), model.m_vertexVector[indexC].position.x),
			glm::min(glm::min(model.m_vertexVector[indexA].position.y, model.m_vertexVector[indexB].position.y), model.m_vertexVector[indexC].position.y),
			glm::min(glm::min(model.m_vertexVector[indexA].position.z, model.m_vertexVector[indexB].position.z), model.m_vertexVector[indexC].position.z)
		};
		glm::vec3 trianglebbmax = {
			glm::max(glm::max(model.m_vertexVector[indexA].position.x, model.m_vertexVector[indexB].position.x), model.m_vertexVector[indexC].position.x),
			glm::max(glm::max(model.m_vertexVector[indexA].position.y, model.m_vertexVector[indexB].position.y), model.m_vertexVector[indexC].position.y),
			glm::max(glm::max(model.m_vertexVector[indexA].position.z, model.m_vertexVector[indexB].position.z), model.m_vertexVector[indexC].position.z)
		};
		glm::uvec3 minVoxelCoords = {
			glm::clamp((int)((trianglebbmin.x - m_minPos.x) / m_voxelSize), 0, (int)(m_mat.size() - 1)),
			glm::clamp((int)((trianglebbmin.y - m_minPos.y) / m_voxelSize), 0, (int)(m_mat[0].size() - 1)),
			glm::clamp((int)((trianglebbmin.z - m_minPos.z) / m_voxelSize), 0, (int)(m_mat[0][0].size() - 1))
		};
		glm::uvec3 maxVoxelCoords = {
			glm::clamp((int)((trianglebbmax.x - m_minPos.x) / m_voxelSize), 0, (int)(m_mat.size() - 1)),
			glm::clamp((int)((trianglebbmax.y - m_minPos.y) / m_voxelSize), 0, (int)(m_mat[0].size() - 1)),
			glm::clamp((int)((trianglebbmax.z - m_minPos.z) / m_voxelSize), 0, (int)(m_mat[0][0].size() - 1))
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
				glm::vec2 voxelCenter = glm::vec2(m_minPos.x, m_minPos.y) + glm::vec2(i * m_voxelSize + m_voxelSize / 2.0f, j * m_voxelSize + m_voxelSize / 2.0f);
				glm::vec2 tri0 = glm::vec2(model.m_vertexVector[indexA].position.x, model.m_vertexVector[indexA].position.y);
				glm::vec2 tri1 = glm::vec2(model.m_vertexVector[indexB].position.x, model.m_vertexVector[indexB].position.y);
				glm::vec2 tri2 = glm::vec2(model.m_vertexVector[indexC].position.x, model.m_vertexVector[indexC].position.y);
				xyMat[i - minVoxelCoords.x][j - minVoxelCoords.y] = Math::TriPointDistance2D(voxelCenter, tri0, tri1, tri2) < m_voxelSize * RASTERIZE_MAX_DISTANCE;
			}
		}

		for (int i = minVoxelCoords.x; i <= maxVoxelCoords.x; i++)
		{
			for (int j = minVoxelCoords.z; j <= maxVoxelCoords.z; j++)
			{
				glm::vec2 voxelCenter = glm::vec2(m_minPos.x, m_minPos.z) + glm::vec2(i * m_voxelSize + m_voxelSize / 2.0f, j * m_voxelSize + m_voxelSize / 2.0f);
				glm::vec2 tri0 = glm::vec2(model.m_vertexVector[indexA].position.x, model.m_vertexVector[indexA].position.z);
				glm::vec2 tri1 = glm::vec2(model.m_vertexVector[indexB].position.x, model.m_vertexVector[indexB].position.z);
				glm::vec2 tri2 = glm::vec2(model.m_vertexVector[indexC].position.x, model.m_vertexVector[indexC].position.z);
				xzMat[i - minVoxelCoords.x][j - minVoxelCoords.z] = Math::TriPointDistance2D(voxelCenter, tri0, tri1, tri2) < m_voxelSize * RASTERIZE_MAX_DISTANCE;
			}
		}

		for (int i = minVoxelCoords.z; i <= maxVoxelCoords.z; i++)
		{
			for (int j = minVoxelCoords.y; j <= maxVoxelCoords.y; j++)
			{
				glm::vec2 voxelCenter = glm::vec2(m_minPos.z, m_minPos.y) + glm::vec2(i * m_voxelSize + m_voxelSize / 2.0f, j * m_voxelSize + m_voxelSize / 2.0f);
				glm::vec2 tri0 = glm::vec2(model.m_vertexVector[indexA].position.z, model.m_vertexVector[indexA].position.y);
				glm::vec2 tri1 = glm::vec2(model.m_vertexVector[indexB].position.z, model.m_vertexVector[indexB].position.y);
				glm::vec2 tri2 = glm::vec2(model.m_vertexVector[indexC].position.z, model.m_vertexVector[indexC].position.y);
				zyMat[i - minVoxelCoords.z][j - minVoxelCoords.y] = Math::TriPointDistance2D(voxelCenter, tri0, tri1, tri2) < m_voxelSize * RASTERIZE_MAX_DISTANCE;
			}
		}

		for (int i = minVoxelCoords.x; i <= maxVoxelCoords.x; i++)
		{
			for (int j = minVoxelCoords.y; j <= maxVoxelCoords.y; j++)
			{
				for (int k = minVoxelCoords.z; k <= maxVoxelCoords.z; k++)
				{
					glm::vec3 voxelCenter =
						glm::vec3(m_minPos.x, m_minPos.y, m_minPos.z) +
						glm::vec3(i * m_voxelSize + m_voxelSize / 2.0f, j * m_voxelSize + m_voxelSize / 2.0f, k * m_voxelSize + m_voxelSize / 2.0f);

					bool voxelValueForCurrentTri =
						xyMat[i - minVoxelCoords.x][j - minVoxelCoords.y] &&
						xzMat[i - minVoxelCoords.x][k - minVoxelCoords.z] &&
						zyMat[k - minVoxelCoords.z][j - minVoxelCoords.y] &&
						glm::abs(Math::PlanePointDistance(triNormal, model.m_vertexVector[indexA].position, voxelCenter)) < m_voxelSize * RASTERIZE_MAX_DISTANCE;

					m_mat[i][j][k] = m_mat[i][j][k] || voxelValueForCurrentTri;
				}
			}
		}
	}
}

// https://www.researchgate.net/publication/2611491_A_Fast_Voxel_Traversal_Algorithm_for_Ray_Tracing
bool sf::VoxelModel::CastRay(const glm::vec3& origin, const glm::vec3& direction, bool avoidEarlyCollision, float* out_t) const
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
			(point.x - bbmin.x) / m_voxelSize,
			(point.y - bbmin.y) / m_voxelSize,
			(point.z - bbmin.z) / m_voxelSize
		};
		if (currentVoxel.x < 0) currentVoxel.x = 0;
		if (currentVoxel.x > m_mat.size() - 1) currentVoxel.x = m_mat.size() - 1;
		if (currentVoxel.y < 0) currentVoxel.y = 0;
		if (currentVoxel.y > m_mat[0].size() - 1) currentVoxel.y = m_mat[0].size() - 1;
		if (currentVoxel.z < 0) currentVoxel.z = 0;
		if (currentVoxel.z > m_mat[0][0].size() - 1) currentVoxel.z = m_mat[0][0].size() - 1;
	}
	else
	{
		currentVoxel = {
			(origin.x - bbmin.x) / m_voxelSize,
			(origin.y - bbmin.y) / m_voxelSize,
			(origin.z - bbmin.z) / m_voxelSize
		};
	}

	glm::vec3 currentVoxelCenter = GetVoxelCenterLocation(currentVoxel);
	glm::vec3 nextBoundaries = {
		currentVoxelCenter.x + (step.x * m_voxelSize / 2.0f),
		currentVoxelCenter.y + (step.y * m_voxelSize / 2.0f),
		currentVoxelCenter.z + (step.z * m_voxelSize / 2.0f)
	};

	glm::vec3 tMax = {
		(nextBoundaries.x - origin.x) / dir.x,
		(nextBoundaries.y - origin.y) / dir.y,
		(nextBoundaries.z - origin.z) / dir.z,
	};

	glm::vec3 tDelta{
		m_voxelSize / dir.x,
		m_voxelSize / dir.y,
		m_voxelSize / dir.z
	};

	bool inAir = !avoidEarlyCollision;
	while (true)
	{
		if (tMax.x < tMax.y)
		{
			if (tMax.x < tMax.z)
			{
				currentVoxel.x = currentVoxel.x + step.x;
				if (currentVoxel.x > m_mat.size() - 1) return false;
				tMax.x = tMax.x + glm::abs(tDelta.x);
			}
			else
			{
				currentVoxel.z = currentVoxel.z + step.z;
				if (currentVoxel.z > m_mat[0][0].size() - 1) return false;
				tMax.z = tMax.z + glm::abs(tDelta.z);
			}
		}
		else
		{
			if (tMax.y < tMax.z)
			{
				currentVoxel.y = currentVoxel.y + step.y;
				if (currentVoxel.y > m_mat[0].size() - 1) return false;
				tMax.y = tMax.y + glm::abs(tDelta.y);
			}
			else
			{
				currentVoxel.z = currentVoxel.z + step.z;
				if (currentVoxel.z > m_mat[0][0].size() - 1) return false;
				tMax.z = tMax.z + glm::abs(tDelta.z);
			}
		}
		if (!m_mat[currentVoxel.x][currentVoxel.y][currentVoxel.z] && !inAir)
			inAir = true;

		if (m_mat[currentVoxel.x][currentVoxel.y][currentVoxel.z] && inAir)
		{
			if (out_t != nullptr)
				*out_t = glm::distance(GetVoxelCenterLocation(currentVoxel), origin);
			return true;
		}
	}
}

void sf::VoxelModel::DrawRay(const glm::vec3& origin, const glm::vec3 direction, bool avoidEarlyCollision)
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
			return;

		currentVoxel = {
			(point.x - bbmin.x) / m_voxelSize,
			(point.y - bbmin.y) / m_voxelSize,
			(point.z - bbmin.z) / m_voxelSize
		};
		if (currentVoxel.x < 0) currentVoxel.x = 0;
		if (currentVoxel.x > m_mat.size() - 1) currentVoxel.x = m_mat.size() - 1;
		if (currentVoxel.y < 0) currentVoxel.y = 0;
		if (currentVoxel.y > m_mat[0].size() - 1) currentVoxel.y = m_mat[0].size() - 1;
		if (currentVoxel.z < 0) currentVoxel.z = 0;
		if (currentVoxel.z > m_mat[0][0].size() - 1) currentVoxel.z = m_mat[0][0].size() - 1;
	}
	else
	{
		currentVoxel = {
			(origin.x - bbmin.x) / m_voxelSize,
			(origin.y - bbmin.y) / m_voxelSize,
			(origin.z - bbmin.z) / m_voxelSize
		};
	}

	glm::vec3 currentVoxelCenter = GetVoxelCenterLocation(currentVoxel);
	glm::vec3 nextBoundaries = {
		currentVoxelCenter.x + (step.x * m_voxelSize / 2.0f),
		currentVoxelCenter.y + (step.y * m_voxelSize / 2.0f),
		currentVoxelCenter.z + (step.z * m_voxelSize / 2.0f)
	};

	glm::vec3 tMax = {
		(nextBoundaries.x - origin.x) / dir.x,
		(nextBoundaries.y - origin.y) / dir.y,
		(nextBoundaries.z - origin.z) / dir.z,
	};

	glm::vec3 tDelta{
		m_voxelSize / dir.x,
		m_voxelSize / dir.y,
		m_voxelSize / dir.z
	};

	bool inAir = !avoidEarlyCollision;
	while (true)
	{
		if (tMax.x < tMax.y)
		{
			if (tMax.x < tMax.z)
			{
				currentVoxel.x = currentVoxel.x + step.x;
				if (currentVoxel.x > m_mat.size() - 1) return;
				tMax.x = tMax.x + glm::abs(tDelta.x);
			}
			else
			{
				currentVoxel.z = currentVoxel.z + step.z;
				if (currentVoxel.z > m_mat[0][0].size() - 1) return;
				tMax.z = tMax.z + glm::abs(tDelta.z);
			}
		}
		else
		{
			if (tMax.y < tMax.z)
			{
				currentVoxel.y = currentVoxel.y + step.y;
				if (currentVoxel.y > m_mat[0].size() - 1) return;
				tMax.y = tMax.y + glm::abs(tDelta.y);
			}
			else
			{
				currentVoxel.z = currentVoxel.z + step.z;
				if (currentVoxel.z > m_mat[0][0].size() - 1) return;
				tMax.z = tMax.z + glm::abs(tDelta.z);
			}
		}
		if (!m_mat[currentVoxel.x][currentVoxel.y][currentVoxel.z] && !inAir)
			inAir = true;

		if (m_mat[currentVoxel.x][currentVoxel.y][currentVoxel.z] && inAir)
			return;
		m_mat[currentVoxel.x][currentVoxel.y][currentVoxel.z] = true;
	}
}
