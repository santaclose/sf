#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Animation.h>

namespace sf
{
	struct BoneData
	{
		int32_t parent = -1;
		glm::mat4 localMatrix = glm::mat4(1.0f);
		glm::mat4 invModelMatrix;
	};
	struct BlendSpacePoint1DCreateInfo
	{
		uint32_t animationIndex;
		float speed;
		float pos;
	};
	struct BlendSpacePoint2DCreateInfo
	{
		uint32_t animationIndex;
		float speed;
		glm::vec2 pos;
	};

	struct SkeletonData
	{
		uint32_t AddNodeSingle(uint32_t animationIndex, float speed = 1.0f);
		uint32_t AddNodeBlendSpace1D(const std::vector<BlendSpacePoint1DCreateInfo>& points, float pos, float* weightsQuery = nullptr);
		uint32_t AddNodeBlendSpace2D(const std::vector<BlendSpacePoint2DCreateInfo>& points, const glm::vec2& pos, float* weightsQuery = nullptr);

		inline bool GetAnimate() { return m_animate; }
		inline void SetAnimate(bool value) { m_animate = value; }
		inline void SetBlendSpace1DPosition(uint32_t node, float pos) { assert(m_nodes[node].single.type == Animation::NodeType::BlendSpace1D); m_nodes[node].bs1d.pos = pos; }
		inline void SetBlendSpace2DPosition(uint32_t node, const glm::vec2& pos) { assert(m_nodes[node].single.type == Animation::NodeType::BlendSpace2D); m_nodes[node].bs2d.pos = pos; }

		void UpdateAnimation(float deltaTime);

		bool m_animate = false;

		std::vector<Transform> m_boneTransforms;
		std::vector<BoneData> m_boneData;
		std::vector<glm::mat4> m_skinningMatrices;

		std::vector<Animation::SkeletalAnimation> m_animations;

		std::vector<Animation::Node> m_nodes;
	};
}