#pragma once

#include <Components/Camera.h>
#include <Components/Transform.h>
#include <MeshData.h>
#include <SkeletonData.h>

namespace sf::Renderer::FrustumCulling {

	bool IsSphereVisible(const SphereCollider& sphere, const glm::vec3& cameraPosition);
	void ComputePlaneNormals(float aspectRatio, const Camera& camera, const Transform& cameraTransform);
}