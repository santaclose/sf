#pragma once

#include <Components/SphereCollider.h>
#include <Components/CapsuleCollider.h>
#include <Components/BoxCollider.h>
#include <Renderer/Renderer.h>
#include <cassert>
#include <glm/gtx/norm.hpp>

namespace sf::Geometry
{
	inline glm::vec3 ClosestPointPointSegment(const glm::vec3& point, const glm::vec3& segA, const glm::vec3& segB)
	{
		glm::vec3 AB = segB - segA;
		float t = glm::dot(point - segA, AB) / glm::dot(AB, AB);
		return segA + glm::clamp(t, 0.0f, 1.0f) * AB;
	}

	inline void ClosestPointsSegmentSegment(
		const glm::vec3& seg0A, const glm::vec3& seg0B,
		const glm::vec3& seg1A, const glm::vec3& seg1B, glm::vec3& out0, glm::vec3& out1)
	{
#define segA seg0A
#define segB seg0B
#define segC seg1A
#define segD seg1B
		glm::vec3 segDC = segD - segC; float lineDirSqrMag = glm::dot(segDC, segDC);
		glm::vec3 inPlaneA = segA - ((glm::dot(segA - segC, segDC) / lineDirSqrMag) * segDC);
		glm::vec3 inPlaneB = segB - ((glm::dot(segB - segC, segDC) / lineDirSqrMag) * segDC);
		glm::vec3 inPlaneBA = inPlaneB - inPlaneA;
		float t = glm::dot(segC - inPlaneA, inPlaneBA) / glm::dot(inPlaneBA, inPlaneBA);
		t = (inPlaneA != inPlaneB) ? t : 0.0f; // Zero's t if parallel
		glm::vec3 AB = segB - segA;
		glm::vec3 segABtoLineCD = segA + glm::clamp(t, 0.0f, 1.0f) * AB;
		{
			glm::vec3 ba = segD - segC; t = glm::dot(segABtoLineCD - segC, ba) / glm::dot(ba, ba);
			out1 = segC + glm::clamp(t, 0.0f, 1.0f) * ba;
		}
		{
			glm::vec3 ba = segB - segA; t = glm::dot(out1 - segA, ba) / glm::dot(ba, ba);
			out0 = segA + glm::clamp(t, 0.0f, 1.0f) * ba;
		}
#undef segA
#undef segB
#undef segC
#undef segD
	}

	void ClosestPointsSegmentAABB(
		const glm::vec3& segA, const glm::vec3& segB,
		const glm::vec3& boxMin, const glm::vec3& boxMax,
		glm::vec3& outSegment, glm::vec3& outAABB, bool& intersects)
	{
		// segA is closer to inner face
		if (
			(segA.x > boxMax.x && segA.y >= boxMin.y && segA.y <= boxMax.y && segA.z >= boxMin.z && segA.z <= boxMax.z && (segB.x - segA.x) > 0.0f) ||
			(segA.x < boxMin.x && segA.y >= boxMin.y && segA.y <= boxMax.y && segA.z >= boxMin.z && segA.z <= boxMax.z && (segB.x - segA.x) < 0.0f) ||
			(segA.y > boxMax.y && segA.x >= boxMin.x && segA.x <= boxMax.x && segA.z >= boxMin.z && segA.z <= boxMax.z && (segB.y - segA.y) > 0.0f) ||
			(segA.y < boxMin.y && segA.x >= boxMin.x && segA.x <= boxMax.x && segA.z >= boxMin.z && segA.z <= boxMax.z && (segB.y - segA.y) < 0.0f) ||
			(segA.z > boxMax.z && segA.x >= boxMin.x && segA.x <= boxMax.x && segA.y >= boxMin.y && segA.y <= boxMax.y && (segB.z - segA.z) > 0.0f) ||
			(segA.z < boxMin.z && segA.x >= boxMin.x && segA.x <= boxMax.x && segA.y >= boxMin.y && segA.y <= boxMax.y && (segB.z - segA.z) < 0.0f))
		{
			outAABB = glm::clamp(segA, boxMin, boxMax);
			outSegment = segA;
			intersects = false;
		}
		// segB is closer to inner face
		else if (
			(segB.x > boxMax.x && segB.y >= boxMin.y && segB.y <= boxMax.y && segB.z >= boxMin.z && segB.z <= boxMax.z && (segA.x - segB.x) > 0.0f) ||
			(segB.x < boxMin.x && segB.y >= boxMin.y && segB.y <= boxMax.y && segB.z >= boxMin.z && segB.z <= boxMax.z && (segA.x - segB.x) < 0.0f) ||
			(segB.y > boxMax.y && segB.x >= boxMin.x && segB.x <= boxMax.x && segB.z >= boxMin.z && segB.z <= boxMax.z && (segA.y - segB.y) > 0.0f) ||
			(segB.y < boxMin.y && segB.x >= boxMin.x && segB.x <= boxMax.x && segB.z >= boxMin.z && segB.z <= boxMax.z && (segA.y - segB.y) < 0.0f) ||
			(segB.z > boxMax.z && segB.x >= boxMin.x && segB.x <= boxMax.x && segB.y >= boxMin.y && segB.y <= boxMax.y && (segA.z - segB.z) > 0.0f) ||
			(segB.z < boxMin.z && segB.x >= boxMin.x && segB.x <= boxMax.x && segB.y >= boxMin.y && segB.y <= boxMax.y && (segA.z - segB.z) < 0.0f))
		{
			outAABB = glm::clamp(segB, boxMin, boxMax);
			outSegment = segB;
			intersects = false;
		}
		// closest point in every edge
		else
		{
			glm::vec3 corners[8];
			corners[0] = boxMin;
			corners[1] = glm::vec3(boxMax.x, boxMin.y, boxMin.z);
			corners[2] = glm::vec3(boxMax.x, boxMin.y, boxMax.z);
			corners[3] = glm::vec3(boxMin.x, boxMin.y, boxMax.z);

			corners[4] = glm::vec3(boxMin.x, boxMax.y, boxMin.z);
			corners[5] = glm::vec3(boxMax.x, boxMax.y, boxMin.z);
			corners[6] = boxMax;
			corners[7] = glm::vec3(boxMin.x, boxMax.y, boxMax.z);

			glm::vec3 outAABBArray[12];
			glm::vec3 outSegmentArray[12];
			ClosestPointsSegmentSegment(segA, segB, corners[0], corners[1], outSegmentArray[0], outAABBArray[0]);
			ClosestPointsSegmentSegment(segA, segB, corners[1], corners[2], outSegmentArray[1], outAABBArray[1]);
			ClosestPointsSegmentSegment(segA, segB, corners[2], corners[3], outSegmentArray[2], outAABBArray[2]);
			ClosestPointsSegmentSegment(segA, segB, corners[3], corners[0], outSegmentArray[3], outAABBArray[3]);
			ClosestPointsSegmentSegment(segA, segB, corners[4], corners[5], outSegmentArray[4], outAABBArray[4]);
			ClosestPointsSegmentSegment(segA, segB, corners[5], corners[6], outSegmentArray[5], outAABBArray[5]);
			ClosestPointsSegmentSegment(segA, segB, corners[6], corners[7], outSegmentArray[6], outAABBArray[6]);
			ClosestPointsSegmentSegment(segA, segB, corners[7], corners[4], outSegmentArray[7], outAABBArray[7]);
			ClosestPointsSegmentSegment(segA, segB, corners[0], corners[4], outSegmentArray[8], outAABBArray[8]);
			ClosestPointsSegmentSegment(segA, segB, corners[1], corners[5], outSegmentArray[9], outAABBArray[9]);
			ClosestPointsSegmentSegment(segA, segB, corners[2], corners[6], outSegmentArray[10], outAABBArray[10]);
			ClosestPointsSegmentSegment(segA, segB, corners[3], corners[7], outSegmentArray[11], outAABBArray[11]);

			int closest = 0;
			float distanceSquared = glm::distance2(outSegmentArray[0], outAABBArray[0]);
			for (int i = 1; i < 12; i++)
			{
				float d2 = glm::distance2(outSegmentArray[i], outAABBArray[i]);
				if (d2 < distanceSquared)
				{
					closest = i;
					distanceSquared = d2;
				}
			}

			outAABB = outAABBArray[closest];
			outSegment = outSegmentArray[closest];
			intersects = false;
			for (int i = 0; i < 12; i++)
			{
				if (outSegmentArray[i].x >= boxMin.x && outSegmentArray[i].x <= boxMax.x &&
					outSegmentArray[i].y >= boxMin.y && outSegmentArray[i].y <= boxMax.y &&
					outSegmentArray[i].z >= boxMin.z && outSegmentArray[i].z <= boxMax.z)
				{
					intersects = true;
					return;
				}
			}
		}
	}

	inline bool IntersectSphereSphere(const SphereCollider& sphereA, const SphereCollider& sphereB)
	{
		assert(sphereA.radius >= 0.0f && sphereB.radius >= 0.0f);
		float radiusSum = sphereA.radius + sphereB.radius;
		return glm::distance2(sphereA.center, sphereB.center) < radiusSum * radiusSum;
	}

	inline bool IntersectSphereCapsule(const SphereCollider& sphere, const CapsuleCollider& capsule)
	{
		assert(sphere.radius >= 0.0f && capsule.radius >= 0.0f);
		glm::vec3 pointOnLine = ClosestPointPointSegment(sphere.center, capsule.centerA, capsule.centerB);
		float radiusSum = capsule.radius + sphere.radius;
		return glm::distance2(pointOnLine, sphere.center) < radiusSum * radiusSum;
	}
	
	inline bool IntersectSphereBox(const SphereCollider& sphere, const BoxCollider& box)
	{
		assert(sphere.radius >= 0.0f && box.size.x >= 0.0f && box.size.y >= 0.0f && box.size.z >= 0.0f);
		glm::quat undoRotationQuat = glm::conjugate(box.orientation);
		glm::vec3 localSphereCenter = undoRotationQuat * (sphere.center - box.center);
		glm::vec3 clampedPoint = glm::clamp(localSphereCenter, -box.size * 0.5f, box.size * 0.5f);
		return glm::distance2(localSphereCenter, clampedPoint) <= sphere.radius * sphere.radius;
	}

	inline bool IntersectCapsuleCapsule(const CapsuleCollider& capsuleA, const CapsuleCollider& capsuleB)
	{
		glm::vec3 a, b;
		ClosestPointsSegmentSegment(capsuleA.centerA, capsuleA.centerB, capsuleB.centerA, capsuleB.centerB, a, b);
		float radiusSum = capsuleA.radius + capsuleB.radius;
		return glm::distance2(a, b) < radiusSum * radiusSum;
	}

	inline bool IntersectCapsuleBox(const CapsuleCollider& capsule, const BoxCollider& box)
	{
		glm::quat undoRotationQuat = glm::conjugate(box.orientation);
		glm::vec3 localCapsuleA = undoRotationQuat * (capsule.centerA - box.center);
		glm::vec3 localCapsuleB = undoRotationQuat * (capsule.centerB - box.center);

		glm::vec3 boxMin = -box.size * 0.5f;
		glm::vec3 boxMax = box.size * 0.5f;

		glm::vec3 closestSegmentPoint, closestAABBPoint;
		bool segmentIntersectsAABB;
		ClosestPointsSegmentAABB(localCapsuleA, localCapsuleB, boxMin, boxMax, closestSegmentPoint, closestAABBPoint, segmentIntersectsAABB);
		return segmentIntersectsAABB || glm::distance2(closestSegmentPoint, closestAABBPoint) <= (capsule.radius * capsule.radius);
	}

	inline bool Intersect2dSegmentAARectangle(const glm::vec2& segA, const glm::vec2& segB, const glm::vec2& boxMin, const glm::vec2& boxMax)
	{
		float t0 = 0, t1 = 1;
		float dx = segB.x - segA.x, dy = segB.y - segA.y;

		float pArray[4] = { -dx, dx, -dy, dy };
		float qArray[4] = { -(boxMin.x - segA.x), (boxMax.x - segA.x), -(boxMin.y - segA.y), (boxMax.y - segA.y) };

		for (int edge = 0; edge < 4; edge++)
		{
			float r = qArray[edge] / pArray[edge];

			if (pArray[edge] == 0 && qArray[edge] < 0)
				return false;

			if (pArray[edge] < 0)
			{
				if (r > t1)
					return false;
				else if
					(r > t0) t0 = r;
			}
			else if (pArray[edge] > 0)
			{
				if (r < t0)
					return false;
				else if (r < t1)
					t1 = r;
			}
		}
		return true;
	}

	inline bool IntersectAABBAnyBoxSegment(const glm::vec3& boxMin, const glm::vec3& boxMax, const BoxCollider& boxCol)
	{
		glm::vec3 edges[12][2];
		edges[0][0] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[0][1] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));
		edges[1][0] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));
		edges[1][1] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));
		edges[2][0] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));
		edges[2][1] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[3][0] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[3][1] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[4][0] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[4][1] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));
		edges[5][0] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));
		edges[5][1] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));
		edges[6][0] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));
		edges[6][1] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[7][0] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[7][1] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[8][0] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[8][1] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[9][0] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[9][1] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, -boxCol.size.z * 0.5f));
		edges[10][0] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));
		edges[10][1] = boxCol.center + (boxCol.orientation * glm::vec3(-boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));
		edges[11][0] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, -boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));
		edges[11][1] = boxCol.center + (boxCol.orientation * glm::vec3(+boxCol.size.x * 0.5f, +boxCol.size.y * 0.5f, +boxCol.size.z * 0.5f));

		for (int i = 0; i < 12; i++)
		{
			bool xyIntersects = Intersect2dSegmentAARectangle(
				glm::vec2(edges[i][0].x, edges[i][0].y), glm::vec2(edges[i][1].x, edges[i][1].y),
				glm::vec2(boxMin.x, boxMin.y), glm::vec2(boxMax.x, boxMax.y));
			bool xzIntersects = Intersect2dSegmentAARectangle(
				glm::vec2(edges[i][0].x, edges[i][0].z), glm::vec2(edges[i][1].x, edges[i][1].z),
				glm::vec2(boxMin.x, boxMin.z), glm::vec2(boxMax.x, boxMax.z));
			bool zyIntersects = Intersect2dSegmentAARectangle(
				glm::vec2(edges[i][0].z, edges[i][0].y), glm::vec2(edges[i][1].z, edges[i][1].y),
				glm::vec2(boxMin.z, boxMin.y), glm::vec2(boxMax.z, boxMax.y));

			if (xzIntersects && xyIntersects && zyIntersects)
				return true;
		}
		return false;
	}

	inline bool IntersectBoxBox(const BoxCollider& boxA, const BoxCollider& boxB)
	{
		// Need to test both cases: AABB vs Box and Box vs AABB because AABB can be completely inside the box
		// and IntersectAABBAnyBoxSegment will be false
		glm::quat undoRotationQuatA = glm::conjugate(boxA.orientation);
		glm::quat undoRotationQuatB = glm::conjugate(boxB.orientation);

		BoxCollider bRelativeToA;
		bRelativeToA.size = boxB.size;
		bRelativeToA.center = undoRotationQuatA * (boxB.center - boxA.center);
		bRelativeToA.orientation = undoRotationQuatA * boxB.orientation;
		BoxCollider aRelativeToB;
		aRelativeToB.size = boxA.size;
		aRelativeToB.center = undoRotationQuatB * (boxA.center - boxB.center);
		aRelativeToB.orientation = undoRotationQuatB * boxA.orientation;
		return IntersectAABBAnyBoxSegment(-boxA.size * 0.5f, boxA.size * 0.5f, bRelativeToA) || IntersectAABBAnyBoxSegment(-boxB.size * 0.5f, boxB.size * 0.5f, aRelativeToB);
	}
}