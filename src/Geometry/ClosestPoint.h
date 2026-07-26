#pragma once

#include <Components/SphereCollider.h>
#include <Components/CapsuleCollider.h>
#include <Components/BoxCollider.h>
#include <Components/MeshCollider.h>

namespace sf::Geometry
{
	inline glm::vec3 ClosestPointPointSegment(
		const glm::vec3& point,
		const glm::vec3& segA, const glm::vec3& segB)
	{
		glm::vec3 AB = segB - segA;
		float t = glm::dot(point - segA, AB) / glm::dot(AB, AB);
		return segA + glm::clamp(t, 0.0f, 1.0f) * AB;
	}

	inline glm::vec3 ClosestPointPointPlane(
		const glm::vec3& point,
		const glm::vec3& planeUnormal, const glm::vec3& planePoint)
	{
		float distance = glm::dot(point - planePoint, planeUnormal);
		return point - distance * planeUnormal;
	}

	inline glm::vec3 ClosestPointPointTriangle(
		const glm::vec3& point,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC)
	{
		const glm::vec3 ab = triB - triA;
		const glm::vec3 ac = triC - triA;
		const glm::vec3 ap = point - triA;

		// Corners
		const float d1 = glm::dot(ab, ap);
		const float d2 = glm::dot(ac, ap);
		if (d1 <= 0.0f && d2 <= 0.0f)
			return triA;

		const glm::vec3 bp = point - triB;
		const float d3 = glm::dot(ab, bp);
		const float d4 = glm::dot(ac, bp);
		if (d3 >= 0.0f && d4 <= d3)
			return triB;

		const glm::vec3 cp = point - triC;
		const float d5 = glm::dot(ab, cp);
		const float d6 = glm::dot(ac, cp);
		if (d6 >= 0.0f && d5 <= d6)
			return triC;

		// Edges
		const float vc = d1 * d4 - d3 * d2;
		if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
		{
			const float v = d1 / (d1 - d3);
			return triA + v * ab;
		}

		const float vb = d5 * d2 - d1 * d6;
		if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
		{
			const float v = d2 / (d2 - d6);
			return triA + v * ac;
		}

		const float va = d3 * d6 - d5 * d4;
		if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
		{
			const float v = (d4 - d3) / ((d4 - d3) + (d5 - d6));
			return triB + v * (triC - triB);
		}

		// Face
		const float denom = 1.f / (va + vb + vc);
		const float v = vb * denom;
		const float w = vc * denom;
		return triA + v * ab + w * ac;
	}

	inline glm::vec3 ClosestPointPointAABB(
		const glm::vec3& point,
		const glm::vec3& boxMin, const glm::vec3& boxMax)
	{
		return glm::vec3(
			glm::clamp(point.x, boxMin.x, boxMax.x),
			glm::clamp(point.y, boxMin.y, boxMax.y),
			glm::clamp(point.z, boxMin.z, boxMax.z));
	}

	inline glm::vec3 ClosestPointPointBox(
		const glm::vec3& point,
		const BoxCollider& box)
	{
		glm::quat undoRotationQuat = glm::conjugate(box.orientation);
		glm::vec3 localPoint = undoRotationQuat * (point - box.center);
		return box.orientation * glm::clamp(localPoint, -box.size * 0.5f, box.size * 0.5f);
	}

	inline void ClosestPointsSegmentSegment(
		const glm::vec3& seg0A, const glm::vec3& seg0B,
		const glm::vec3& seg1A, const glm::vec3& seg1B,
		glm::vec3& out0, glm::vec3& out1)
	{
		glm::vec3 seg1 = seg1B - seg1A;
		float lineDirSqrMag = glm::dot(seg1, seg1);
		glm::vec3 inPlaneA = seg0A - ((glm::dot(seg0A - seg1A, seg1) / lineDirSqrMag) * seg1);
		glm::vec3 inPlaneB = seg0B - ((glm::dot(seg0B - seg1A, seg1) / lineDirSqrMag) * seg1);
		glm::vec3 inPlaneBA = inPlaneB - inPlaneA;
		float t = glm::dot(seg1A - inPlaneA, inPlaneBA) / glm::dot(inPlaneBA, inPlaneBA);
		t = (inPlaneA != inPlaneB) ? t : 0.0f; // Zero's t if parallel
		glm::vec3 AB = seg0B - seg0A;
		glm::vec3 seg0toLine1 = seg0A + glm::clamp(t, 0.0f, 1.0f) * AB;
		{
			glm::vec3 ba = seg1B - seg1A; t = glm::dot(seg0toLine1 - seg1A, ba) / glm::dot(ba, ba);
			out1 = seg1A + glm::clamp(t, 0.0f, 1.0f) * ba;
		}
		{
			glm::vec3 ba = seg0B - seg0A; t = glm::dot(out1 - seg0A, ba) / glm::dot(ba, ba);
			out0 = seg0A + glm::clamp(t, 0.0f, 1.0f) * ba;
		}
	}

	inline void ClosestPointsSegmentPlane(
		const glm::vec3& segA, const glm::vec3& segB,
		const glm::vec3& planeUnormal, const glm::vec3& planePoint,
		glm::vec3& outSeg, glm::vec3& outPlane)
	{
		const glm::vec3 dir = segB - segA;

		const float da = glm::dot(planeUnormal, segA - planePoint);
		const float db = glm::dot(planeUnormal, segB - planePoint);

		// Segment crosses (or touches) the plane.
		if ((da <= 0.0f && db >= 0.0f) || (da >= 0.0f && db <= 0.0f))
		{
			const float t = da / (da - db); // Safe unless segment is coplanar.
			outSeg = segA + t * dir;
			outPlane = outSeg;
			return;
		}

		// Choose the endpoint closest to the plane.
		if (glm::abs(da) < glm::abs(db))
			outSeg = segA;
		else
			outSeg = segB;

		// Orthogonal projection onto the plane.
		const float invLenSq = 1.0f / glm::dot(planeUnormal, planeUnormal);
		outPlane = outSeg - planeUnormal * (glm::dot(planeUnormal, outSeg - planePoint) * invLenSq);
	}

	inline void ClosestPointsSegmentTriangle(
		const glm::vec3& segA, const glm::vec3& segB,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC,
		glm::vec3& outSeg, glm::vec3& outTri)
	{
		// Plane normal (doesn't need to be normalized)
		const glm::vec3 planeNormal = glm::cross(triB - triA, triC - triA);

		// First, compute the closest point between the segment and the triangle's plane.
		glm::vec3 planeSeg, planePoint;
		ClosestPointsSegmentPlane(
			segA, segB,
			planeNormal, triA,
			planeSeg, planePoint);

		// Is the projected/intersection point inside the triangle?
		const glm::vec3 edgePerpA = glm::cross(planeNormal, triB - triA);
		const glm::vec3 edgePerpB = glm::cross(planeNormal, triC - triB);
		const glm::vec3 edgePerpC = glm::cross(planeNormal, triA - triC);

		if (glm::dot(edgePerpA, planePoint - triA) >= 0.0f &&
			glm::dot(edgePerpB, planePoint - triB) >= 0.0f &&
			glm::dot(edgePerpC, planePoint - triC) >= 0.0f)
		{
			outSeg = planeSeg;
			outTri = planePoint;
			return;
		}

		// Otherwise, the closest point must lie on an edge.
		glm::vec3 edgeClosest[6];

		ClosestPointsSegmentSegment(segA, segB, triA, triB,
			edgeClosest[0], edgeClosest[1]);
		ClosestPointsSegmentSegment(segA, segB, triB, triC,
			edgeClosest[2], edgeClosest[3]);
		ClosestPointsSegmentSegment(segA, segB, triC, triA,
			edgeClosest[4], edgeClosest[5]);

		float minDist2 = glm::distance2(edgeClosest[0], edgeClosest[1]);
		outSeg = edgeClosest[0];
		outTri = edgeClosest[1];

		for (int i = 2; i < 6; i += 2)
		{
			float dist2 = glm::distance2(edgeClosest[i], edgeClosest[i + 1]);
			if (dist2 < minDist2)
			{
				minDist2 = dist2;
				outSeg = edgeClosest[i];
				outTri = edgeClosest[i + 1];
			}
		}
	}

	inline void ClosestPointsSegmentAABB(
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

	inline glm::vec2 ClosestPointPointSegment2D(
		const glm::vec2& point,
		const glm::vec2& segA, const glm::vec2& segB)
	{
		glm::vec2 ab = segB - segA;
		glm::vec2 ap = point - segA;
		float l2 = glm::dot(ab, ab);

		if (l2 < 0.00001f)
			return segA;

		float t = glm::dot(ab, ap) / l2;
		t = glm::clamp(t, 0.0f, 1.0f);
		return segA + t * ab;
	}

	inline glm::vec2 ClosestPointPointTriangle2D(
		const glm::vec2& point,
		const glm::vec2& triA, const glm::vec2& triB, const glm::vec2& triC)
	{
		float abp = Math::Cross2D(triB - triA, point - triA);
		float bcp = Math::Cross2D(triC - triB, point - triB);
		float cap = Math::Cross2D(triA - triC, point - triC);

		if (abp < 0.0f && bcp < 0.0f)
			return triB;

		if (bcp < 0.0f && cap < 0.0f)
			return triC;

		if (cap < 0.0f && abp < 0.0f)
			return triA;

		if (abp < 0.0f)
			return ClosestPointPointSegment2D(point, triA, triB);

		if (bcp < 0.0f)
			return ClosestPointPointSegment2D(point, triB, triC);

		if (cap < 0.0f)
			return ClosestPointPointSegment2D(point, triC, triA);

		return point;
	}
}