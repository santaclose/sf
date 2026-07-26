#pragma once

#include <Math.hpp>

namespace sf::Geometry
{
	inline glm::vec3 Barycentric(
		const glm::vec3& point,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC)
	{
		glm::vec3 v0 = triB - triA, v1 = triC - triA, v2 = point - triA;
		float d00 = glm::dot(v0, v0);
		float d01 = glm::dot(v0, v1);
		float d11 = glm::dot(v1, v1);
		float d20 = glm::dot(v2, v0);
		float d21 = glm::dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;
		v0.x = (d11 * d20 - d01 * d21) / denom;
		v0.y = (d00 * d21 - d01 * d20) / denom;
		v0.z = 1.0f - v0.x - v0.y;
		return v0;
	}

	inline bool SolveQuadratic(float a, float b, float c, float& t0, float& t1)
	{
		float disc = b * b - 4.0f * a * c;
		if (disc < 0.0f)
			return false;

		float s = glm::sqrt(disc);
		float inv = 0.5f / a;

		t0 = (-b - s) * inv;
		t1 = (-b + s) * inv;

		if (t0 > t1)
			std::swap(t0, t1);

		return true;
	}

	inline bool PointLandsInTriangle(
		const glm::vec3& point,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC)
	{
		glm::vec3 normal = glm::cross(triB - triA, triC - triA);
		bool a = glm::dot(point - triA, glm::cross(normal, triB - triA)) > 0.0f;
		bool b = glm::dot(point - triB, glm::cross(normal, triC - triB)) > 0.0f;
		bool c = glm::dot(point - triC, glm::cross(normal, triA - triC)) > 0.0f;
		return a && b && c;
	}

	inline bool PointInAABB(
		const glm::vec3& point,
		const glm::vec3& boxMin, const glm::vec3& boxMax)
	{
		return
			point.x > boxMin.x && point.y > boxMin.y && point.z > boxMin.z &&
			point.x < boxMax.x && point.y < boxMax.y && point.z < boxMax.z;
	}

	inline bool PointInTriangle2D(
		const glm::vec2& point,
		const glm::vec2& triA, const glm::vec2& triB, const glm::vec2& triC)
	{
		float abp = Math::Cross2D(triB - triA, point - triA);
		float bcp = Math::Cross2D(triC - triB, point - triB);
		float cap = Math::Cross2D(triA - triC, point - triC);

		return abp > 0.0f && bcp > 0.0f && cap > 0.0f;
	}
}