#include "SkeletonData.h"

#include <cstring>
#include <glm/gtx/vector_angle.hpp>

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

	// Update entity space bone transforms and skinning matrices
	TwoBoneIkData* nextIkLeafToRotateData = nullptr;
	for (uint32_t i = 0; i < m_boneData.size(); i++)
	{
		if (m_ikData.find(i) != m_ikData.end())
		{
			SolveTwoBoneIK(m_ikData[i]);
			nextIkLeafToRotateData = &m_ikData[i];
		}
		else if (nextIkLeafToRotateData != nullptr &&
			i == nextIkLeafToRotateData->elbowBone &&
			nextIkLeafToRotateData->targetRotEntitySpace != nullptr)
		{
			RotateLeafIK(*nextIkLeafToRotateData);
			nextIkLeafToRotateData = nullptr;
		}

		const BoneData* currentBone = &(m_boneData[i]);
		if (currentBone->parent < 0)
			m_boneTransforms[i] = m_boneLocalTransforms[i];
		else
		{
			m_boneTransforms[i] = m_boneTransforms[currentBone->parent];
			m_boneTransforms[i].Apply(m_boneLocalTransforms[i]);
		}
		Transform skinningTransform = m_boneTransforms[i];
		skinningTransform.Apply(currentBone->invModelTransform);
		m_skinningMatrices[i] = skinningTransform.ComputeMatrix();
	}
}

void sf::SkeletonData::RotateLeafIK(const TwoBoneIkData& ikData)
{
	uint32_t elbowBoneId = ikData.elbowBone;
	const BoneData* elbowBone = &(m_boneData[elbowBoneId]);
	uint32_t armBoneId = elbowBone->parent;
	m_boneLocalTransforms[elbowBoneId].rotation = glm::inverse(m_boneTransforms[armBoneId].rotation) * *ikData.targetRotEntitySpace;
}

void sf::SkeletonData::SolveTwoBoneIK(const TwoBoneIkData& ikData)
{
	uint32_t elbowBoneId = ikData.elbowBone;
	const BoneData* elbowBone = &(m_boneData[elbowBoneId]);
	uint32_t armBoneId = elbowBone->parent;
	const BoneData* armBone = &(m_boneData[armBoneId]);
	uint32_t shoulderBoneId = armBone->parent;
	const BoneData* shoulderBone = &(m_boneData[shoulderBoneId]);
	uint32_t shoulderParentBoneId = shoulderBone->parent;

	assert(shoulderParentBoneId != ~0);
	Transform temp = m_boneTransforms[shoulderParentBoneId];
	temp.Apply(m_boneLocalTransforms[shoulderBoneId]);
	glm::vec3 a = temp.position;
	glm::quat a_gr = temp.rotation;
	temp.Apply(m_boneLocalTransforms[armBoneId]);
	glm::vec3 b = temp.position;
	glm::quat b_gr = temp.rotation;
	temp.Apply(m_boneLocalTransforms[elbowBoneId]);
	glm::vec3 c = temp.position;

	assert(ikData.targetPosEntitySpace != nullptr);
	glm::vec3 t = *ikData.targetPosEntitySpace;
	glm::quat& a_lr = m_boneLocalTransforms[shoulderBoneId].rotation;
	glm::quat& b_lr = m_boneLocalTransforms[armBoneId].rotation;

	float eps = 0.01f;
	float lab = glm::length(b - a);
	float lcb = glm::length(b - c);
	float lat = glm::clamp(glm::length(t - a), eps, lab + lcb - eps);

	float ac_ab_0 = glm::acos(glm::clamp(glm::dot(
		glm::normalize(c - a),
		glm::normalize(b - a)), -1.0f, 1.0f));

	float ba_bc_0 = glm::acos(glm::clamp(glm::dot(
		glm::normalize(a - b),
		glm::normalize(c - b)), -1.0f, 1.0f));

	float ac_at_0 = glm::acos(glm::clamp(glm::dot(
		glm::normalize(c - a),
		glm::normalize(t - a)), -1.0f, 1.0f));

	float ac_ab_1 = glm::acos(glm::clamp((lcb * lcb - lab * lab - lat * lat) / (-2.0f * lab * lat), -1.0f, 1.0f));
	float ba_bc_1 = glm::acos(glm::clamp((lat * lat - lab * lab - lcb * lcb) / (-2.0f * lab * lcb), -1.0f, 1.0f));

	glm::vec3 axis0 = glm::normalize(glm::cross(c - a, b - a));

	glm::vec3 axis1 = glm::normalize(glm::cross(c - a, t - a));

	glm::quat r0 = glm::angleAxis(ac_ab_1 - ac_ab_0, glm::inverse(a_gr) * axis0);
	glm::quat r1 = glm::angleAxis(ba_bc_1 - ba_bc_0, glm::inverse(b_gr) * axis0);
	glm::quat r2 = glm::angleAxis(ac_at_0, glm::inverse(a_gr) * axis1);

	glm::quat rotOffset = ikData.armRotOffset != nullptr ?
		glm::angleAxis(*ikData.armRotOffset, glm::inverse(a_gr) * glm::normalize(t - a)) :
		glm::identity<glm::quat>();

	a_lr = a_lr * (rotOffset * r2 * r0);
	b_lr = b_lr * r1;
}