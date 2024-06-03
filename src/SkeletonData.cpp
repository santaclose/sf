#include "SkeletonData.h"

#include <glm/gtc/type_ptr.hpp>
#include <Math.hpp>
#include <BlendMatrixInterpolation.h>

void sf::SkeletonData::UpdateAnimation(float deltaTime, float* outWeights)
{
	if (m_selectedBlendSpaceD != -1)
	{
		UpdateAnimationUsingBlendspace(deltaTime, outWeights);
		return;
	}

	assert(m_animationIndex < m_animations.size());

	m_animationTime += deltaTime;
	if (m_animationTime > m_animations[m_animationIndex].end)
		m_animationTime = m_animations[m_animationIndex].start;
	else if (m_animationTime < m_animations[m_animationIndex].start)
		m_animationTime = m_animations[m_animationIndex].end;

	SkeletalAnimation& animation = m_animations[m_animationIndex];
	for (auto& channel : animation.channels)
	{
		AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
		if (sampler.inputs.size() > sampler.outputsVec4.size())
			continue;

		for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
		{
			if ((m_animationTime >= sampler.inputs[i]) && (m_animationTime <= sampler.inputs[i + 1]))
			{
				float u = std::max(0.0f, m_animationTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
				if (u <= 1.0f)
				{
					switch (channel.path)
					{
					case AnimationChannel::PathType::TRANSLATION:
					{
						glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
						m_bones[channel.bone].translation = glm::vec3(trans);
						break;
					}
					case AnimationChannel::PathType::SCALE:
					{
						glm::vec4 scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
						m_bones[channel.bone].scale = glm::max(glm::max(scale.x, scale.y), scale.z);
						break;
					}
					case AnimationChannel::PathType::ROTATION:
					{
						glm::quat q1;
						q1.x = sampler.outputsVec4[i].x;
						q1.y = sampler.outputsVec4[i].y;
						q1.z = sampler.outputsVec4[i].z;
						q1.w = sampler.outputsVec4[i].w;
						glm::quat q2;
						q2.x = sampler.outputsVec4[i + 1].x;
						q2.y = sampler.outputsVec4[i + 1].y;
						q2.z = sampler.outputsVec4[i + 1].z;
						q2.w = sampler.outputsVec4[i + 1].w;
						m_bones[channel.bone].rotation = glm::normalize(glm::slerp(q1, q2, u));
						break;
					}
					}
				}
			}
		}
	}

	// update local matrices
	for (Bone& bone : m_bones)
		bone.localMatrix = glm::translate(glm::mat4(1.0f), bone.translation) * glm::mat4(bone.rotation) * glm::scale(glm::mat4(1.0f), glm::vec3(bone.scale));

	// update skinning matrices
	{
		glm::mat4* boneMatrices = (glm::mat4*)alloca(sizeof(glm::mat4) * m_bones.size());
		for (uint32_t i = 0; i < m_bones.size(); i++)
		{
			const Bone* currentBone = &(m_bones[i]);
			if (currentBone->parent < 0)
			{
				boneMatrices[i] = currentBone->localMatrix;
				m_skinningMatrices[i] = boneMatrices[i] * currentBone->invModelMatrix;
			}
			else
			{
				boneMatrices[i] = boneMatrices[currentBone->parent] * currentBone->localMatrix;
				m_skinningMatrices[i] = boneMatrices[i] * currentBone->invModelMatrix;
			}
		}
	}
}

void sf::SkeletonData::UpdateAnimationUsingBlendspace(float deltaTime, float* outWeights)
{
	static std::vector<float> weights;
	static std::vector<float> speeds;
	static std::vector<uint32_t> indices;
	static std::vector<float> durations;

	if (m_selectedBlendSpaceD == 1)
	{
		// at most will be blending two animations at a time
		BlendSpace1D& blendSpace = m_blendSpaces1D[m_selectedBlendSpace];
		assert(blendSpace.points.size() >= 2);
		weights.resize(2);
		speeds.resize(2);
		indices.resize(2);
		durations.resize(2);
		int a, b;
		for (int i = 1; i < blendSpace.points.size(); i++)
		{
			assert(blendSpace.points[i].x > blendSpace.points[i - 1].x);
			if (blendSpace.points[i].x > blendSpace.x)
			{
				a = i - 1;
				b = i;
				indices[1] = blendSpace.points[b].animationIndex;
				indices[0] = blendSpace.points[a].animationIndex;
				float xDistance = blendSpace.points[b].x - blendSpace.points[a].x;
				weights[1] = (blendSpace.x - blendSpace.points[a].x) / xDistance;
				weights[0] = 1.0f - weights[1];
				speeds[1] = blendSpace.points[b].animationSpeed;
				speeds[0] = blendSpace.points[a].animationSpeed;
				SkeletalAnimation& animB = m_animations[indices[1]];
				SkeletalAnimation& animA = m_animations[indices[0]];
				durations[1] = (animB.end - animB.start) / glm::abs(speeds[1]);
				durations[0] = (animA.end - animA.start) / glm::abs(speeds[0]);
				break;
			}
		}
		if (outWeights != nullptr)
			for (int i = 0; i < blendSpace.points.size(); i++)
				outWeights[i] = (i == a ? weights[0] : (i == b ? weights[1] : 0.0f));
		UpdateAnimationBlended(weights, speeds, indices, durations, blendSpace.animationBlendingTimer, deltaTime);
	}
	else // m_selectedBlendSpaceD == 2
	{
		// can be blending any number of animations at a time
		BlendSpace2D& blendSpace = m_blendSpaces2D[m_selectedBlendSpace];
		assert(blendSpace.points.size() >= 2);
		weights.resize(blendSpace.points.size());
		speeds.resize(blendSpace.points.size());
		indices.resize(blendSpace.points.size());
		durations.resize(blendSpace.points.size());

		static std::vector<glm::vec2> blendSpace2dPoints;
		blendSpace2dPoints.resize(blendSpace.points.size());
		for (int i = 0; i < blendSpace.points.size(); i++)
		{
			speeds[i] = blendSpace.points[i].animationSpeed;
			indices[i] = blendSpace.points[i].animationIndex;
			SkeletalAnimation& anim = m_animations[indices[i]];
			durations[i] = (anim.end - anim.start) / glm::abs(speeds[i]);

			blendSpace2dPoints[i] = blendSpace.points[i].pos;
		}
		{ // compute weights
			std::vector<float> distances;
			distances.clear();
			distances.reserve(blendSpace2dPoints.size());
			for (const glm::vec2& p : blendSpace2dPoints)
				distances.push_back(glm::length(p - blendSpace.pos));

			bool needToFitMatrix = blendSpace.blendMatrix.size() == 0;
			if (needToFitMatrix)
				blendSpace.blendMatrix.resize(blendSpace2dPoints.size() * blendSpace2dPoints.size());
			BlendMatrixInterpolation::slice2d<float> blend_matrix(blendSpace2dPoints.size(), blendSpace2dPoints.size(), blendSpace.blendMatrix.data());
			if (needToFitMatrix)
				BlendMatrixInterpolation::fit_blend_matrix(blend_matrix, blendSpace2dPoints.data(), blendSpace2dPoints.size());
			BlendMatrixInterpolation::compute_blend_weights(BlendMatrixInterpolation::slice1d<float>(weights.size(), weights.data()), BlendMatrixInterpolation::slice1d<float>(distances.size(), distances.data()), blend_matrix);
		}
		if (outWeights != nullptr)
			for (int i = 0; i < blendSpace.points.size(); i++)
				outWeights[i] = weights[i];
		UpdateAnimationBlended(weights, speeds, indices, durations, blendSpace.animationBlendingTimer, deltaTime);
	}
}

void sf::SkeletonData::UpdateAnimationBlended(const std::vector<float>& weights, const std::vector<float>& speeds, const std::vector<uint32_t>& indices, const std::vector<float>& durations, float& timer, float deltaTime)
{
	static std::vector<std::vector<glm::vec3>> intermediateTranslations;
	static std::vector<std::vector<glm::quat>> intermediateRotations;
	static std::vector<std::vector<float>> intermediateScales;

	intermediateTranslations.resize(m_animations.size());
	intermediateRotations.resize(m_animations.size());
	intermediateScales.resize(m_animations.size());
	for (int i = 0; i < m_animations.size(); i++)
	{
		intermediateTranslations[i].resize(m_bones.size());
		intermediateRotations[i].resize(m_bones.size());
		intermediateScales[i].resize(m_bones.size());
	}

	for (int i = 0; i < m_bones.size(); i++)
	{
		for (int j = 0; j < m_animations.size(); j++)
		{
			intermediateTranslations[j][i] = m_bones[i].translation;
			intermediateRotations[j][i] = m_bones[i].rotation;
			intermediateScales[j][i] = m_bones[i].scale;
		}
	}

	float blendedDuration;
	Math::WeightedBlend(durations.data(), weights.data(), weights.size(), blendedDuration);
	if (m_previousBlendedDuration != -1.0f && m_previousBlendedDuration != blendedDuration)
		timer *= blendedDuration / m_previousBlendedDuration;
	m_previousBlendedDuration = blendedDuration;
	timer += deltaTime;
	if (timer > blendedDuration)
		timer = 0.0f;

	float alpha = timer / blendedDuration;

	for (int currentAnim = 0; currentAnim < indices.size(); currentAnim++)
	{
		uint32_t animationIndex = indices[currentAnim];
		SkeletalAnimation& animation = m_animations[animationIndex];

		float duration = animation.end - animation.start;

		float convertedTime = duration * (speeds[currentAnim] < 0.0f ? 1.0f - alpha : alpha) + animation.start;
		for (auto& channel : animation.channels)
		{
			AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
			if (sampler.inputs.size() > sampler.outputsVec4.size())
				continue;

			for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
			{
				if ((convertedTime >= sampler.inputs[i]) && (convertedTime <= sampler.inputs[i + 1]))
				{
					float u = std::max(0.0f, convertedTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
					if (u <= 1.0f)
					{
						switch (channel.path)
						{
						case AnimationChannel::PathType::TRANSLATION:
						{
							glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
							intermediateTranslations[animationIndex][channel.bone] = glm::vec3(trans);
							break;
						}
						case AnimationChannel::PathType::SCALE:
						{
							glm::vec4 scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
							intermediateScales[animationIndex][channel.bone] = glm::max(glm::max(scale.x, scale.y), scale.z);
							break;
						}
						case AnimationChannel::PathType::ROTATION:
						{
							glm::quat q1;
							q1.x = sampler.outputsVec4[i].x;
							q1.y = sampler.outputsVec4[i].y;
							q1.z = sampler.outputsVec4[i].z;
							q1.w = sampler.outputsVec4[i].w;
							glm::quat q2;
							q2.x = sampler.outputsVec4[i + 1].x;
							q2.y = sampler.outputsVec4[i + 1].y;
							q2.z = sampler.outputsVec4[i + 1].z;
							q2.w = sampler.outputsVec4[i + 1].w;
							intermediateRotations[animationIndex][channel.bone] = glm::normalize(glm::slerp(q1, q2, u));
							break;
						}
						}
					}
				}
			}
		}
	}

	// blend
	std::vector<glm::vec3> translationHelper(indices.size());
	std::vector<glm::quat> rotationHelper(indices.size());
	std::vector<float> scaleHelper(indices.size());
	for (int i = 0; i < m_bones.size(); i++)
	{
		for (int j = 0; j < indices.size(); j++)
		{
			translationHelper[j] = intermediateTranslations[indices[j]][i];
			rotationHelper[j] = intermediateRotations[indices[j]][i];
			scaleHelper[j] = intermediateScales[indices[j]][i];
		}

		Math::WeightedBlend(&(translationHelper[0]), &(weights[0]), indices.size(), m_bones[i].translation);
		Math::WeightedBlend(&(rotationHelper[0]), &(weights[0]), indices.size(), m_bones[i].rotation);
		Math::WeightedBlend(&(scaleHelper[0]), &(weights[0]), indices.size(), m_bones[i].scale);
	}

	// update local matrices
	for (Bone& bone : m_bones)
		bone.localMatrix = glm::translate(glm::mat4(1.0f), bone.translation) * glm::mat4(bone.rotation) * glm::scale(glm::mat4(1.0f), glm::vec3(bone.scale));

	// update skinning matrices
	{
		glm::mat4* boneMatrices = (glm::mat4*)alloca(sizeof(glm::mat4) * m_bones.size());
		for (uint32_t i = 0; i < m_bones.size(); i++)
		{
			const Bone* currentBone = &(m_bones[i]);
			if (currentBone->parent < 0)
			{
				boneMatrices[i] = currentBone->localMatrix;
				m_skinningMatrices[i] = boneMatrices[i] * currentBone->invModelMatrix;
			}
			else
			{
				boneMatrices[i] = boneMatrices[currentBone->parent] * currentBone->localMatrix;
				m_skinningMatrices[i] = boneMatrices[i] * currentBone->invModelMatrix;
			}
		}
	}
}
