#pragma once

#include <Components/Camera.h>
#include <Components/Transform.h>
#include <MeshData.h>
#include <SkeletonData.h>

namespace sf::Renderer::FrustumCulling {

	bool IsSphereVisible(const SphereCollider& sphere, const glm::vec3& cameraPosition);
	const SphereCollider& GetOrCreateMeshBoundingSphere(const MeshData* sourceMeshData);
	const SphereCollider& GetOrCreateSkinnedMeshBoundingSphere(const MeshData* sourceMeshData, const SkeletonData* skeletonData);
	void ComputePlaneNormals(float aspectRatio, const Camera& camera, const Transform& cameraTransform);

	uint32_t InvalidateMeshData(const MeshData* meshData);
	uint32_t InvalidateSkeletonData(const SkeletonData* skeletonData);
}