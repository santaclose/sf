#pragma once

#include "Common.h"

namespace sf::Geometry
{
	struct RayHit
	{
		float distance;
		glm::vec3 point;
		glm::vec3 normal;
	};

	inline bool IntersectRayPlane(
		const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		const glm::vec3& planePoint, const glm::vec3& planeNormal,
		RayHit* out = nullptr)
	{
		constexpr float EPSILON = 1e-6f;

		float denom = glm::dot(planeNormal, rayDir);

		// Plane oriented in the opposite direction
		if (denom > 0.0f)
			return false;

		// Ray is parallel to the plane
		if (glm::abs(denom) < EPSILON)
			return false;

		float t = glm::dot(planePoint - rayOrigin, planeNormal) / denom;

		// Intersection is behind the ray origin
		if (t < 0.0f)
			return false;

		if (out != nullptr)
		{
			out->distance = t;
			out->point = rayOrigin + rayDir * t;
			out->normal = planeNormal;
		}

		return true;
	}

	inline bool IntersectRayTriangle(
		const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC,
		RayHit* out = nullptr)
	{
		glm::vec3 normal = glm::normalize(glm::cross(triB - triA, triC - triA));
		RayHit rh;
		if (!IntersectRayPlane(rayOrigin, rayDir, triA, normal, &rh))
			return false;

		if (!PointLandsInTriangle(rh.point, triA, triB, triC))
			return false;

		if (out != nullptr)
			*out = rh;

		return true;
	}

	inline bool IntersectRaySphere(
		const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		const SphereCollider& sphere,
		RayHit* out = nullptr)
	{
		// Analytic solution
		glm::vec3 L = rayOrigin - sphere.center;

		float a = glm::dot(rayDir, rayDir);
		float b = 2.0f * glm::dot(rayDir, L);
		float c = glm::dot(L, L) - sphere.radius * sphere.radius;

		float t0, t1;
		if (!SolveQuadratic(a, b, c, t0, t1))
			return false;

		// If nearest hit is behind the ray origin,
		// try the farther intersection.
		if (t0 < 0.0f)
		{
			t0 = t1;
			if (t0 < 0.0f)
				return false;
		}

		if (out != nullptr)
		{
			out->distance = t0;
			out->point = rayOrigin + rayDir * t0;
			out->normal = glm::normalize(out->point - sphere.center);
		}

		return true;
	}

	inline bool IntersectRayAABB(
		const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		const glm::vec3& boxMin, const glm::vec3& boxMax,
		RayHit* out = nullptr)
	{
		if (PointInAABB(rayOrigin, boxMin, boxMax))
		{
			if (out != nullptr)
				out->distance = 0.0f;
			return true;
		}

		const glm::vec3* planePoints[6] = { &boxMin, &boxMin, &boxMin, &boxMax, &boxMax, &boxMax };
		glm::vec3 planeNormals[6] = {
			{ -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f, }, { 0.0f, 0.0f, -1.0f },
			{ 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, }, { 0.0f, 0.0f, 1.0f } };

		for (uint32_t i = 0; i < 6; i++)
		{
			RayHit rh;
			if (!IntersectRayPlane(rayOrigin, rayDir, *planePoints[i], planeNormals[i], &rh))
				continue;

			bool insideQuad;
			if (planeNormals[i].x != 0.0f)
			{
				insideQuad =
					boxMin.y < rh.point.y && boxMax.y > rh.point.y &&
					boxMin.z < rh.point.z && boxMax.z > rh.point.z;
			}
			else if (planeNormals[i].y != 0.0f)
			{
				insideQuad =
					boxMin.x < rh.point.x && boxMax.x > rh.point.x &&
					boxMin.z < rh.point.z && boxMax.z > rh.point.z;
			}
			else
			{
				insideQuad =
					boxMin.x < rh.point.x && boxMax.x > rh.point.x &&
					boxMin.y < rh.point.y && boxMax.y > rh.point.y;
			}
			if (insideQuad)
			{
				if (out != nullptr)
					*out = rh;
				return true;
			}
		}
		return false;
	}

	inline bool IntersectRayMesh(
		const glm::vec3& rayOrigin, const glm::vec3& rayDir,
		const MeshCollider& meshCollider,
		RayHit* out = nullptr)
	{
		if (!IntersectRaySphere(rayOrigin, rayDir, meshCollider.boundingSphere))
			return false;

		RayHit rh, prevRh;
		rh.distance = INFINITY;
		const MeshData* meshData = meshCollider.meshData;
		for (uint32_t j = 0; j < meshData->indexCount; j += 3)
		{
			const glm::vec3* a = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 0]);
			const glm::vec3* b = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 1]);
			const glm::vec3* c = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 2]);
			prevRh = rh;
			if (IntersectRayTriangle(rayOrigin, rayDir, *a, *b, *c, &rh))
			{
				rh.distance = prevRh.distance < rh.distance ? prevRh.distance : rh.distance;
				rh.normal = prevRh.distance < rh.distance ? prevRh.normal : rh.normal;
			}

		}
		if (rh.distance != INFINITY && out != nullptr)
			*out = rh;

		return rh.distance != INFINITY;
	}

	inline bool MovingSphereTriangle(
		const SphereCollider& sphere, const glm::vec3& disp,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC,
		RayHit* out = nullptr)
	{
		glm::vec3 normal = glm::cross(triB - triA, triC - triA);
		if (glm::dot(normal, disp) >= 0.0f)
			return false;

		glm::vec3 uNormal = glm::normalize(normal);
		float dispLength = glm::length(disp);
		glm::vec3 dispDir = disp / dispLength;

		glm::vec3 dVec = sphere.center - uNormal * sphere.radius;
		RayHit rh;
		if (!IntersectRayPlane(dVec, dispDir, triA, uNormal, &rh))
			return false;

		bool pointHitsTri = PointLandsInTriangle(rh.point, triA, triB, triC);
		if (pointHitsTri && out != nullptr)
		{
			if (rh.distance > dispLength)
				return false;
			*out = rh;
			return true;
		}

		glm::vec3 qVec = ClosestPointPointTriangle(rh.point, triA, triB, triC);
		if (!IntersectRaySphere(qVec, -dispDir, sphere, &rh))
			return false;
		if (rh.distance < 0.0f || rh.distance > dispLength)
			return false;

		if (out != nullptr)
			*out = { rh.distance, qVec, uNormal };
		return true;
	}

	inline bool MovingSphereMesh(
		const SphereCollider& sphere, const glm::vec3& disp,
		const MeshCollider& meshCollider,
		RayHit* out = nullptr)
	{
		assert(out != nullptr);
		CapsuleCollider capsule;
		capsule.centerA = sphere.center;
		capsule.centerB = sphere.center + disp;
		capsule.radius = sphere.radius;
		if (!IntersectSphereCapsule(meshCollider.boundingSphere, capsule))
			return false;

		RayHit frh, rh;
		frh.distance = INFINITY;
		const MeshData* meshData = meshCollider.meshData;
		for (uint32_t j = 0; j < meshData->indexCount; j += 3)
		{
			const glm::vec3* a = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 0]);
			const glm::vec3* b = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 1]);
			const glm::vec3* c = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 2]);

			if (MovingSphereTriangle(sphere, disp, *a, *b, *c, &rh) && rh.distance < frh.distance)
				frh = rh;
		}

		if (frh.distance != INFINITY)
		{
			*out = frh;
			return true;
		}
		return false;
	}

	inline bool MovingCapsuleTriangle(
		const CapsuleCollider& capsule, const glm::vec3& disp,
		const glm::vec3& triA, const glm::vec3& triB, const glm::vec3& triC,
		RayHit* out = nullptr)
	{
		glm::vec3 normal = glm::cross(triB - triA, triC - triA);
		if (glm::dot(normal, disp) >= 0.0f)
			return false;

		/* Make sure capsule A -> B goes with disp, so we don't have to check the other extruded triangle cap */
		CapsuleCollider orientedCapsule;
		if (glm::dot(disp, capsule.centerB - capsule.centerA) < 0.0f)
		{
			orientedCapsule.centerA = capsule.centerB;
			orientedCapsule.centerB = capsule.centerA;
			orientedCapsule.radius = capsule.radius;
		}
		else
			orientedCapsule = capsule;

		/* Convert problem into sphere against multiple triangles */
		glm::vec3 capsuleDisp = orientedCapsule.centerB - orientedCapsule.centerA;
		glm::vec3 triAmoved = triA + capsuleDisp;
		glm::vec3 triBmoved = triB + capsuleDisp;
		glm::vec3 triCmoved = triC + capsuleDisp;

		SphereCollider sphere;
		sphere.center = orientedCapsule.centerB;
		sphere.radius = orientedCapsule.radius;

		RayHit rh, crh;
		rh.distance = INFINITY;

		bool collided = false, collidedWithOriginalTriangle = false;
		collidedWithOriginalTriangle = MovingSphereTriangle(sphere, disp, triA, triB, triC, &crh);
		if (collidedWithOriginalTriangle)
			rh = crh;

		glm::vec3 generatedTriangleNormal;
		generatedTriangleNormal = glm::cross(triAmoved - triA, triB - triA);
		if (glm::dot(generatedTriangleNormal, disp) < 0.0f)
		{
			collided = MovingSphereTriangle(sphere, disp, triA, triAmoved, triB, &crh);
			if (collided && crh.distance < rh.distance)
				rh = crh;
			collided = MovingSphereTriangle(sphere, disp, triB, triAmoved, triBmoved, &crh);
			if (collided && crh.distance < rh.distance)
				rh = crh;
		}

		generatedTriangleNormal = glm::cross(triBmoved - triB, triC - triB);
		if (glm::dot(generatedTriangleNormal, disp) < 0.0f)
		{
			collided = MovingSphereTriangle(sphere, disp, triB, triBmoved, triC, &crh);
			if (collided && crh.distance < rh.distance)
				rh = crh;
			collided = MovingSphereTriangle(sphere, disp, triC, triBmoved, triCmoved, &crh);
			if (collided && crh.distance < rh.distance)
				rh = crh;
		}

		generatedTriangleNormal = glm::cross(triCmoved - triC, triA - triC);
		if (glm::dot(generatedTriangleNormal, disp) < 0.0f)
		{
			collided = MovingSphereTriangle(sphere, disp, triC, triCmoved, triA, &crh);
			if (collided && crh.distance < rh.distance)
				rh = crh;
			collided = MovingSphereTriangle(sphere, disp, triA, triCmoved, triAmoved, &crh);
			if (collided && crh.distance < rh.distance)
				rh = crh;
		}

		if (rh.distance != INFINITY)
		{
			if (out != nullptr)
			{
				*out = rh;
				out->normal = glm::normalize(normal);
				if (!collidedWithOriginalTriangle)
					out->point -=  capsuleDisp;
			}
			return true;
		}
		return false;
	}

	inline bool MovingCapsuleMesh(
		const CapsuleCollider& capsule, const glm::vec3& disp,
		const MeshCollider& meshCollider,
		RayHit* out = nullptr)
	{
		assert(out != nullptr);
		CapsuleCollider boundingCapsule;
		boundingCapsule.centerA = (capsule.centerA + capsule.centerB) / 2.0f;
		boundingCapsule.centerB = boundingCapsule.centerA + disp;
		boundingCapsule.radius = capsule.radius * 2 + glm::distance(capsule.centerA, capsule.centerB);
		if (!IntersectSphereCapsule(meshCollider.boundingSphere, boundingCapsule))
			return false;

		RayHit frh, rh;
		frh.distance = INFINITY;
		const MeshData* meshData = meshCollider.meshData;
		for (uint32_t j = 0; j < meshData->indexCount; j += 3)
		{
			const glm::vec3* a = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 0]);
			const glm::vec3* b = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 1]);
			const glm::vec3* c = meshData->AccessVertexComponent<glm::vec3>(BufferComponent::Position, meshData->indexBuffer[j + 2]);

			if (MovingCapsuleTriangle(capsule, disp, *a, *b, *c, &rh) && rh.distance < frh.distance)
				frh = rh;
		}

		if (frh.distance != INFINITY)
		{
			*out = frh;

			return true;
		}
		return false;
	}
}