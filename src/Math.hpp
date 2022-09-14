#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace sf {

	glm::vec3 operator/(const glm::vec3& v, const float& f);
	glm::vec3 operator*(const glm::vec3& v, const float& f);

	namespace Math {

		extern const float Pi;

		int Mod(int a, int b);

		void WeightedBlend(const float* input, const float* weights, int count, float& output);
		void WeightedBlend(const glm::vec3* input, const float* weights, int count, glm::vec3& output);
		void WeightedBlend(const glm::quat* input, const float* weights, int count, glm::quat& output);

		bool RayTriIntersect(
			const glm::vec3& origin,
			const glm::vec3& dir,
			const glm::vec3& t0, const glm::vec3& t1, const glm::vec3& t2,
			float* out_t = nullptr);

		bool RayAABBIntersect(
			const glm::vec3& origin,
			const glm::vec3& dir,
			const glm::vec3& min,
			const glm::vec3& max,
			glm::vec3* out = nullptr);

		float LineSegmentPointDistance2D(
			const glm::vec2& p,
			const glm::vec2& l0, const glm::vec2& l1);

		bool TriPointIntersect2D(
			const glm::vec2& p,
			const glm::vec2& t0, const glm::vec2& t1, const glm::vec2& t2);

		float TriPointDistance2D(
			const glm::vec2& p,
			const glm::vec2& t0, const glm::vec2& t1, const glm::vec2& t2);

		inline float PlanePointDistance(
			glm::vec3 planeNormal, glm::vec3 planePoint,
			glm::vec3 point)
		{
			return glm::dot(glm::normalize(planeNormal), point - planePoint);
		}

		inline bool PointInsideAABB(
			const glm::vec3& point,
			const glm::vec3& min,
			const glm::vec3& max)
		{
			return point.x > min.x && point.y > min.y && point.z > min.z && point.x < max.x && point.y < max.y && point.z < max.z;
		}
	}
}