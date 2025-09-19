#include "Animation.h"

#include <BlendMatrixInterpolation.h>

void sf::Animation::ComputeNodeWeights(sf::Animation::Node& node)
{
	switch (node.single.type)
	{
	case NodeType::Single:
		/* Nothing to do */
		break;
	case NodeType::BlendSpace1D:
	{
		uint32_t pointCount = node.bs1d.pointCount;
		float* outWeights = node.bs1d.weightsQuery;
		assert(pointCount > 1);
		if (outWeights == nullptr)
			outWeights = (float*)alloca(sizeof(float) * pointCount);
		assert(node.bs1d.pos >= node.bs1d.points[0].pos && node.bs1d.pos <= node.bs1d.points[pointCount - 1].pos);
		for (int i = 0; i < pointCount; i++)
		{
			assert(i == 0 || node.bs1d.points[i].pos > node.bs1d.points[i - 1].pos);
			if (i < pointCount - 1 && node.bs1d.points[i].pos < node.bs1d.pos && node.bs1d.points[i + 1].pos > node.bs1d.pos)
			{
				float distance = node.bs1d.points[i + 1].pos - node.bs1d.points[i].pos;
				outWeights[i + 1] = (node.bs1d.pos - node.bs1d.points[i].pos) / distance;
				outWeights[i] = 1.0f - outWeights[i + 1];
				i++;
			}
			else if (node.bs1d.points[i].pos == node.bs1d.pos)
				outWeights[i] = 1.0f;
			else
				outWeights[i] = 0.0f;
		}
		for (int i = 0; i < pointCount; i++)
			node.bs2d.points[i].weight = outWeights[i];
		break;
	}
	case NodeType::BlendSpace2D:
	{
		uint32_t pointCount = node.bs2d.pointCount;
		float* outWeights = node.bs2d.weightsQuery;
		BlendMatrixInterpolation::slice2d<float> blend_matrix(pointCount, pointCount, node.bs2d.blendMatrix);
		assert(pointCount > 1);
		if (outWeights == nullptr)
			outWeights = (float*)alloca(sizeof(float) * pointCount);
		if (node.bs2d.needsToComputeBlendMatrix)
		{
			glm::vec2* bs2dPoints = (glm::vec2*)alloca(sizeof(glm::vec2) * pointCount);
			for (int i = 0; i < pointCount; i++)
				bs2dPoints[i] = node.bs2d.points[i].pos;
			BlendMatrixInterpolation::fit_blend_matrix(blend_matrix, bs2dPoints, pointCount);
			node.bs2d.needsToComputeBlendMatrix = false;
		}
		float* bs2dDistances = (float*)alloca(sizeof(float) * pointCount);
		for (int i = 0; i < pointCount; i++)
			bs2dDistances[i] = glm::length(node.bs2d.points[i].pos- node.bs2d.pos);
		BlendMatrixInterpolation::compute_blend_weights(BlendMatrixInterpolation::slice1d<float>(pointCount, outWeights), BlendMatrixInterpolation::slice1d<float>(pointCount, bs2dDistances), blend_matrix);
		for (int i = 0; i < pointCount; i++)
			node.bs2d.points[i].weight = outWeights[i];
		break;
	}
	}
}

void sf::Animation::ComputeNodeDuration(sf::Animation::Node& node)
{
	switch (node.single.type)
	{
	case NodeType::Single:
		node.single.duration = node.single.ComputeDuration();
		break;
	case NodeType::BlendSpace1D:
	{
		node.bs1d.prevFrameDuration = node.bs1d.duration;
		uint32_t pointCount = node.bs1d.pointCount;
		float* weights = (float*)alloca(sizeof(float) * pointCount);
		float* durations = (float*)alloca(sizeof(float) * pointCount);
		for (int i = 0; i < pointCount; i++)
		{
			weights[i] = node.bs1d.points[i].weight;
			durations[i] = node.bs1d.points[i].ComputeDuration();
		}
		Math::WeightedBlend(durations, weights, pointCount, node.bs1d.duration);
		break;
	}
	case NodeType::BlendSpace2D:
	{
		node.bs2d.prevFrameDuration = node.bs2d.duration;
		uint32_t pointCount = node.bs2d.pointCount;
		float* weights = (float*)alloca(sizeof(float) * pointCount);
		float* durations = (float*)alloca(sizeof(float) * pointCount);
		for (int i = 0; i < pointCount; i++)
		{
			weights[i] = node.bs2d.points[i].weight;
			durations[i] = node.bs2d.points[i].ComputeDuration();
		}
		Math::WeightedBlend(durations, weights, pointCount, node.bs2d.duration);
		break;
	}
	default:
		assert(false);
		break;
	}
}

void sf::Animation::ComputeNodePose(sf::Animation::Node& node)
{
	static std::vector<Transform> intermeditateBones;
	switch (node.single.type)
	{
	case NodeType::Single:
	{
		float convertedTimer = node.single.timer * (node.single.RawDuration() / node.single.duration);
		if (node.single.speed >= 0.0f)
			convertedTimer = node.single.animation->start + convertedTimer;
		else
			convertedTimer = node.single.animation->end - convertedTimer;
		SampleToBones(*(node.single.animation), node.single.pose, convertedTimer);
		break;
	}
	case NodeType::BlendSpace1D:
	{
		uint32_t pointCount = node.bs1d.pointCount;
		float* weights = (float*)alloca(sizeof(float) * pointCount);
		for (int i = 0; i < pointCount; i++)
		{
			weights[i] = node.bs1d.points[i].weight;
			float currentPointTimer = node.bs1d.timer * (node.bs1d.points[i].RawDuration() / node.bs1d.duration);
			if (node.bs1d.points[i].speed >= 0.0f)
				currentPointTimer = node.bs1d.points[i].animation->start + currentPointTimer;
			else
				currentPointTimer = node.bs1d.points[i].animation->end - currentPointTimer;
			SampleToBones(*(node.bs1d.points[i].animation), node.bs1d.points[i].pose, currentPointTimer);
		}
		for (int i = 0; i < node.bs1d.boneCount; i++)
		{
			glm::vec3* inpos = (glm::vec3*)alloca(sizeof(glm::vec3) * pointCount);
			glm::quat* inrot = (glm::quat*)alloca(sizeof(glm::quat) * pointCount);
			float* inscale = (float*)alloca(sizeof(float) * pointCount);
			for (int j = 0; j < pointCount; j++)
			{
				inpos[j] = node.bs1d.points[j].pose[i].position;
				inrot[j] = node.bs1d.points[j].pose[i].rotation;
				inscale[j] = node.bs1d.points[j].pose[i].scale;
			}
			Math::WeightedBlend(inpos, weights, pointCount, node.bs1d.pose[i].position);
			Math::WeightedBlend(inrot, weights, pointCount, node.bs1d.pose[i].rotation);
			Math::WeightedBlend(inscale, weights, pointCount, node.bs1d.pose[i].scale);
		}
		break;
	}
	case NodeType::BlendSpace2D:
	{
		uint32_t pointCount = node.bs2d.pointCount;
		float* weights = (float*)alloca(sizeof(float) * pointCount);
		for (int i = 0; i < pointCount; i++)
		{
			weights[i] = node.bs2d.points[i].weight;
			float currentPointTimer = node.bs2d.timer * (node.bs2d.points[i].RawDuration() / node.bs2d.duration);
			if (node.bs2d.points[i].speed >= 0.0f)
				currentPointTimer = node.bs2d.points[i].animation->start + currentPointTimer;
			else
				currentPointTimer = node.bs2d.points[i].animation->end - currentPointTimer;
			SampleToBones(*(node.bs2d.points[i].animation), node.bs2d.points[i].pose, currentPointTimer);
		}
		for (int i = 0; i < node.bs2d.boneCount; i++)
		{
			glm::vec3* inpos = (glm::vec3*)alloca(sizeof(glm::vec3) * pointCount);
			glm::quat* inrot = (glm::quat*)alloca(sizeof(glm::quat) * pointCount);
			float* inscale = (float*)alloca(sizeof(float) * pointCount);
			for (int j = 0; j < pointCount; j++)
			{
				inpos[j] = node.bs2d.points[j].pose[i].position;
				inrot[j] = node.bs2d.points[j].pose[i].rotation;
				inscale[j] = node.bs2d.points[j].pose[i].scale;
			}
			Math::WeightedBlend(inpos, weights, pointCount, node.bs2d.pose[i].position);
			Math::WeightedBlend(inrot, weights, pointCount, node.bs2d.pose[i].rotation);
			Math::WeightedBlend(inscale, weights, pointCount, node.bs2d.pose[i].scale);
		}
		break;
	}
	}
}

void sf::Animation::SampleToBones(sf::Animation::SkeletalAnimation& animation, sf::Transform* bones, float animationTime)
{
	for (Channel& channel : animation.channels)
	{
		Sampler& sampler = animation.samplers[channel.samplerIndex];
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
					case Channel::PathType::TRANSLATION:
					{
						glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
						bones[channel.bone].position = glm::vec3(trans);
						break;
					}
					case Channel::PathType::SCALE:
					{
						glm::vec4 scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
						bones[channel.bone].scale = glm::max(glm::max(scale.x, scale.y), scale.z);
						break;
					}
					case Channel::PathType::ROTATION:
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
}

void sf::Animation::AdvanceNode(sf::Animation::Node& node, float deltaTime)
{
	ComputeNodeWeights(node);
	ComputeNodeDuration(node);

	if (node.single.type != NodeType::Single &&
		node.bs1d.prevFrameDuration != -1.0f &&
		node.bs1d.prevFrameDuration != node.bs1d.duration)
		node.single.timer *= node.bs1d.duration / node.bs1d.prevFrameDuration;

	ComputeNodePose(node);

	node.single.timer += deltaTime;
	float offset = node.single.timer - node.single.duration;
	if (offset > 0.0f)
		node.single.timer = offset;
}