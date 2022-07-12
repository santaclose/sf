#include "SkeletonData.h"

#include <glm/gtc/type_ptr.hpp>

void sf::SkeletonData::ClampAnimationTime()
{
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
						bones[channel.bone].translationAnim = glm::vec3(trans);
						break;
					}
					case AnimationChannel::PathType::SCALE:
					{
						glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
						bones[channel.bone].scaleAnim = glm::max(glm::max(trans.x, trans.y), trans.z);
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
						bones[channel.bone].rotationAnim = glm::normalize(glm::slerp(q1, q2, u));

						break;
					}
					}
				}
			}
		}
	}

	// update local matrices
	for (Bone& bone : bones)
		bone.localMatrixAnim = glm::translate(glm::mat4(1.0f), bone.translationAnim) * glm::mat4(bone.rotationAnim) * glm::scale(glm::mat4(1.0f), glm::vec3(bone.scaleAnim));

	// update skinning matrices
	{
		glm::mat4* boneMatrices = (glm::mat4*)alloca(sizeof(glm::mat4) * bones.size());
		for (uint32_t i = 0; i < bones.size(); i++)
		{
			const Bone* currentBone = &(bones[i]);
			if (currentBone->parent < 0)
			{
				boneMatrices[i] = currentBone->localMatrixAnim;
				skinningMatrices[i] = boneMatrices[i] * currentBone->invModelMatrix;
			}
			else
			{
				boneMatrices[i] = boneMatrices[currentBone->parent] * currentBone->localMatrixAnim;
				skinningMatrices[i] = boneMatrices[i] * currentBone->invModelMatrix;
			}
		}
	}
}
