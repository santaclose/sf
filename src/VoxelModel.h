#pragma once

#include <glm/glm.hpp>
#include <vector>

#include <Entity.h>
#include <Model.h>

namespace sf {

	class VoxelModel : public Entity
	{
		friend Model;

		glm::vec3 m_minPos;
		float m_voxelSize;
		std::vector<std::vector<std::vector<bool>>> m_mat;

	public:
		void CreateFromModel(const Model& model, float voxelSize);
		bool CastRay(const glm::vec3& origin, const glm::vec3& direction, bool avoidEarlyCollision = false, float* out_t = nullptr) const;

		inline glm::vec3 GetAABBMin() const
		{
			return m_minPos;
		}
		inline glm::vec3 GetAABBMax() const
		{
			assert(m_mat.size() > 0 && m_mat[0].size() > 0 && m_mat[0][0].size() > 0);
			return m_minPos + glm::vec3(m_mat.size() * m_voxelSize, m_mat[0].size() * m_voxelSize, m_mat[0][0].size() * m_voxelSize);
		}
		inline glm::vec3 GetVoxelCenterLocation(glm::uvec3 coords) const
		{
			return glm::vec3(
				m_minPos.x + (m_voxelSize * (float)coords.x) + m_voxelSize / 2.0f,
				m_minPos.y + (m_voxelSize * (float)coords.y) + m_voxelSize / 2.0f,
				m_minPos.z + (m_voxelSize * (float)coords.z) + m_voxelSize / 2.0f);
		}
	};

}