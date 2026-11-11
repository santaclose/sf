#include "FrustumCulling.h"

#include <Geometry/Geometry.h>
#include <MeshProcessor.h>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace sf::Renderer::FrustumCulling
{
	glm::vec3 leftPlaneNormal;
	glm::vec3 rightPlaneNormal;
	glm::vec3 topPlaneNormal;
	glm::vec3 bottomPlaneNormal;
	glm::vec3 farPlanePos, farPlaneNormal;
}

bool sf::Renderer::FrustumCulling::IsSphereVisible(const SphereCollider& sphere, const glm::vec3& cameraPosition)
{
	glm::vec3 leftPoint = Geometry::ClosestPointPointPlane(sphere.center, leftPlaneNormal, cameraPosition);
	if (glm::dot(leftPlaneNormal, sphere.center - cameraPosition) < 0.0f &&
		glm::distance2(leftPoint, sphere.center) > sphere.radius * sphere.radius)
		return false;
	glm::vec3 rightPoint = Geometry::ClosestPointPointPlane(sphere.center, rightPlaneNormal, cameraPosition);
	if (glm::dot(rightPlaneNormal, sphere.center - cameraPosition) < 0.0f &&
		glm::distance2(rightPoint, sphere.center) > sphere.radius * sphere.radius)
		return false;
	glm::vec3 topPoint = Geometry::ClosestPointPointPlane(sphere.center, topPlaneNormal, cameraPosition);
	if (glm::dot(topPlaneNormal, sphere.center - cameraPosition) < 0.0f &&
		glm::distance2(topPoint, sphere.center) > sphere.radius * sphere.radius)
		return false;
	glm::vec3 bottomPoint = Geometry::ClosestPointPointPlane(sphere.center, bottomPlaneNormal, cameraPosition);
	if (glm::dot(bottomPlaneNormal, sphere.center - cameraPosition) < 0.0f &&
		glm::distance2(bottomPoint, sphere.center) > sphere.radius * sphere.radius)
		return false;
	glm::vec3 farPoint = Geometry::ClosestPointPointPlane(sphere.center, farPlaneNormal, farPlanePos);
	if (glm::dot(farPlaneNormal, sphere.center - farPlanePos) < 0.0f &&
		glm::distance2(farPoint, sphere.center) > sphere.radius * sphere.radius)
		return false;
	return true;
}

void sf::Renderer::FrustumCulling::ComputePlaneNormals(float aspectRatio, const Camera& camera, const Transform& cameraTransform)
{
	glm::vec3 camForward = cameraTransform.Forward();
	farPlanePos = cameraTransform.position + camForward * camera.farClippingPlane;
	farPlaneNormal = -camForward;
	float halfVerticalFOV = camera.fieldOfView * 0.5f;
	float tanHalfVerticalFOV = glm::tan(halfVerticalFOV);

	float cosV = glm::cos(halfVerticalFOV);
	float sinV = glm::sin(halfVerticalFOV); 

	float halfHorizontalFOV = glm::atan(tanHalfVerticalFOV * aspectRatio);
	float cosH = glm::cos(halfHorizontalFOV);
	float sinH = glm::sin(halfHorizontalFOV);

	glm::vec3 topNormalVS = glm::vec3( 0.0f, -cosV, -sinV);
	glm::vec3 bottomNormalVS = glm::vec3( 0.0f,  cosV, -sinV);
	glm::vec3 leftNormalVS = glm::vec3( cosH,  0.0f, -sinH);
	glm::vec3 rightNormalVS = glm::vec3(-cosH,  0.0f, -sinH);

	topPlaneNormal = glm::normalize(cameraTransform.rotation * topNormalVS);
	bottomPlaneNormal = glm::normalize(cameraTransform.rotation * bottomNormalVS);
	leftPlaneNormal = glm::normalize(cameraTransform.rotation * leftNormalVS);
	rightPlaneNormal = glm::normalize(cameraTransform.rotation * rightNormalVS);
}