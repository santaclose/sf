#include "SkeletonData.h"

#include <unordered_set>
#include <glm/gtc/type_ptr.hpp>

#include <Math.hpp>

void sf::SkeletonData::ClampAnimationTime()
{
	assert(animationIndex < animations.size());

	if (animationTime > animations[animationIndex].end)
		animationTime = animations[animationIndex].start;
	else if (animationTime < animations[animationIndex].start)
		animationTime = animations[animationIndex].end;
}

void sf::SkeletonData::UpdateAnimation()
{
	assert(animationIndex < animations.size());

	SkeletalAnimation& animation = animations[animationIndex];
	for (auto& channel : animation.channels)
	{
		AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
		if (sampler.inputs.size() > sampler.outputsVec4.size())
			continue;

		for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
		{
			if ((animationTime >= sampler.inputs[i]) && (animationTime <= sampler.inputs[i + 1]))
			{
				float u = std::max(0.0f, animationTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
				if (u <= 1.0f)
				{

					switch (channel.path)
					{
					case AnimationChannel::PathType::TRANSLATION:
					{
						glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
						bones[channel.bone].translation = glm::vec3(trans);
						break;
					}
					case AnimationChannel::PathType::SCALE:
					{
						glm::vec4 scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
						bones[channel.bone].scale = glm::max(glm::max(scale.x, scale.y), scale.z);
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
						bones[channel.bone].rotation = glm::normalize(glm::slerp(q1, q2, u));

						break;
					}
					}
				}
			}
		}
	}

	// update local matrices
	for (Bone& bone : bones)
		bone.localMatrix = glm::translate(glm::mat4(1.0f), bone.translation) * glm::mat4(bone.rotation) * glm::scale(glm::mat4(1.0f), glm::vec3(bone.scale));

	// update skinning matrices
	{
		glm::mat4* boneMatrices = (glm::mat4*)alloca(sizeof(glm::mat4) * bones.size());
		for (uint32_t i = 0; i < bones.size(); i++)
		{
			const Bone* currentBone = &(bones[i]);
			if (currentBone->parent < 0)
			{
				boneMatrices[i] = currentBone->localMatrix;
				skinningMatrices[i] = boneMatrices[i] * currentBone->invModelMatrix;
			}
			else
			{
				boneMatrices[i] = boneMatrices[currentBone->parent] * currentBone->localMatrix;
				skinningMatrices[i] = boneMatrices[i] * currentBone->invModelMatrix;
			}
		}
	}
}


float animationBlendingTimer = 0.0f;
void sf::SkeletonData::UpdateAnimation(const std::vector<uint32_t>& indices, const std::vector<float>& weights, float deltaTime)
{
	assert(indices.size() == weights.size());
	assert(animationIndex < animations.size());

	static std::vector<std::vector<glm::vec3>> intermediateTranslations;
	static std::vector<std::vector<glm::quat>> intermediateRotations;
	static std::vector<std::vector<float>> intermediateScales;
	//if (intermediateTranslations.size() == 0)
	{
		intermediateTranslations.resize(animations.size());
		intermediateRotations.resize(animations.size());
		intermediateScales.resize(animations.size());
		for (int i = 0; i < animations.size(); i++)
		{
			intermediateTranslations[i].resize(bones.size());
			intermediateRotations[i].resize(bones.size());
			intermediateScales[i].resize(bones.size());
		}
	}
	for (int i = 0; i < bones.size(); i++)
	{
		for (int j = 0; j < animations.size(); j++)
		{
			intermediateTranslations[j][i] = bones[i].translation;
			intermediateRotations[j][i] = bones[i].rotation;
			intermediateScales[j][i] = bones[i].scale;
		}
	}
	// compute pose for each index
	//float blendedStart = glm::mix(animations[indices[0]].start, animations[indices[1]].start, weights[0]);
	//float blendedEnd = glm::mix(animations[indices[0]].end, animations[indices[1]].end, weights[0]);
	float blendedStart = 0.0f;
	float blendedEnd = 0.0f;
	for (int i = 0; i < indices.size(); i++)
	{
		blendedEnd += animations[indices[i]].end * weights[i];
		blendedStart += animations[indices[i]].start * weights[i];
	}
	float blendedDuration = blendedEnd - blendedStart;

	animationBlendingTimer += deltaTime;
	if (animationBlendingTimer > blendedDuration)
		animationBlendingTimer = 0.0f;
	
	float alpha = animationBlendingTimer / blendedDuration;

	for (uint32_t animationIndex : indices)
	{
		SkeletalAnimation& animation = animations[animationIndex];

		float duration = animation.end - animation.start;
		float timeScale = blendedDuration / duration;

		float convertedTime = duration * alpha + animation.start;
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
	for (int i = 0; i < bones.size(); i++)
	{
		for (int j = 0; j < indices.size(); j++)
		{
			translationHelper[j] = intermediateTranslations[indices[j]][i];
			rotationHelper[j] = intermediateRotations[indices[j]][i];
			scaleHelper[j] = intermediateScales[indices[j]][i];
		}

		Math::WeightedBlend(&(translationHelper[0]), &(weights[0]), indices.size(), bones[i].translation);
		Math::WeightedBlend(&(rotationHelper[0]), &(weights[0]), indices.size(), bones[i].rotation);
		Math::WeightedBlend(&(scaleHelper[0]), &(weights[0]), indices.size(), bones[i].scale);

		//intermediateTranslations[indices[0]][i] = glm::mix(intermediateTranslations[indices[0]][i], intermediateTranslations[indices[1]][i], weights[0]);
		//intermediateRotations[indices[0]][i] = glm::normalize(glm::slerp(intermediateRotations[indices[0]][i], intermediateRotations[indices[1]][i], weights[0]));
		//intermediateScales[indices[0]][i] = glm::mix(intermediateScales[indices[0]][i], intermediateScales[indices[1]][i], weights[0]);

		//bones[i].translation = intermediateTranslations[indices[0]][i];
		//bones[i].rotation = intermediateRotations[indices[0]][i];
		//bones[i].scale = intermediateScales[indices[0]][i];
	}

	// update local matrices
	for (Bone& bone : bones)
		bone.localMatrix = glm::translate(glm::mat4(1.0f), bone.translation) * glm::mat4(bone.rotation) * glm::scale(glm::mat4(1.0f), glm::vec3(bone.scale));

	// update skinning matrices
	{
		glm::mat4* boneMatrices = (glm::mat4*)alloca(sizeof(glm::mat4) * bones.size());
		for (uint32_t i = 0; i < bones.size(); i++)
		{
			const Bone* currentBone = &(bones[i]);
			if (currentBone->parent < 0)
			{
				boneMatrices[i] = currentBone->localMatrix;
				skinningMatrices[i] = boneMatrices[i] * currentBone->invModelMatrix;
			}
			else
			{
				boneMatrices[i] = boneMatrices[currentBone->parent] * currentBone->localMatrix;
				skinningMatrices[i] = boneMatrices[i] * currentBone->invModelMatrix;
			}
		}
	}
}
