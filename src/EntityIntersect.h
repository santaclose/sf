#pragma once

#include <Geometry/Geometry.h>

#include <Scene/Entity.h>
#include <Components/Transform.h>

namespace sf::EntityIntersect {

	template <typename T>
	T WorldSpace(Entity entity)
	{
		return entity.GetComponent<T>().ApplyTransform(entity.GetComponent<Transform>());
	}

	bool SphereMesh(Entity sphereEntity, Entity meshEntity, uint32_t outMax = 0, uint32_t* outCount = nullptr, Geometry::ContactData* out = nullptr)
	{
		Transform& meshTransform = meshEntity.GetComponent<Transform>();
		SphereCollider worldSpaceSphereCollider = WorldSpace<SphereCollider>(sphereEntity);
		bool res = Geometry::IntersectSphereMesh(
			worldSpaceSphereCollider.ApplyTransform(meshTransform.Inverse()),
			meshEntity.GetComponent<MeshCollider>(), outMax, outCount, out);
		if (res && outMax != 0)
		{
			for (uint32_t i = 0; i < *outCount; i++)
			{
				out[i].normalA = meshTransform.ApplyToDirection(out[i].normalA);
				out[i].normalB = meshTransform.ApplyToDirection(out[i].normalB);
			}
		}
		return res;
	}

	bool CapsuleMesh(Entity capsuleEntity, Entity meshEntity, uint32_t outMax = 0, uint32_t* outCount = nullptr, Geometry::ContactData* out = nullptr)
	{
		Transform& meshTransform = meshEntity.GetComponent<Transform>();
		CapsuleCollider worldSpaceCapsuleCollider = WorldSpace<CapsuleCollider>(capsuleEntity);
		bool res = Geometry::IntersectCapsuleMesh(
			worldSpaceCapsuleCollider.ApplyTransform(meshTransform.Inverse()),
			meshEntity.GetComponent<MeshCollider>(), outMax, outCount, out);
		if (res && outMax != 0)
		{
			for (uint32_t i = 0; i < *outCount; i++)
			{
				out[i].normalA = meshTransform.ApplyToDirection(out[i].normalA);
				out[i].normalB = meshTransform.ApplyToDirection(out[i].normalB);
			}
		}
		return res;
	}

	bool BoxMesh(Entity boxEntity, Entity meshEntity)
	{
		BoxCollider worldSpaceBoxCollider = WorldSpace<BoxCollider>(boxEntity);
		return Geometry::IntersectBoxMesh(
			worldSpaceBoxCollider.ApplyTransform(meshEntity.GetComponent<Transform>().Inverse()),
			meshEntity.GetComponent<MeshCollider>());
	}

	bool MeshBoundingSpheres(Entity a, Entity b)
	{
		SphereCollider worldSpaceA = a.GetComponent<MeshCollider>().boundingSphere.ApplyTransform(a.GetComponent<Transform>());
		return Geometry::IntersectSphereSphere(worldSpaceA.ApplyTransform(b.GetComponent<Transform>().Inverse()), b.GetComponent<MeshCollider>().boundingSphere);
	}

	bool RayMesh(const glm::vec3& rayOrigin, const glm::vec3& rayDir, Entity meshEntity, Geometry::RayHit* out = nullptr)
	{
		Transform meshTransform = meshEntity.GetComponent<Transform>();
		Transform meshTransformInverse = meshTransform.Inverse();
		glm::vec3 rayOriginInMeshEntitySpace = meshTransformInverse.ApplyToPoint(rayOrigin);
		glm::vec3 rayDirInMeshEntitySpace = meshTransformInverse.ApplyToDirection(rayDir);
		bool res = Geometry::IntersectRayMesh(
			rayOriginInMeshEntitySpace, rayDirInMeshEntitySpace,
			meshEntity.GetComponent<MeshCollider>(), out);
		if (res && out != nullptr)
		{
			out->point = meshTransform.ApplyToPoint(out->point);
			out->normal = meshTransform.ApplyToDirection(out->normal);
			out->distance *= meshTransform.scale; 
		}
		return res;
	}

	bool MovingSphereMesh(Entity sphereEntity, const glm::vec3& disp, Entity meshEntity, Geometry::RayHit* out = nullptr)
	{
		SphereCollider worldSpaceSphereCollider = WorldSpace<SphereCollider>(sphereEntity);
		Transform meshTransform = meshEntity.GetComponent<Transform>();
		Transform meshTransformInverse = meshTransform.Inverse();
		glm::vec3 dispInMeshEntitySpace = meshTransformInverse.ApplyToDirection(disp) * meshTransformInverse.scale;
		bool res = Geometry::MovingSphereMesh(
			worldSpaceSphereCollider.ApplyTransform(meshTransformInverse), dispInMeshEntitySpace,
			meshEntity.GetComponent<MeshCollider>(), out);
		if (res && out != nullptr)
		{
			out->point = meshTransform.ApplyToPoint(out->point);
			out->normal = meshTransform.ApplyToDirection(out->normal);
			out->distance *= meshTransform.scale; 
		}
		return res;
	}

	bool MovingCapsuleMesh(Entity capsuleEntity, const glm::vec3& disp, Entity meshEntity, Geometry::RayHit* out = nullptr)
	{
		CapsuleCollider worldSpaceCapsuleCollider = WorldSpace<CapsuleCollider>(capsuleEntity);
		Transform meshTransform = meshEntity.GetComponent<Transform>();
		Transform meshTransformInverse = meshTransform.Inverse();
		glm::vec3 dispInMeshEntitySpace = meshTransformInverse.ApplyToDirection(disp) * meshTransformInverse.scale;
		bool res = Geometry::MovingCapsuleMesh(
			worldSpaceCapsuleCollider.ApplyTransform(meshTransformInverse), dispInMeshEntitySpace,
			meshEntity.GetComponent<MeshCollider>(), out);
		if (res && out != nullptr)
		{
			out->point = meshTransform.ApplyToPoint(out->point);
			out->normal = meshTransform.ApplyToDirection(out->normal);
			out->distance *= meshTransform.scale;
		}
		return res;
	}

	bool RayTerrain(const glm::vec3& rayOrigin, const glm::vec3& rayDir, Entity terrainEntity, Geometry::RayHit* out = nullptr)
	{
		Transform terrainTransform = terrainEntity.GetComponent<Transform>();
		Transform terrainTransformInverse = terrainTransform.Inverse();
		glm::vec3 rayOriginInTerrainEntitySpace = terrainTransformInverse.ApplyToPoint(rayOrigin);
		glm::vec3 rayDirInTerrainEntitySpace = terrainTransformInverse.ApplyToDirection(rayDir);
		bool res = Geometry::IntersectRayTerrain(
			rayOriginInTerrainEntitySpace, rayDirInTerrainEntitySpace,
			terrainEntity.GetComponent<TerrainCollider>(), out);
		if (res && out != nullptr)
		{
			out->point = terrainTransform.ApplyToPoint(out->point);
			out->normal = terrainTransform.ApplyToDirection(out->normal);
			out->distance *= terrainTransform.scale; 
		}
		return res;
	}
}