#include "SkeletonData.h"

#include <cstring>
#include <Geometry.h>

uint32_t sf::SkeletonData::AddNodeSingle(uint32_t animationIndex, float speed)
{
	m_nodes.emplace_back();
	m_nodes.back().single.type = Animation::NodeType::Single;
	if (m_nodes.size() == 1)
		m_nodes.back().single.weight = 1.0f;
	m_nodes.back().single.timer = 0.0f;
	m_nodes.back().single.animation = &m_animations[animationIndex];
	m_nodes.back().single.speed = speed;
	m_nodes.back().single.pose = new Transform[m_boneData.size()];
	memcpy(m_nodes.back().single.pose, m_boneLocalTransforms.data(), m_boneLocalTransforms.size() * sizeof(Transform)); /* Not all data comes from animation samplers */
	m_nodes.back().single.boneCount = m_boneData.size();
	return m_nodes.size() - 1;
}

uint32_t sf::SkeletonData::AddNodeBlendSpace1D(const std::vector<BlendSpacePoint1DCreateInfo>& points, float pos, float* weightsQuery)
{
	m_nodes.emplace_back();
	m_nodes.back().bs1d.type = Animation::NodeType::BlendSpace1D;
	if (m_nodes.size() == 1)
		m_nodes.back().bs1d.weight = 1.0f;
	m_nodes.back().bs1d.timer = 0.0f;
	m_nodes.back().bs1d.duration = -1.0f;
	m_nodes.back().bs1d.prevFrameDuration = -1.0f;
	m_nodes.back().bs1d.points = new Animation::BlendSpacePoint1D[points.size()];
	m_nodes.back().bs1d.pointCount = points.size();
	for (int i = 0; i < points.size(); i++)
	{
		m_nodes.back().bs1d.points[i].animation = &m_animations[points[i].animationIndex];
		m_nodes.back().bs1d.points[i].pos = points[i].pos;
		m_nodes.back().bs1d.points[i].speed = points[i].speed;
		m_nodes.back().bs1d.points[i].pose = new Transform[m_boneData.size()];
		memcpy(m_nodes.back().bs1d.points[i].pose, m_boneLocalTransforms.data(), m_boneLocalTransforms.size() * sizeof(Transform)); /* Not all data comes from animation samplers */
	}
	m_nodes.back().bs1d.pos = pos;
	m_nodes.back().bs1d.pose = new Transform[m_boneData.size()];
	m_nodes.back().bs1d.boneCount = m_boneData.size();
	m_nodes.back().bs1d.weightsQuery = weightsQuery;
	return m_nodes.size() - 1;
}

uint32_t sf::SkeletonData::AddNodeBlendSpace2D(const std::vector<BlendSpacePoint2DCreateInfo>& points, const glm::vec2& pos, float* weightsQuery)
{
	m_nodes.emplace_back();
	m_nodes.back().bs2d.type = Animation::NodeType::BlendSpace2D;
	if (m_nodes.size() == 1)
		m_nodes.back().bs2d.weight = 1.0f;
	m_nodes.back().bs2d.timer = 0.0f;
	m_nodes.back().bs2d.duration = -1.0f;
	m_nodes.back().bs2d.prevFrameDuration = -1.0f;
	m_nodes.back().bs2d.points = new Animation::BlendSpacePoint2D[points.size()];
	m_nodes.back().bs2d.pointCount = points.size();
	for (int i = 0; i < points.size(); i++)
	{
		m_nodes.back().bs2d.points[i].animation = &m_animations[points[i].animationIndex];
		m_nodes.back().bs2d.points[i].pos = points[i].pos;
		m_nodes.back().bs2d.points[i].speed = points[i].speed;
		m_nodes.back().bs2d.points[i].pose = new Transform[m_boneData.size()];
		memcpy(m_nodes.back().bs2d.points[i].pose, m_boneLocalTransforms.data(), m_boneLocalTransforms.size() * sizeof(Transform)); /* Not all data comes from animation samplers */
	}
	m_nodes.back().bs2d.pos = pos;
	m_nodes.back().bs2d.pose = new Transform[m_boneData.size()];
	m_nodes.back().bs2d.boneCount = m_boneData.size();
	m_nodes.back().bs2d.blendMatrix = new float[points.size() * points.size()];
	m_nodes.back().bs2d.needsToComputeBlendMatrix = true;
	m_nodes.back().bs2d.weightsQuery = weightsQuery;
	return m_nodes.size() - 1;
}

void sf::SkeletonData::UpdateAnimation(float deltaTime)
{
	uint32_t nodeCount = m_nodes.size();
	if (nodeCount == 0)
		return;

	// Update nodes
	for (int i = 0; i < nodeCount; i++)
		Animation::AdvanceNode(m_nodes[i], deltaTime);

	// Node blending
	float* weights = (float*)alloca(sizeof(float) * nodeCount);
	for (int j = 0; j < nodeCount; j++)
		weights[j] = m_nodes[j].single.weight;

	for (int i = 0; i < m_boneLocalTransforms.size(); i++)
	{
		if (nodeCount < 2)
		{
			m_boneLocalTransforms[i].position = m_nodes[0].single.pose[i].position;
			m_boneLocalTransforms[i].rotation = m_nodes[0].single.pose[i].rotation;
			m_boneLocalTransforms[i].scale = m_nodes[0].single.pose[i].scale;
		}
		else
		{
			glm::vec3* inpos = (glm::vec3*)alloca(sizeof(glm::vec3) * nodeCount);
			glm::quat* inrot = (glm::quat*)alloca(sizeof(glm::quat) * nodeCount);
			float* inscale = (float*)alloca(sizeof(float) * nodeCount);
			for (int j = 0; j < nodeCount; j++)
			{
				inpos[j] = m_nodes[j].single.pose[i].position;
				inrot[j] = m_nodes[j].single.pose[i].rotation;
				inscale[j] = m_nodes[j].single.pose[i].scale;
			}
			Math::WeightedBlend(inpos, weights, nodeCount, m_boneLocalTransforms[i].position);
			Math::WeightedBlend(inrot, weights, nodeCount, m_boneLocalTransforms[i].rotation);
			Math::WeightedBlend(inscale, weights, nodeCount, m_boneLocalTransforms[i].scale);
		}
	}

	// Update skinning matrices
	for (uint32_t i = 0; i < m_boneData.size(); i++)
	{
		const BoneData* currentBone = &(m_boneData[i]);
		if (currentBone->parent < 0)
			m_boneTransforms[i] = m_boneLocalTransforms[i];
		else
		{
			m_boneTransforms[i] = m_boneTransforms[currentBone->parent];
			m_boneTransforms[i].Apply(m_boneLocalTransforms[i]);
		}
		m_skinningMatrices[i] = m_boneTransforms[i].ComputeMatrix() * currentBone->invModelMatrix;
	}

	// // Update bone entity space transforms
	// for (uint32_t i = 0; i < m_boneData.size(); i++)
	// {
	// 	const BoneData* currentBone = &(m_boneData[i]);
	// 	if (currentBone->parent < 0)
	// 		m_boneTransforms[i] = m_boneLocalTransforms[i];
	// 	else
	// 	{
	// 		m_boneTransforms[i] = m_boneTransforms[currentBone->parent];
	// 		m_boneTransforms[i].Apply(m_boneLocalTransforms[i]);
	// 	}
	// }

	// // Apply two bone IK
	// // std::unordered_set<uint32_t> parentsForIkAffectedBones;
	// for (uint32_t i = 0; i < m_ikData.size(); i++)
	// {
	// 	TwoBoneIkData& ikd = m_ikData[i];
	// 	const BoneData* boneA = &(m_boneData[ikd.firstBone]);

	// 	printf("applying ik for bone: %u\n", ikd.firstBone);
	// 	Transform& a = m_boneLocalTransforms[ikd.firstBone + 0];
	// 	Transform& b = m_boneLocalTransforms[ikd.firstBone + 1];
	// 	Transform& aBase = m_boneTransforms[boneA->parent];

	// 	Geometry::TwoBoneIk(a, b, *ikd.targetPosEntitySpace, aBase);
	// 	// Geometry::TwoBoneIk(a.position, b.position, c.position, *ikd.targetPosEntitySpace, a.rotation, b.rotation);

	// 	// Update bone entity space transforms again for IK altered bones assuming the root bone is never affected
	// 	// parentsForIkAffectedBones.clear();
	// 	for (uint32_t j = ikd.firstBone; j < m_boneData.size(); j++)
	// 	{
	// 		// if (j == ikd.firstBone || parentsForIkAffectedBones.find(m_boneData[j].parent) != parentsForIkAffectedBones.end())
	// 		{
	// 			m_boneTransforms[j] = m_boneTransforms[m_boneData[j].parent];
	// 			m_boneTransforms[j].Apply(m_boneLocalTransforms[j]);
	// 			// parentsForIkAffectedBones.insert(j);
	// 		}
	// 	}
	// }

	// // Update skinning matrices
	// for (uint32_t i = 0; i < m_boneData.size(); i++)
	// 	m_skinningMatrices[i] = m_boneTransforms[i].ComputeMatrix() * m_boneData[i].invModelMatrix;
}