#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace sf
{
	struct Bone
	{
		int32_t parent;
		glm::mat4 localMatrix;
		glm::mat4 invModelMatrix;
		glm::mat4 localMatrixAnim;
		glm::vec3 translationAnim;
		glm::quat rotationAnim;
		float scaleAnim = 1.0f;
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