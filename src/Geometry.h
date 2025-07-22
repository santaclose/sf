#pragma once

#include <Components/SphereCollider.h>
#include <Components/CapsuleCollider.h>
#include <Components/BoxCollider.h>
#include <Components/MeshCollider.h>
#include <Renderer/Renderer.h>
#include <cassert>
#include <glm/gtx/norm.hpp>

namespace sf::Geometry
{
	inline bool IntersectPointAABB(
		const glm::vec3& point,
		const glm::vec3& boxMin, const glm::vec3& boxMax)
	{
		return point.x > boxMin.x && point.y > boxMin.y && point.z > boxMin.z && point.x < boxMax.x && point.y < boxMax.y && point.z < boxMax.z;
	}

	inline bool IntersectRayTriangle(
		const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC, float* out_t)
	{
		glm::vec3 normal = glm::cross(triB - triA, triC - triA);
		float q = glm::dot(normal, rayDir);
		if (q == 0) return false;

		float d = -glm::dot(normal, triA);
		float t = -(glm::dot(normal, rayOrigin) + d) / q;
		if (t < 0) return false;

		glm::vec3 hit_point = rayOrigin + rayDir * t;

		glm::vec3 edge0 = triB - triA;
		glm::vec3 VP0 = hit_point - triA;
		if (glm::dot(normal, glm::cross(edge0, VP0)) < 0)
			return false;

		glm::vec3 edge1 = triC - triB;
		glm::vec3 VP1 = hit_point - triB;
		if (glm::dot(normal, glm::cross(edge1, VP1)) < 0)
			return false;

		glm::vec3 edge2 = triA - triC;
		glm::vec3 VP2 = hit_point - triC;
		if (glm::dot(normal, glm::cross(edge2, VP2)) < 0)
			return false;

		if (out_t) *out_t = t;
		return true;
	}

	inline bool IntersectRayAABB(
		const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		const glm::vec3& boxMin, const glm::vec3& boxMax, glm::vec3* out)
	{
		glm::vec3 dirfrac;

		dirfrac.x = 1.0f / (rayDir.x == 0 ? 0.00000001f : rayDir.x);
		dirfrac.y = 1.0f / (rayDir.y == 0 ? 0.00000001f : rayDir.y);
		dirfrac.z = 1.0f / (rayDir.z == 0 ? 0.00000001f : rayDir.z);

		float t1 = (boxMin.x - rayOrigin.x) * dirfrac.x;
		float t2 = (boxMax.x - rayOrigin.x) * dirfrac.x;
		float t3 = (boxMin.y - rayOrigin.y) * dirfrac.y;
		float t4 = (boxMax.y - rayOrigin.y) * dirfrac.y;
		float t5 = (boxMin.z - rayOrigin.z) * dirfrac.z;
		float t6 = (boxMax.z - rayOrigin.z) * dirfrac.z;

		float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
		float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

		if (tmax < 0 || tmin > tmax)
			return false;

		if (out != nullptr)
			*out = tmin < 0 ? rayOrigin : rayOrigin + rayDir * tmin;
		return true;
	}

	inline float DistancePointSegment2D(
		const glm::vec2& point,
		const glm::vec2& segA, const glm::vec2& segB)
	{
		float l2 = glm::pow(segA.x - segB.x, 2.0f) + glm::pow(segA.y - segB.y, 2.0f);
		if (l2 == 0.0f)
			return glm::pow(point.x - segA.x, 2.0f) + glm::pow(point.y - segA.y, 2.0f);
		float t = ((point.x - segA.x) * (segB.x - segA.x) + (point.y - segA.y) * (segB.y - segA.y)) / l2;
		t = glm::max(0.0f, glm::min(1.0f, t));
		glm::vec2 o = { segA.x + t * (segB.x - segA.x), segA.y + t * (segB.y - segA.y) };
		return glm::sqrt(glm::pow(point.x - o.x, 2.0f) + glm::pow(point.y - o.y, 2.0f));
	}

	inline bool IntersectPointTriangle2D(
		const glm::vec2& point,
		const glm::vec2& triA, const glm::vec2& triB, const glm::vec2& triC)
	{
		float d1, d2, d3;
		bool has_neg, has_pos;

		d1 = (point.x - triB.x) * (triA.y - triB.y) - (triA.x - triB.x) * (point.y - triB.y);
		d2 = (point.x - triC.x) * (triB.y - triC.y) - (triB.x - triC.x) * (point.y - triC.y);
		d3 = (point.x - triA.x) * (triC.y - triA.y) - (triC.x - triA.x) * (point.y - triA.y);

		has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		return !(has_neg && has_pos);
	}

	inline float DistancePointTriangle2D(
		const glm::vec2& point,
		const glm::vec2& triA, const glm::vec2& triB, const glm::vec2& triC)
	{
		if (IntersectPointTriangle2D(point, triA, triB, triC))
			return 0.0f;

		float disToSideA = DistancePointSegment2D(point, triA, triB);
		float disToSideB = DistancePointSegment2D(point, triB, triC);
		float disToSideC = DistancePointSegment2D(point, triC, triA);

		return glm::min(glm::min(disToSideA, disToSideB), disToSideC);
	}

	// From https://github.com/juj/MathGeoLib/blob/master/src/Geometry/AABB.cpp#L725
	inline bool IntersectLineAABB_CPP(
		const glm::vec3& linePos, const glm::vec3& lineDir,
		const glm::vec3& boxMin, const glm::vec3& boxMax,
		float& tNear, float& tFar)
	{
		// lineDir must be normalized
		assert(tNear <= tFar); // AABB::IntersectLineAABB: User gave a degenerate line as input for the intersection test
		// The user should have inputted values for tNear and tFar to specify the desired subrange [tNear, tFar] of the line
		// for this intersection test.
		// For a Line-AABB test, pass in
		//    tNear = -FLOAT_INF;
		//    tFar = FLOAT_INF;
		// For a Ray-AABB test, pass in
		//    tNear = 0.f;
		//    tFar = FLOAT_INF;
		// For a LineSegment-AABB test, pass in
		//    tNear = 0.f;
		//    tFar = LineSegment.Length();

		// Test each cardinal plane (X, Y and Z) in turn.
		if (glm::abs(lineDir.x) < 0.0001f)
		{
			float recipDir = 1.0f / lineDir.x;
			float t1 = (boxMin.x - linePos.x) * recipDir;
			float t2 = (boxMax.x - linePos.x) * recipDir;

			// tNear tracks distance to intersect (enter) the AABB.
			// tFar tracks the distance to exit the AABB.
			if (t1 < t2)
				tNear = glm::max(t1, tNear), tFar = glm::min(t2, tFar);
			else // Swap t1 and t2.
				tNear = glm::max(t2, tNear), tFar = glm::min(t1, tFar);

			if (tNear > tFar)
				return false; // Box is missed since we "exit" before entering it.
		}
		else if (linePos.x < boxMin.x || linePos.x > boxMax.x)
			return false; // The ray can't possibly enter the box, abort.

		if (glm::abs(lineDir.y) < 0.0001f)
		{
			float recipDir = 1.0f / lineDir.y;
			float t1 = (boxMin.y - linePos.y) * recipDir;
			float t2 = (boxMax.y - linePos.y) * recipDir;

			if (t1 < t2)
				tNear = glm::max(t1, tNear), tFar = glm::min(t2, tFar);
			else // Swap t1 and t2.
				tNear = glm::max(t2, tNear), tFar = glm::min(t1, tFar);

			if (tNear > tFar)
				return false; // Box is missed since we "exit" before entering it.
		}
		else if (linePos.y < boxMin.y || linePos.y > boxMax.y)
			return false; // The ray can't possibly enter the box, abort.

		if (glm::abs(lineDir.z) < 0.0001f) // ray is parallel to plane in question
		{
			float recipDir = 1.0f / lineDir.z;
			float t1 = (boxMin.z - linePos.z) * recipDir;
			float t2 = (boxMax.z - linePos.z) * recipDir;

			if (t1 < t2)
				tNear = glm::max(t1, tNear), tFar = glm::min(t2, tFar);
			else // Swap t1 and t2.
				tNear = glm::max(t2, tNear), tFar = glm::min(t1, tFar);
		}
		else if (linePos.z < boxMin.z || linePos.z > boxMax.z)
			return false; // The ray can't possibly enter the box, abort.

		return tNear <= tFar;
	}

	inline float DistancePointPlane(
		const glm::vec3& planeNormal, const glm::vec3& planePoint,
		const glm::vec3& point)
	{
		return glm::dot(glm::normalize(planeNormal), point - planePoint);
	}

	inline glm::vec3 ClosestPointPointSegment(
		const glm::vec3& point,
		const glm::vec3& segA, const glm::vec3& segB)
	{
		glm::vec3 AB = segB - segA;
		float t = glm::dot(point - segA, AB) / glm::dot(AB, AB);
		return segA + glm::clamp(t, 0.0f, 1.0f) * AB;
	}

	// From Fedor's answer at https://stackoverflow.com/questions/2924795/fastest-way-to-compute-point-to-triangle-distance-in-3d
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

	inline void ClosestPointsSegmentTriangle(
		const glm::vec3& segA, const glm::vec3& segB,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC,
		glm::vec3& outSeg, glm::vec3& outTri)
	{
		glm::vec3 trianglePlaneNormal = glm::cross(triB - triA, triC - triB);
		glm::vec3 edgePerpA = glm::cross(trianglePlaneNormal, triB - triA);
		glm::vec3 edgePerpB = glm::cross(trianglePlaneNormal, triC - triB);
		glm::vec3 edgePerpC = glm::cross(trianglePlaneNormal, triA - triC);
		bool segAFallsOnFace = glm::dot(edgePerpA, segA - triA) >= 0.0f && glm::dot(edgePerpB, segA - triB) >= 0.0f && glm::dot(edgePerpC, segA - triC) >= 0.0f;
		bool segBFallsOnFace = glm::dot(edgePerpA, segB - triA) >= 0.0f && glm::dot(edgePerpB, segB - triB) >= 0.0f && glm::dot(edgePerpC, segB - triC) >= 0.0f;
		glm::vec3 segAOnFace, segBOnFace;
		if (segAFallsOnFace || segBFallsOnFace)
			trianglePlaneNormal = glm::normalize(trianglePlaneNormal);
		if (segAFallsOnFace)
			segAOnFace = segA - trianglePlaneNormal * glm::dot(segA - triA, trianglePlaneNormal);
		if (segBFallsOnFace)
			segBOnFace = segB - trianglePlaneNormal * glm::dot(segB - triA, trianglePlaneNormal);

		glm::vec3 closestInEdges[6];
		ClosestPointsSegmentSegment(segA, segB, triA, triB, closestInEdges[0], closestInEdges[1]);
		ClosestPointsSegmentSegment(segA, segB, triB, triC, closestInEdges[2], closestInEdges[3]);
		ClosestPointsSegmentSegment(segA, segB, triC, triA, closestInEdges[4], closestInEdges[5]);

		float minDist2 = glm::distance2(closestInEdges[0], closestInEdges[1]);
		const glm::vec3* posSeg = &closestInEdges[0];
		const glm::vec3* posTri = &closestInEdges[1];
		for (int i = 2; i < 6; i += 2)
		{
			float dist2 = glm::distance2(closestInEdges[i], closestInEdges[i + 1]);
			if (dist2 < minDist2)
			{
				minDist2 = dist2;
				posSeg = &closestInEdges[i];
				posTri = &closestInEdges[i + 1];
			}
		}

		if (segAFallsOnFace)
		{
			float dist2 = glm::distance2(segAOnFace, segA);
			if (dist2 < minDist2)
			{
				minDist2 = dist2;
				posSeg = &segA;
				posTri = &segAOnFace;
			}
		}

		if (segBFallsOnFace)
		{
			float dist2 = glm::distance2(segBOnFace, segB);
			if (dist2 < minDist2)
			{
				minDist2 = dist2;
				posSeg = &segB;
				posTri = &segBOnFace;
			}
		}
		outSeg = *posSeg;
		outTri = *posTri;
	}

	inline bool IntersectPlaneSegment(
		const glm::vec3& planePoint, const glm::vec3& planeNormal,
		const glm::vec3& segA, const glm::vec3& segB,
		glm::vec3& out)
	{
		glm::vec3 segDisp = segB - segA;
		float ratio = glm::dot(planePoint - segA, planeNormal) / glm::dot(segDisp, planeNormal);
		out = segA + segDisp * ratio;
		return ratio >= 0.0f && ratio <= 1.0f;
	}

	inline bool IntersectTriangleSegment(
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC,
		const glm::vec3& segA, const glm::vec3& segB,
		glm::vec3& out)
	{
		glm::vec3 normal = glm::cross(triB - triA, triC - triB);
		bool planeIntersects = IntersectPlaneSegment(triA, normal, segA, segB, out);
		return planeIntersects &&
			glm::dot(out - triA, glm::cross(normal, triB - triA)) > 0.0f &&
			glm::dot(out - triB, glm::cross(normal, triC - triB)) > 0.0f &&
			glm::dot(out - triC, glm::cross(normal, triA - triC)) > 0.0f;
	}

	inline bool IntersectAABBSegment(
		const glm::vec3& boxMin, const glm::vec3& boxMax,
		const glm::vec3& segA, const glm::vec3& segB)
	{
		float segLength = glm::length(segB - segA);
		float tNear = 0.0f;
		return IntersectLineAABB_CPP(segA, (segB - segA) / segLength, boxMin, boxMax, tNear, segLength);
	}

	inline bool IntersectAABBTriangle(
		const glm::vec3& boxMin, const glm::vec3& boxMax,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC)
	{
		if ((triA.x < boxMax.x && triA.x > boxMin.x &&
				triA.y < boxMax.y && triA.y > boxMin.y &&
				triA.z < boxMax.z && triA.z > boxMin.z) ||
			(triB.x < boxMax.x && triB.x > boxMin.x &&
				triB.y < boxMax.y && triB.y > boxMin.y &&
				triB.z < boxMax.z && triB.z > boxMin.z) ||
			(triC.x < boxMax.x && triC.x > boxMin.x &&
				triC.y < boxMax.y && triC.y > boxMin.y &&
				triC.z < boxMax.z && triC.z > boxMin.z))
			return true;

		glm::vec3 unused;
		if (IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMin.y, boxMin.z), glm::vec3(boxMin.x, boxMin.y, boxMax.z), unused) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMin.y, boxMax.z), glm::vec3(boxMax.x, boxMin.y, boxMax.z), unused) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMin.y, boxMax.z), glm::vec3(boxMax.x, boxMin.y, boxMin.z), unused) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMin.y, boxMin.z), glm::vec3(boxMin.x, boxMin.y, boxMin.z), unused) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMax.y, boxMin.z), glm::vec3(boxMin.x, boxMax.y, boxMax.z), unused) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMax.y, boxMax.z), glm::vec3(boxMax.x, boxMax.y, boxMax.z), unused) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMax.y, boxMax.z), glm::vec3(boxMax.x, boxMax.y, boxMin.z), unused) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMax.y, boxMin.z), glm::vec3(boxMin.x, boxMax.y, boxMin.z), unused) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMin.y, boxMin.z), glm::vec3(boxMin.x, boxMax.y, boxMin.z), unused) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMin.y, boxMax.z), glm::vec3(boxMin.x, boxMax.y, boxMax.z), unused) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMin.y, boxMax.z), glm::vec3(boxMax.x, boxMax.y, boxMax.z), unused) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMin.y, boxMin.z), glm::vec3(boxMax.x, boxMax.y, boxMin.z), unused))
			return true;

		if (IntersectAABBSegment(boxMin, boxMax, triA, triB) ||
			IntersectAABBSegment(boxMin, boxMax, triB, triC) ||
			IntersectAABBSegment(boxMin, boxMax, triC, triA))
			return true;

		return false;
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
		return glm::distance2(localSphereCenter, clampedPoint) < sphere.radius * sphere.radius;
	}

	inline bool IntersectSphereTriangle(
		const SphereCollider& sphere,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC)
	{
		assert(sphere.radius >= 0.0f);
		glm::vec3 closestPointInTriangle = ClosestPointPointTriangle(sphere.center, triA, triB, triC);
		return glm::distance2(sphere.center, closestPointInTriangle) <= sphere.radius * sphere.radius;
	}

	inline bool IntersectCapsuleCapsule(const CapsuleCollider& capsuleA, const CapsuleCollider& capsuleB)
	{
		assert(capsuleA.radius >= 0.0f && capsuleB.radius >= 0.0f);
		glm::vec3 a, b;
		ClosestPointsSegmentSegment(capsuleA.centerA, capsuleA.centerB, capsuleB.centerA, capsuleB.centerB, a, b);
		float radiusSum = capsuleA.radius + capsuleB.radius;
		return glm::distance2(a, b) < radiusSum * radiusSum;
	}

	inline bool IntersectCapsuleBox(const CapsuleCollider& capsule, const BoxCollider& box)
	{
		assert(capsule.radius >= 0.0f && box.size.x >= 0.0f && box.size.y >= 0.0f && box.size.z >= 0.0f);
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

	inline bool IntersectCapsuleTriangle(
		const CapsuleCollider& capsule,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC)
	{
		assert(capsule.radius >= 0.0f);
		glm::vec3 a, b;
		ClosestPointsSegmentTriangle(capsule.centerA, capsule.centerB, triA, triB, triC, a, b);
		return glm::distance2(a, b) < capsule.radius * capsule.radius;
	}

	inline bool IntersectSegmentRectangle2D(
		const glm::vec2& segA, const glm::vec2& segB,
		const glm::vec2& boxMin, const glm::vec2& boxMax)
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
			bool xyIntersects = IntersectSegmentRectangle2D(
				glm::vec2(edges[i][0].x, edges[i][0].y), glm::vec2(edges[i][1].x, edges[i][1].y),
				glm::vec2(boxMin.x, boxMin.y), glm::vec2(boxMax.x, boxMax.y));
			bool xzIntersects = IntersectSegmentRectangle2D(
				glm::vec2(edges[i][0].x, edges[i][0].z), glm::vec2(edges[i][1].x, edges[i][1].z),
				glm::vec2(boxMin.x, boxMin.z), glm::vec2(boxMax.x, boxMax.z));
			bool zyIntersects = IntersectSegmentRectangle2D(
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

	inline bool IntersectBoxTriangle(
		const BoxCollider& box,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC)
	{
		glm::quat undoRotationQuat = glm::conjugate(box.orientation);
		glm::vec3 localTriA = undoRotationQuat * (triA - box.center);
		glm::vec3 localTriB = undoRotationQuat * (triB - box.center);
		glm::vec3 localTriC = undoRotationQuat * (triC - box.center);

		glm::vec3 boxMin = -box.size * 0.5f;
		glm::vec3 boxMax = box.size * 0.5f;
		return IntersectAABBTriangle(boxMin, boxMax, localTriA, localTriB, localTriC);
	}

	inline bool IntersectTriangleTriangle(
		const glm::vec3& tri0A, const glm::vec3& tri0B, const glm::vec3& tri0C,
		const glm::vec3& tri1A, const glm::vec3& tri1B, const glm::vec3& tri1C)
	{
		glm::vec3 unused;
		return
			IntersectTriangleSegment(tri0A, tri0B, tri0C, tri1A, tri1B, unused) ||
			IntersectTriangleSegment(tri0A, tri0B, tri0C, tri1B, tri1C, unused) ||
			IntersectTriangleSegment(tri0A, tri0B, tri0C, tri1C, tri1A, unused) ||
			IntersectTriangleSegment(tri1A, tri1B, tri1C, tri0A, tri0B, unused) ||
			IntersectTriangleSegment(tri1A, tri1B, tri1C, tri0B, tri0C, unused) ||
			IntersectTriangleSegment(tri1A, tri1B, tri1C, tri0C, tri0A, unused);
	}

	// for mesh intersects, the other collider must be in mesh local space
	inline bool IntersectSphereMesh(const SphereCollider& sphere, const MeshCollider& meshCollider)
	{
		SphereCollider optimizationSphereCollider;
		optimizationSphereCollider.center = { 0.0f, 0.0f, 0.0f };
		optimizationSphereCollider.radius = meshCollider.boundingSphereRadius;
		if (!IntersectSphereSphere(optimizationSphereCollider, sphere))
			return false;

		uint32_t j;
		const MeshData* meshData = meshCollider.meshData;
		for (j = 0; j < meshData->indexVector.size(); j += 3)
		{
			const glm::vec3* a = (glm::vec3*)meshData->vertexLayout.Access(meshData->vertexBuffer, MeshData::Position, meshData->indexVector[j + 0]);
			const glm::vec3* b = (glm::vec3*)meshData->vertexLayout.Access(meshData->vertexBuffer, MeshData::Position, meshData->indexVector[j + 1]);
			const glm::vec3* c = (glm::vec3*)meshData->vertexLayout.Access(meshData->vertexBuffer, MeshData::Position, meshData->indexVector[j + 2]);
			if (IntersectSphereTriangle(sphere, *a, *b, *c))
				break;
		}
		return j < meshData->indexVector.size();
	}
	inline bool IntersectCapsuleMesh(const CapsuleCollider& capsule, const MeshCollider& meshCollider)
	{
		SphereCollider optimizationSphereCollider;
		optimizationSphereCollider.center = { 0.0f, 0.0f, 0.0f };
		optimizationSphereCollider.radius = meshCollider.boundingSphereRadius;
		if (!IntersectSphereCapsule(optimizationSphereCollider, capsule))
			return false;

		uint32_t j;
		const MeshData* meshData = meshCollider.meshData;
		for (j = 0; j < meshData->indexVector.size(); j += 3)
		{
			const glm::vec3* a = (glm::vec3*)meshData->vertexLayout.Access(meshData->vertexBuffer, MeshData::Position, meshData->indexVector[j + 0]);
			const glm::vec3* b = (glm::vec3*)meshData->vertexLayout.Access(meshData->vertexBuffer, MeshData::Position, meshData->indexVector[j + 1]);
			const glm::vec3* c = (glm::vec3*)meshData->vertexLayout.Access(meshData->vertexBuffer, MeshData::Position, meshData->indexVector[j + 2]);
			if (IntersectCapsuleTriangle(capsule, *a, *b, *c))
				break;
		}
		return j < meshData->indexVector.size();
	}
	inline bool IntersectBoxMesh(const BoxCollider& box, const MeshCollider& meshCollider)
	{
		SphereCollider optimizationSphereCollider;
		optimizationSphereCollider.center = { 0.0f, 0.0f, 0.0f };
		optimizationSphereCollider.radius = meshCollider.boundingSphereRadius;
		if (!IntersectSphereBox(optimizationSphereCollider, box))
			return false;

		uint32_t j;
		const MeshData* meshData = meshCollider.meshData;
		for (j = 0; j < meshData->indexVector.size(); j += 3)
		{
			const glm::vec3* a = (glm::vec3*)meshData->vertexLayout.Access(meshData->vertexBuffer, MeshData::Position, meshData->indexVector[j + 0]);
			const glm::vec3* b = (glm::vec3*)meshData->vertexLayout.Access(meshData->vertexBuffer, MeshData::Position, meshData->indexVector[j + 1]);
			const glm::vec3* c = (glm::vec3*)meshData->vertexLayout.Access(meshData->vertexBuffer, MeshData::Position, meshData->indexVector[j + 2]);
			if (IntersectBoxTriangle(box, *a, *b, *c))
				break;
		}
		return j < meshData->indexVector.size();
	}

	// Convenience functions
	inline bool IntersectCapsuleSphere(const CapsuleCollider& capsule, const SphereCollider& sphere)
	{
		return IntersectSphereCapsule(sphere, capsule);
	}

	inline bool IntersectBoxSphere(const BoxCollider& box, const SphereCollider& sphere)
	{
		return IntersectSphereBox(sphere, box);
	}

	inline bool IntersectTriangleSphere(const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC, const SphereCollider& sphere)
	{
		return IntersectSphereTriangle(sphere, triA, triB, triC);
	}

	inline bool IntersectBoxCapsule(const BoxCollider& box, const CapsuleCollider& capsule)
	{
		return IntersectCapsuleBox(capsule, box);
	}

	inline bool IntersectTriangleCapsule(const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC, const CapsuleCollider& capsule)
	{
		return IntersectCapsuleTriangle(capsule, triA, triB, triC);
	}

	inline bool IntersectTriangleBox(const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC, const BoxCollider& box)
	{
		return IntersectBoxTriangle(box, triA, triB, triC);
	}

	inline bool IntersectMeshSphere(const MeshCollider& meshCollider, const SphereCollider& sphere)
	{
		return IntersectSphereMesh(sphere, meshCollider);
	}
	inline bool IntersectMeshCapsule(const MeshCollider& meshCollider, const CapsuleCollider& capsule)
	{
		return IntersectCapsuleMesh(capsule, meshCollider);
	}
	inline bool IntersectMeshBox(const MeshCollider& meshCollider, const BoxCollider& box)
	{
		return IntersectBoxMesh(box, meshCollider);
	}
}

#define WORLD_SPACE_SPHERE_COLLIDER(x) (x.GetComponent<SphereCollider>().ApplyTransform(x.GetComponent<Transform>()))
#define WORLD_SPACE_CAPSULE_COLLIDER(x) (x.GetComponent<CapsuleCollider>().ApplyTransform(x.GetComponent<Transform>()))
#define WORLD_SPACE_BOX_COLLIDER(x) (x.GetComponent<BoxCollider>().ApplyTransform(x.GetComponent<Transform>()))

#define INTERSECT_MESH_SPHERE(meshEntity, sphereEntity) Geometry::IntersectSphereMesh( \
	sphereEntity.GetComponent<SphereCollider>().ApplyTransform(sphereEntity.GetComponent<Transform>()).ApplyTransformInverse(meshEntity.GetComponent<Transform>()), \
	meshEntity.GetComponent<MeshCollider>())
#define INTERSECT_SPHERE_MESH(sphereEntity, meshEntity) INTERSECT_MESH_SPHERE(meshEntity, sphereEntity)
#define INTERSECT_MESH_CAPSULE(meshEntity, capsuleEntity) Geometry::IntersectCapsuleMesh( \
	capsuleEntity.GetComponent<CapsuleCollider>().ApplyTransform(capsuleEntity.GetComponent<Transform>()).ApplyTransformInverse(meshEntity.GetComponent<Transform>()), \
	meshEntity.GetComponent<MeshCollider>())
#define INTERSECT_CAPSULE_MESH(capsuleEntity, meshEntity) INTERSECT_MESH_CAPSULE(meshEntity, capsuleEntity)
#define INTERSECT_MESH_BOX(meshEntity, boxEntity) Geometry::IntersectBoxMesh( \
	boxEntity.GetComponent<BoxCollider>().ApplyTransform(boxEntity.GetComponent<Transform>()).ApplyTransformInverse(meshEntity.GetComponent<Transform>()), \
	meshEntity.GetComponent<MeshCollider>())
#define INTERSECT_BOX_MESH(boxEntity, meshEntity) INTERSECT_MESH_BOX(meshEntity, boxEntity)