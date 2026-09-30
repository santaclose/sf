#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>

#include <Animation.h>
#include <Components/Transform.h>

#include <Components/SphereCollider.h>

namespace sf
{
	struct BoneData
	{
		const char* name;
		int32_t parent = -1;
		Transform invModelTransform;
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
	struct TwoBoneIkData
	{
		uint32_t elbowBone;
		const glm::vec3* targetPosEntitySpace;
		const glm::quat* targetRotEntitySpace;
		const float* armRotOffset;
	};

	struct SkeletonCoreData
	{
		std::vector<BoneData> m_boneData;
		std::vector<Transform> m_boneLocalTransforms;
		std::vector<Transform> m_boneTransforms;
		std::vector<glm::mat4> m_skinningMatrices;

		void SolveTwoBoneIK(const TwoBoneIkData& ikData);
		void RotateLeafIK(const TwoBoneIkData& ikData);
		void PropagateLocalTransforms(const std::unordered_map<uint32_t, TwoBoneIkData>* ikMap = nullptr, bool computeSkinningMatrices = false);
	};

	struct SkeletonData
	{
		SkeletonCoreData m_core;
		std::vector<Animation::SkeletalAnimation> m_animations;
		std::vector<Animation::Node> m_nodes;
		std::unordered_map<uint32_t, TwoBoneIkData> m_ikData;
		bool m_animate = false;

		uint32_t AddNodeSingle(uint32_t animationIndex, float speed = 1.0f);
		uint32_t AddNodeBlendSpace1D(const std::vector<BlendSpacePoint1DCreateInfo>& points, float pos, float* weightsQuery = nullptr);
		uint32_t AddNodeBlendSpace2D(const std::vector<BlendSpacePoint2DCreateInfo>& points, const glm::vec2& pos, float* weightsQuery = nullptr);

		inline uint32_t GetBoneCount() const { return m_core.m_boneData.size(); }
		inline const BoneData& GetBoneData(uint32_t boneIndex) const { return m_core.m_boneData[boneIndex]; }
		inline const Transform& GetBoneTransform(uint32_t boneIndex) const { return m_core.m_boneTransforms[boneIndex]; }
		inline const Transform& GetBoneLocalTransform(uint32_t boneIndex) const { return m_core.m_boneLocalTransforms[boneIndex]; }
		inline const Transform& GetInverseModelTransform(uint32_t boneIndex) const { return m_core.m_boneData[boneIndex].invModelTransform; }
		inline const glm::mat4* GetSkinMatrixBuffer() const { return m_core.m_skinningMatrices.data(); }
		inline const Transform* GetLocalTransformBuffer() const { return m_core.m_boneLocalTransforms.data(); }

		inline bool GetAnimate() { return m_animate; }
		inline void SetAnimate(bool value) { m_animate = value; }
		inline void SetBlendSpace1DPosition(uint32_t node, float pos) { assert(m_nodes[node].single.type == Animation::NodeType::BlendSpace1D); m_nodes[node].bs1d.pos = pos; }
		inline void SetBlendSpace2DPosition(uint32_t node, const glm::vec2& pos) { assert(m_nodes[node].single.type == Animation::NodeType::BlendSpace2D); m_nodes[node].bs2d.pos = pos; }

		void UpdateAnimation(float deltaTime);

		inline void AddTwoBoneIkData(uint32_t elbowBone, const glm::vec3* targetPosEntitySpace, const glm::quat* targetRotEntitySpace = nullptr, const float* armRotOffset = nullptr)
		{
			uint32_t shoulderBone = m_core.m_boneData[m_core.m_boneData[elbowBone].parent].parent;
			m_ikData[shoulderBone].targetPosEntitySpace = targetPosEntitySpace;
			m_ikData[shoulderBone].targetRotEntitySpace = targetRotEntitySpace;
			m_ikData[shoulderBone].armRotOffset = armRotOffset;
			m_ikData[shoulderBone].elbowBone = elbowBone;
		}
		void Destroy();
	};
}