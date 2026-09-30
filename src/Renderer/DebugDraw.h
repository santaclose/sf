#pragma once
#pragma once

#include <Components/Mesh.h>
#include <Components/SkinnedMesh.h>

#include <Components/SphereCollider.h>
#include <Components/CapsuleCollider.h>
#include <Components/BoxCollider.h>
#include <Components/MeshCollider.h>

namespace sf::Renderer::Debug {

	struct Config
	{
		bool colliders = false;
		bool skeletons = false;
		bool skeletonsModelSpace = false;
		bool frustumCullingSpheres = false;
	};

	void AddLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);

	void AddSphereCollider(const SphereCollider& sphereCollider, const Transform& transform, const glm::vec3* color = nullptr);
	void AddCapsuleCollider(const CapsuleCollider& capsuleCollider, const Transform& transform, const glm::vec3* color = nullptr);
	void AddBoxCollider(const BoxCollider& boxCollider, const Transform& transform, const glm::vec3* color = nullptr);
	void AddMeshCollider(const MeshCollider& meshCollider, const Transform& transform, const glm::vec3* color = nullptr);
	void AddSkeleton(const SkinnedMesh& mesh, const Transform& transform);
	void AddSkeletonModelSpace(const SkinnedMesh& mesh, const Transform& transform);
	void AddFrustumCullingSphere(const SphereCollider& sphereCollider);

	void Draw(uint32_t sharedGpuDataUbo);
	void FrameEnd();
	void Terminate();
}