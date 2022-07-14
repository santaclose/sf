#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace sf
{
	struct Bone
	{
		int32_t parent = -1;
		glm::mat4 localMatrix = glm::mat4(1.0f);
		glm::mat4 invModelMatrix;
		glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		float scale = 1.0f;
	};

	struct AnimationChannel
	{
		enum PathType { TRANSLATION, ROTATION, SCALE };
		PathType path;
		uint32_t bone;
		uint32_t samplerIndex;
	};

	struct AnimationSampler
	{
		enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
		InterpolationType interpolation;
		std::vector<float> inputs;
		std::vector<glm::vec4> outputsVec4;
	};

	struct SkeletalAnimation
	{
		std::vector<AnimationSampler> samplers;
		std::vector<AnimationChannel> channels;
		float start = std::numeric_limits<float>::max();
		float end = std::numeric_limits<float>::min();
	};

	struct SkeletonData
	{
		bool animate = false;
		uint32_t animationIndex = 0;
		float animationTime = 0.0f;

		std::vector<Bone> bones;
		std::vector<SkeletalAnimation> animations;
		std::vector<glm::mat4> skinningMatrices;

		void ClampAnimationTime();
		void UpdateAnimation();
	};
}