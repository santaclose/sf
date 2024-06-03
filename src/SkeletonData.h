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
		const char* name;
		std::vector<AnimationSampler> samplers;
		std::vector<AnimationChannel> channels;
		float start = std::numeric_limits<float>::max();
		float end = std::numeric_limits<float>::min();
	};

	struct BlendSpacePoint1D
	{
		uint32_t animationIndex;
		float animationSpeed;
		float x;
	};

	struct BlendSpace1D
	{
		std::vector<BlendSpacePoint1D> points;
		float x;
		float animationBlendingTimer = 0.0f;
	};

	struct BlendSpacePoint2D
	{
		uint32_t animationIndex;
		float animationSpeed;
		glm::vec2 pos;
	};

	struct BlendSpace2D
	{
		std::vector<BlendSpacePoint2D> points;
		glm::vec2 pos;
		float animationBlendingTimer = 0.0f;
		std::vector<float> blendMatrix;
	};

	struct SkeletonData
	{
		inline bool GetAnimate() { return m_animate; }
		inline void SetAnimate(bool value) { m_animate = value; }
		inline uint32_t GetAnimationIndex() { return m_animationIndex; }
		inline uint32_t GetAnimationCount() { return m_animations.size(); }
		inline void SetAnimationIndex(uint32_t value) { m_animationIndex = value; }
		inline int AddBlendSpace1D(const std::vector<BlendSpacePoint1D>& points, float x) { m_blendSpaces1D.push_back({ points, x }); return m_blendSpaces1D.size() - 1; }
		inline int AddBlendSpace2D(const std::vector<BlendSpacePoint2D>& points, const glm::vec2& pos) { m_blendSpaces2D.push_back({ points, pos }); return m_blendSpaces2D.size() - 1; }
		inline void SetBlendSpace1D(int id) { m_selectedBlendSpace = id; m_selectedBlendSpaceD = 1; }
		inline void SetBlendSpace2D(int id) { m_selectedBlendSpace = id; m_selectedBlendSpaceD = 2; }
		inline void SetCurrentBlendSpaceX(float x) { assert(m_selectedBlendSpaceD != -1); if (m_selectedBlendSpaceD == 1) m_blendSpaces1D[m_selectedBlendSpace].x = x; else m_blendSpaces2D[m_selectedBlendSpace].pos.x = x; }
		inline void SetCurrentBlendSpaceY(float y) { assert(m_selectedBlendSpaceD == 2); m_blendSpaces2D[m_selectedBlendSpace].pos.y = y; }
		inline void SetCurrentBlendSpacePos(const glm::vec2& pos) { assert(m_selectedBlendSpaceD == 2); m_blendSpaces2D[m_selectedBlendSpace].pos = pos; }
		inline const BlendSpace1D& GetCurrentBlendSpace1D() { assert(m_selectedBlendSpaceD == 1); return m_blendSpaces1D[m_selectedBlendSpace]; }
		inline const BlendSpace2D& GetCurrentBlendSpace2D() { assert(m_selectedBlendSpaceD == 2); return m_blendSpaces2D[m_selectedBlendSpace]; }

		inline SkeletalAnimation& GetCurrentAnimation() { return m_animations[m_animationIndex]; }
		void UpdateAnimation(float deltaTime, float* outWeights = nullptr);
		void UpdateAnimationUsingBlendspace(float deltaTime, float* outWeights = nullptr);
		void UpdateAnimationBlended(const std::vector<float>& weights, const std::vector<float>& speeds, const std::vector<uint32_t>& indices, const std::vector<float>& durations, float& timer, float deltaTime);

		bool m_animate = false;
		uint32_t m_animationIndex = 0;
		float m_animationTime = 0.0f;
		int m_selectedBlendSpace = -1;
		int m_selectedBlendSpaceD = -1;
		float m_previousBlendedDuration = -1.0f;

		std::vector<Bone> m_bones;
		std::vector<SkeletalAnimation> m_animations;
		std::vector<glm::mat4> m_skinningMatrices;

		std::vector<BlendSpace1D> m_blendSpaces1D;
		std::vector<BlendSpace2D> m_blendSpaces2D;
	};
}