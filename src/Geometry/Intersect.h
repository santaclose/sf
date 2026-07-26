#pragma once

#include "ClosestPoint.h"

namespace sf::Geometry
{
	struct ContactData
	{
		glm::vec3 normalA;
		glm::vec3 normalB;
		float depth;
	};

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

	inline bool IntersectCircleCircle2D(
		const glm::vec2& centerA, float radiusA,
		const glm::vec2& centerB, float radiusB,
		glm::vec2& outA, glm::vec2& outB)
	{
		glm::vec2 d = centerB - centerA;
		float distSq = glm::dot(d, d);

		if (distSq <= 1e-12f)
			return false;

		float dist = glm::sqrt(distSq);
		if (dist > radiusA + radiusB || dist < glm::abs(radiusA - radiusB))
			return false;

		float a = (radiusA * radiusA - radiusB * radiusB + distSq) / (2.0f * dist);

		float hSq = radiusA * radiusA - a * a;
		if (hSq < 0.0f)
			hSq = 0.0f; // clamp precision error

		float h = glm::sqrt(hSq); // the unavoidable sqrt
		glm::vec2 mid = centerA + (a / dist) * d;
		glm::vec2 perp(-d.y / dist, d.x / dist);
		outA = mid + h * perp;
		outB = mid - h * perp;

		return true;
	}

	inline bool IntersectSphereSphere(
		const SphereCollider& sphereA,
		const SphereCollider& sphereB,
		ContactData* out = nullptr)
	{
		assert(sphereA.radius >= 0.0f && sphereB.radius >= 0.0f);
		float radiusSum = sphereA.radius + sphereB.radius;
		float dis2 = glm::distance2(sphereA.center, sphereB.center);
		if (dis2 < radiusSum * radiusSum)
		{
			if (out != nullptr)
			{
				out->normalA = glm::normalize(sphereB.center - sphereA.center);
				out->normalB = -out->normalA;
				out->depth = radiusSum - glm::sqrt(dis2);
			}
			return true;
		}
		return false;
	}

	inline bool IntersectSphereCapsule(
		const SphereCollider& sphere,
		const CapsuleCollider& capsule,
		ContactData* out = nullptr)
	{
		assert(sphere.radius >= 0.0f && capsule.radius >= 0.0f);
		glm::vec3 pointOnLine = ClosestPointPointSegment(sphere.center, capsule.centerA, capsule.centerB);
		float radiusSum = capsule.radius + sphere.radius;
		float dis2 = glm::distance2(pointOnLine, sphere.center);
		if (dis2 < radiusSum * radiusSum)
		{
			if (out != nullptr)
			{
				out->normalA = glm::normalize(sphere.center - pointOnLine);
				out->normalB = -out->normalA;
				out->depth = radiusSum - glm::sqrt(dis2);
			}
			return true;
		}
		return false;
	}

	inline bool IntersectSphereTriangle(
		const SphereCollider& sphere,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC,
		ContactData* out = nullptr)
	{
		assert(sphere.radius >= 0.0f);
		glm::vec3 closestPointInTriangle = ClosestPointPointTriangle(sphere.center, triA, triB, triC);
		float dis2 = glm::distance2(sphere.center, closestPointInTriangle);
		if (dis2 < sphere.radius * sphere.radius)
		{
			if (out != nullptr)
			{
				out->normalA = glm::normalize(closestPointInTriangle - sphere.center);
				out->normalB = glm::normalize(glm::cross(triB - triA, triC - triB));
				out->depth = sphere.radius - glm::sqrt(dis2);
			}
			return true;
		}
		return false;
	}

	inline bool IntersectCapsuleCapsule(
		const CapsuleCollider& capsuleA,
		const CapsuleCollider& capsuleB,
		ContactData* out = nullptr)
	{
		assert(capsuleA.radius >= 0.0f && capsuleB.radius >= 0.0f);
		glm::vec3 a, b;
		ClosestPointsSegmentSegment(capsuleA.centerA, capsuleA.centerB, capsuleB.centerA, capsuleB.centerB, a, b);
		float radiusSum = capsuleA.radius + capsuleB.radius;
		float dis2 = glm::distance2(a, b);
		if (dis2 < radiusSum * radiusSum)
		{
			if (out != nullptr)
			{
				out->normalA = glm::normalize(b - a);
				out->normalB = -out->normalA;
				out->depth = radiusSum - glm::sqrt(dis2);
			}
			return true;
		}
		return false;
	}

	inline bool IntersectCapsuleTriangle(
		const CapsuleCollider& capsule,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC,
		ContactData* out = nullptr)
	{
		assert(capsule.radius >= 0.0f);
		glm::vec3 a, b;
		ClosestPointsSegmentTriangle(capsule.centerA, capsule.centerB, triA, triB, triC, a, b);
		float dis2 = glm::distance2(a, b);
		float radius2 = capsule.radius * capsule.radius;
		if (dis2 < radius2)
		{
			if (out != nullptr)
			{
				out->normalA = glm::normalize(b - a);
				out->normalB = glm::normalize(glm::cross(triB - triA, triC - triB));
				out->depth = capsule.radius - glm::sqrt(dis2);
			}
			return true;
		}
		return false;
	}

	inline bool IntersectSphereMesh(
		const SphereCollider& sphere,
		const MeshCollider& meshCollider,
		uint32_t outMax = 0, uint32_t* outCount = nullptr, ContactData* out = nullptr)
	{
		if (!IntersectSphereSphere(meshCollider.boundingSphere, sphere))
			return false;

		if (outMax > 0)
			*outCount = 0;
		const MeshData* meshData = meshCollider.meshData;
		for (uint32_t j = 0; j < meshData->indexCount; j += 3)
		{
			const glm::vec3* a = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 0]);
			const glm::vec3* b = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 1]);
			const glm::vec3* c = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 2]);
			if (outMax == 0)
			{
				if (IntersectSphereTriangle(sphere, *a, *b, *c))
					return true;
			}
			else
			{
				if (IntersectSphereTriangle(sphere, *a, *b, *c, &out[*outCount]))
				{
					(*outCount)++;
					if (*outCount == outMax)
						return true;
				}
			}
		}
		if (outMax == 0)
			return false;
		else
			return *outCount > 0;
	}
	inline bool IntersectCapsuleMesh(
		const CapsuleCollider& capsule,
		const MeshCollider& meshCollider,
		uint32_t outMax = 0, uint32_t* outCount = nullptr, ContactData* out = nullptr)
	{
		if (!IntersectSphereCapsule(meshCollider.boundingSphere, capsule))
			return false;

		if (outMax > 0)
			*outCount = 0;
		const MeshData* meshData = meshCollider.meshData;
		for (uint32_t j = 0; j < meshData->indexCount; j += 3)
		{
			const glm::vec3* a = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 0]);
			const glm::vec3* b = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 1]);
			const glm::vec3* c = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 2]);
			if (outMax == 0)
			{
				if (IntersectCapsuleTriangle(capsule, *a, *b, *c))
					return true;
			}
			else
			{
				if (IntersectCapsuleTriangle(capsule, *a, *b, *c, &out[*outCount]))
				{
					(*outCount)++;
					if (*outCount == outMax)
						return true;
				}
			}
		}
		if (outMax == 0)
			return false;
		else
			return *outCount > 0;
	}

	inline bool IntersectPlaneSegment(
		const glm::vec3& planePoint, const glm::vec3& planeNormal,
		const glm::vec3& segA, const glm::vec3& segB,
		glm::vec3* out = nullptr)
	{
		glm::vec3 segDisp = segB - segA;
		float ratio = glm::dot(planePoint - segA, planeNormal) / glm::dot(segDisp, planeNormal);
		if (out != nullptr)
			*out = segA + segDisp * ratio;
		return ratio >= 0.0f && ratio <= 1.0f;
	}

	inline bool IntersectTriangleSegment(
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC,
		const glm::vec3& segA, const glm::vec3& segB,
		glm::vec3* out = nullptr)
	{
		glm::vec3 normal = glm::cross(triB - triA, triC - triA);
		glm::vec3 planePoint;
		if (!IntersectPlaneSegment(triA, normal, segA, segB, &planePoint))
			return false;
		if (!PointLandsInTriangle(planePoint, triA, triB, triC))
			return false;
		if (out != nullptr)
			*out = planePoint;
		return true;
	}

	inline bool IntersectAABBSegment(
		const glm::vec3& boxMin, const glm::vec3& boxMax,
		const glm::vec3& segA, const glm::vec3& segB)
	{
		if (PointInAABB(segA, boxMin, boxMax) || PointInAABB(segB, boxMin, boxMax))
			return true;

		const glm::vec3* planePoints[6] = { &boxMin, &boxMin, &boxMin, &boxMax, &boxMax, &boxMax };
		glm::vec3 planeNormals[6] = {
			{ -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f, }, { 0.0f, 0.0f, -1.0f },
			{ 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, }, { 0.0f, 0.0f, 1.0f } };

		for (uint32_t i = 0; i < 6; i++)
		{
			glm::vec3 planePoint;
			if (!IntersectPlaneSegment(*planePoints[i], planeNormals[i], segA, segB, &planePoint))
				continue;

			bool insideQuad;
			if (planeNormals[i].x != 0.0f)
			{
				insideQuad =
					boxMin.y < planePoint.y && boxMax.y > planePoint.y &&
					boxMin.z < planePoint.z && boxMax.z > planePoint.z;
			}
			else if (planeNormals[i].y != 0.0f)
			{
				insideQuad =
					boxMin.x < planePoint.x && boxMax.x > planePoint.x &&
					boxMin.z < planePoint.z && boxMax.z > planePoint.z;
			}
			else
			{
				insideQuad =
					boxMin.x < planePoint.x && boxMax.x > planePoint.x &&
					boxMin.y < planePoint.y && boxMax.y > planePoint.y;
			}
			if (insideQuad)
				return true;
		}
		return false;
	}

	inline bool IntersectAABBTriangle(
		const glm::vec3& boxMin, const glm::vec3& boxMax,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC)
	{
		if (PointInAABB(triA, boxMin, boxMax) ||
			PointInAABB(triB, boxMin, boxMax) ||
			PointInAABB(triC, boxMin, boxMax))
			return true;

		if (IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMin.y, boxMin.z), glm::vec3(boxMin.x, boxMin.y, boxMax.z)) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMin.y, boxMax.z), glm::vec3(boxMax.x, boxMin.y, boxMax.z)) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMin.y, boxMax.z), glm::vec3(boxMax.x, boxMin.y, boxMin.z)) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMin.y, boxMin.z), glm::vec3(boxMin.x, boxMin.y, boxMin.z)) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMax.y, boxMin.z), glm::vec3(boxMin.x, boxMax.y, boxMax.z)) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMax.y, boxMax.z), glm::vec3(boxMax.x, boxMax.y, boxMax.z)) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMax.y, boxMax.z), glm::vec3(boxMax.x, boxMax.y, boxMin.z)) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMax.y, boxMin.z), glm::vec3(boxMin.x, boxMax.y, boxMin.z)) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMin.y, boxMin.z), glm::vec3(boxMin.x, boxMax.y, boxMin.z)) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMin.x, boxMin.y, boxMax.z), glm::vec3(boxMin.x, boxMax.y, boxMax.z)) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMin.y, boxMax.z), glm::vec3(boxMax.x, boxMax.y, boxMax.z)) ||
			IntersectTriangleSegment(triA, triB, triC, glm::vec3(boxMax.x, boxMin.y, boxMin.z), glm::vec3(boxMax.x, boxMax.y, boxMin.z)))
			return true;

		if (IntersectAABBSegment(boxMin, boxMax, triA, triB) ||
			IntersectAABBSegment(boxMin, boxMax, triB, triC) ||
			IntersectAABBSegment(boxMin, boxMax, triC, triA))
			return true;

		return false;
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

	inline bool IntersectTriangleTriangle(
		const glm::vec3& tri0A, const glm::vec3& tri0B, const glm::vec3& tri0C,
		const glm::vec3& tri1A, const glm::vec3& tri1B, const glm::vec3& tri1C)
	{
		return
			IntersectTriangleSegment(tri0A, tri0B, tri0C, tri1A, tri1B) ||
			IntersectTriangleSegment(tri0A, tri0B, tri0C, tri1B, tri1C) ||
			IntersectTriangleSegment(tri0A, tri0B, tri0C, tri1C, tri1A) ||
			IntersectTriangleSegment(tri1A, tri1B, tri1C, tri0A, tri0B) ||
			IntersectTriangleSegment(tri1A, tri1B, tri1C, tri0B, tri0C) ||
			IntersectTriangleSegment(tri1A, tri1B, tri1C, tri0C, tri0A);
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
	inline bool IntersectSphereBox(const SphereCollider& sphere, const BoxCollider& box)
	{
		assert(sphere.radius >= 0.0f && box.size.x >= 0.0f && box.size.y >= 0.0f && box.size.z >= 0.0f);
		glm::quat undoRotationQuat = glm::conjugate(box.orientation);
		glm::vec3 localSphereCenter = undoRotationQuat * (sphere.center - box.center);
		glm::vec3 closestPoint = glm::clamp(localSphereCenter, -box.size * 0.5f, box.size * 0.5f);
		float dis2 = glm::distance2(localSphereCenter, closestPoint);
		return dis2 < sphere.radius * sphere.radius;
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
	inline bool IntersectBoxMesh(const BoxCollider& box, const MeshCollider& meshCollider)
	{
		if (!IntersectSphereBox(meshCollider.boundingSphere, box))
			return false;

		uint32_t j;
		const MeshData* meshData = meshCollider.meshData;
		for (j = 0; j < meshData->indexCount; j += 3)
		{
			const glm::vec3* a = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 0]);
			const glm::vec3* b = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 1]);
			const glm::vec3* c = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 2]);
			if (IntersectBoxTriangle(box, *a, *b, *c))
				break;
		}
		return j < meshData->indexCount;
	}
}