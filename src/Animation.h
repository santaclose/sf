#pragma once

#include <vector>
#include <Components/Transform.h>
#include <Math.hpp>

namespace sf::Animation
{
	struct Channel
	{
		enum PathType { TRANSLATION, ROTATION, SCALE };
		PathType path;
		uint32_t bone;
		uint32_t samplerIndex;
	};

	struct Sampler
	{
		enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
		InterpolationType interpolation;
		std::vector<float> inputs;
		std::vector<glm::vec4> outputsVec4;
	};

	struct SkeletalAnimation
	{
		const char* name;
		std::vector<Sampler> samplers;
		std::vector<Channel> channels;
		float start = std::numeric_limits<float>::max();
		float end = std::numeric_limits<float>::min();
	};

	enum NodeType
	{
		Single, BlendSpace1D, BlendSpace2D
	};

	struct NodeSingle
	{
		NodeType type;
		float timer;
		float weight;
		float duration;
		Transform* pose;
		uint32_t boneCount;
		SkeletalAnimation* animation;
		float speed;
		inline float ComputeDuration()
		{
			return (animation->end - animation->start) / glm::abs(speed);
		}
		inline float RawDuration()
		{
			return (animation->end - animation->start);
		}
	};

	struct BlendSpacePoint1D
	{
		SkeletalAnimation* animation;
		float speed;
		float weight;
		Transform* pose;
		float pos;
		inline float ComputeDuration()
		{
			return (animation->end - animation->start) / glm::abs(speed);
		}
		inline float RawDuration()
		{
			return (animation->end - animation->start);
		}
	};

	struct NodeBlendSpace1D
	{
		NodeType type;
		float timer;
		float weight;
		float duration;
		Transform* pose;
		uint32_t boneCount;
		BlendSpacePoint1D* points;
		uint32_t pointCount;
		float prevFrameDuration; /* Needed to adjust timer while blending animations */
		float pos;
		float* weightsQuery; /* nullptr or array with one weight per point */
	};

	struct BlendSpacePoint2D
	{
		SkeletalAnimation* animation;
		float speed;
		float weight;
		Transform* pose;
		glm::vec2 pos;
		inline float ComputeDuration()
		{
			return (animation->end - animation->start) / glm::abs(speed);
		}
		inline float RawDuration()
		{
			return (animation->end - animation->start);
		}
	};

	struct NodeBlendSpace2D
	{
		NodeType type;
		float timer;
		float weight;
		float duration;
		Transform* pose;
		uint32_t boneCount;
		BlendSpacePoint2D* points;
		uint32_t pointCount;
		float prevFrameDuration; /* Needed to adjust timer while blending animations */
		glm::vec2 pos;
		float* blendMatrix; /* Used for generating point weights */
		bool needsToComputeBlendMatrix; /* Needs to be computed whenever a point position changes */
		float* weightsQuery; /* nullptr or array with one weight per point */
	};

	union Node
	{
		NodeSingle single;
		NodeBlendSpace1D bs1d;
		NodeBlendSpace2D bs2d;
	};

	void ComputeNodeWeights(Node& node);
	void ComputeNodeDuration(Node& node);
	void ComputeNodePose(Node& node);

	void SampleToBones(SkeletalAnimation& animation, Transform* bones, float animationTime);
	void AdvanceNode(Node& node, float deltaTime);
}