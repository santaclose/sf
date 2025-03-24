
#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>
#include <Components/SphereCollider.h>
#include <Components/CapsuleCollider.h>
#include <Components/BoxCollider.h>
#include <Components/MeshCollider.h>

#include <Game.h>
#include <MeshData.h>
#include <Defaults.h>
#include <Geometry.h>
#include <Random.h>
#include <Renderer/Renderer.h>

#include <Importer/ObjImporter.h>

#include "../Viewer.hpp"

#define COLOR_WHITE { 1.0f, 1.0f, 1.0f }
#define COLOR_RED { 1.0f, 0.0f, 0.0f }
#define COLOR_LIGHT_RED { 1.0f, 0.8f, 0.8f }
#define COLOR_BLUE { 0.0f, 0.0f, 1.0f }
#define COLOR_LIGHT_BLUE { 0.8f, 0.8f, 1.0f }
#define COLOR_GREEN { 0.0f, 1.0f, 0.0f }

#define MAX_TEST_COUNT 360

namespace sf
{
	glm::vec3 materialColorGreen = COLOR_GREEN;
	glm::vec3 materialColorRed = COLOR_RED;

	std::string Game::ConfigFilePath = "examples/pbr/config.json";

	enum class TestCase
	{
		PointLineClosestPoint,
		PointTriangleClosestPoint,
		PointBoxClosestPoint,
		LineLineClosestPoints,
		LineTriangleClosestPoints,
		LineAABBClosestPoints,
		LineTriangleIntersection,
		SphereSphereIntersection,
		SphereCapsuleIntersection,
		SphereBoxIntersection,
		CapsuleCapsuleIntersection,
		CapsuleBoxIntersection,
		BoxBoxIntersection,
		BoxTriangleIntersection,
		TriangleTriangleIntersection,
		MeshSphereIntersection,
		MeshCapsuleIntersection,
		MeshBoxIntersection
	};
	TestCase currentTestCase;

	glm::vec3 aabbMin;
	glm::vec3 aabbMax;

	Scene scene;

	MeshData capsuleMesh;

	glm::vec3 positionA = glm::vec3(-1.0f, 0.0f, 0.0f);
	glm::quat rotationA = glm::vec3(0.0f, 0.0f, 0.0f);
	float scaleA = 1.0f;

	glm::vec3 positionB = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::quat rotationB = glm::vec3(0.0f, 0.0f, 0.0f);
	float scaleB = 1.0f;

	glm::vec3 testA = glm::vec3(0.0f, 1.0f, 1.0f);
	glm::vec3 testB = glm::vec3(0.0f, 1.0f, -1.0f);

	glm::vec3 boxOrientationA = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 boxOrientationB = glm::vec3(0.0f, 0.0f, 0.0f);

	uint32_t testCount;
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> lines;
	std::vector<glm::vec3> triangles;
	std::vector<Entity> spheres;
	std::vector<Entity> capsules;
	std::vector<Entity> boxes;

	Entity monkey;

	uint32_t greenMaterial, redMaterial;

	void GenerateShapesForCurrentCase()
	{
		monkey.SetEnabled(false);
		switch (currentTestCase)
		{
			case TestCase::PointLineClosestPoint:
			{
				glm::quat lineRotation = Random::Rotation();
				float lineLength = Random::Float() + 0.2f;
				lines[0] = lineRotation * glm::vec3(0.0f, +lineLength * 0.5f, 0.0f);
				lines[1] = lineRotation * glm::vec3(0.0f, -lineLength * 0.5f, 0.0f);
				for (int i = 0; i < MAX_TEST_COUNT; i++)
					points[i] = glm::mix(lines[0], lines[1], Random::Float()) + Random::UnitVec3() * 0.3f;
				break;
			}
			case TestCase::PointTriangleClosestPoint:
			{
				triangles[0] = Random::UnitVec3() * 0.5f;
				triangles[1] = Random::UnitVec3() * 0.5f;
				triangles[2] = Random::UnitVec3() * 0.5f;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
					points[i] = Random::UnitVec3();
				break;
			}
			case TestCase::PointBoxClosestPoint:
			{
				boxes[0].GetComponent<BoxCollider>().center = glm::vec3(0.0f, 0.0f, 0.0f);
				boxes[0].GetComponent<BoxCollider>().orientation = Random::Rotation();
				boxes[0].GetComponent<BoxCollider>().size = glm::vec3(
					Random::Float() * 1.5f + 0.5f,
					Random::Float() * 1.5f + 0.5f,
					Random::Float() * 1.5f + 0.5f);

				boxes[0].GetComponent<Transform>().position = glm::vec3(0.0f, 0.0f, 0.0f);
				boxes[0].GetComponent<Transform>().rotation = Random::Rotation();
				boxes[0].GetComponent<Transform>().scale = Random::Float() + 0.5f;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
					points[i] = Random::UnitVec3();
				break;
			}
			case TestCase::LineLineClosestPoints:
			{
				glm::quat lineRotation = Random::Rotation();
				float lineLength = Random::Float() + 2.2f;
				lines[0] = lineRotation * glm::vec3(0.0f, +lineLength * 0.5f, 0.0f);
				lines[1] = lineRotation * glm::vec3(0.0f, -lineLength * 0.5f, 0.0f);
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					float randomValue = Random::Float();
					glm::vec3 randomVec = Random::UnitVec3();
					lines[i * 2 + 2] = glm::mix(lines[0], lines[1], randomValue) + randomVec * 0.3f + Random::UnitVec3() * 0.3f;
					lines[i * 2 + 2 + 1] = glm::mix(lines[0], lines[1], randomValue) + randomVec * 0.3f + Random::UnitVec3() * 0.3f;
				}
				break;
			}
			case TestCase::LineTriangleClosestPoints:
			{
				triangles[0] = Random::UnitVec3() * 0.5f;
				triangles[1] = Random::UnitVec3() * 0.5f;
				triangles[2] = Random::UnitVec3() * 0.5f;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					lines[i * 2] = Random::UnitVec3();
					lines[i * 2 + 1] = lines[i * 2] + Random::UnitVec3() * 0.3f;
				}
				break;
			}
			case TestCase::LineAABBClosestPoints:
			{
				aabbMin = glm::vec3(-0.5f, -0.5f, -0.5f);
				aabbMax = glm::vec3(+0.5f, +0.5f, +0.5f);
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					lines[i * 2] = Random::UnitVec3();
					lines[i * 2 + 1] = lines[i * 2] + Random::UnitVec3() * 0.3f;
				}
				break;
			}
			case TestCase::LineTriangleIntersection:
			{
				glm::vec3 planeNormal = Random::UnitVec3();
				glm::vec3 planeY = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), planeNormal));
				glm::vec3 planeX = glm::normalize(glm::cross(planeY, planeNormal));

				glm::vec2 triA2d = Random::UnitVec2();
				glm::vec2 triB2d = Random::UnitVec2();
				glm::vec2 triC2d = Random::UnitVec2();
				triangles[0] = planeX * triA2d.x + planeY * triA2d.y;
				triangles[1] = planeX * triB2d.x + planeY * triB2d.y;
				triangles[2] = planeX * triC2d.x + planeY * triC2d.y;
				glm::vec3 lineDirection = Random::UnitVec3();
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					glm::vec2 pointInPlane = Random::PointInCircle() * 1.2f;
					lines[i * 2 + 0] = planeX * pointInPlane.x + planeY * pointInPlane.y + lineDirection * (Random::Float() - 0.5f);
					lines[i * 2 + 1] = planeX * pointInPlane.x + planeY * pointInPlane.y + lineDirection * (Random::Float() - 0.5f);
				}
				break;
			}
			case TestCase::SphereSphereIntersection:
			{
				spheres[0].GetComponent<SphereCollider>().radius = 1.0f;
				spheres[0].GetComponent<SphereCollider>().center = { 0.0f, 0.0f, 0.0f };
				spheres[0].GetComponent<Transform>().position = { 0.0f, 0.0f, 0.0f };
				spheres[0].GetComponent<Transform>().scale = 1.0f;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					spheres[i + 1].GetComponent<Transform>().position = Random::UnitVec3();
					spheres[i + 1].GetComponent<Transform>().scale = Random::Float() * 0.2f + 0.05f;
					spheres[i + 1].GetComponent<SphereCollider>().radius = Random::Float() * 0.2f + 0.05f;
					spheres[i + 1].GetComponent<SphereCollider>().center = Random::UnitVec3() * 0.3f;
				}
				break;
			}
			case TestCase::SphereCapsuleIntersection:
			{
				glm::quat lineRotation = Random::Rotation();
				float lineLength = Random::Float() * 2.0f + 1.0f;

				capsules[0].GetComponent<Transform>().position = { 0.0f, 0.0f, 0.0f };
				capsules[0].GetComponent<Transform>().rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
				capsules[0].GetComponent<Transform>().scale = 1.0f;
				glm::vec3 centerA = lineRotation * glm::vec3(0.0f, +lineLength * 0.5f, 0.0f);
				glm::vec3 centerB = lineRotation * glm::vec3(0.0f, -lineLength * 0.5f, 0.0f);
				float radius = Random::Float() * 0.5 + 0.1f;
				capsules[0].GetComponent<CapsuleCollider>().centerA = centerA;
				capsules[0].GetComponent<CapsuleCollider>().centerB = centerB;
				capsules[0].GetComponent<CapsuleCollider>().radius = radius;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					spheres[i].GetComponent<SphereCollider>().radius = Random::Float() * 0.1f + 0.1f;
					spheres[i].GetComponent<SphereCollider>().center = glm::mix(centerA, centerB, Random::Float()) + Random::UnitVec3() * radius + Random::UnitVec3() * 0.5f;
				}
				break;
			}
			case TestCase::SphereBoxIntersection:
			{
				glm::quat transformRot = Random::Rotation();
				glm::quat colliderRot = Random::Rotation();
				glm::quat finalRot = transformRot * colliderRot;
				glm::vec3 finalSize = glm::vec3(
					Random::Float() * 1.5f + 0.5f,
					Random::Float() * 1.5f + 0.5f,
					Random::Float() * 1.5f + 0.5f);
				boxes[0].GetComponent<Transform>().position = { 0.0f, 0.0f, 0.0f };
				boxes[0].GetComponent<Transform>().rotation = transformRot;
				boxes[0].GetComponent<Transform>().scale = 1.0f;
				boxes[0].GetComponent<BoxCollider>().center = { 0.0f, 0.0f, 0.0f };
				boxes[0].GetComponent<BoxCollider>().orientation = colliderRot;
				boxes[0].GetComponent<BoxCollider>().size = finalSize;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					int faceAxis = Random::Int(3);
					int faceWhich = Random::Int(2);
					glm::vec3 localPos = glm::vec3{
						faceAxis == 0 ? (faceWhich == 0 ? -finalSize.x * 0.5f : finalSize.x * 0.5f) : (Random::Float() - 0.5f) * finalSize.x,
						faceAxis == 1 ? (faceWhich == 0 ? -finalSize.y * 0.5f : finalSize.y * 0.5f) : (Random::Float() - 0.5f) * finalSize.y,
						faceAxis == 2 ? (faceWhich == 0 ? -finalSize.z * 0.5f : finalSize.z * 0.5f) : (Random::Float() - 0.5f) * finalSize.z };
					spheres[i].GetComponent<Transform>().position = finalRot * localPos + Random::UnitVec3() * 0.3f;
					spheres[i].GetComponent<Transform>().scale = 1.0f;
					spheres[i].GetComponent<SphereCollider>().center = { 0.0f, 0.0f, 0.0f };
					spheres[i].GetComponent<SphereCollider>().radius = glm::mix(0.01f, 0.15f, Random::Float());
				}
				break;
			}
			case TestCase::CapsuleCapsuleIntersection:
			{
				glm::quat lineRotation = Random::Rotation();
				float lineLength = Random::Float() + 2.2f;
				glm::vec3 a = lineRotation * glm::vec3(0.0f, +lineLength * 0.5f, 0.0f);
				glm::vec3 b = lineRotation * glm::vec3(0.0f, -lineLength * 0.5f, 0.0f);
				float radius = Random::Float() + 0.5f;
				capsules[0].GetComponent<CapsuleCollider>().centerA = a;
				capsules[0].GetComponent<CapsuleCollider>().centerB = b;
				capsules[0].GetComponent<CapsuleCollider>().radius = radius;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					float randomValue = Random::Float();
					glm::vec3 randomVec = Random::UnitVec3() * (radius + 0.3f);
					capsules[i + 1].GetComponent<CapsuleCollider>().radius = Random::Float() * 0.1f + 0.1f;
					capsules[i + 1].GetComponent<CapsuleCollider>().centerA = glm::mix(a, b, randomValue) + randomVec + Random::UnitVec3() * 0.3f;
					capsules[i + 1].GetComponent<CapsuleCollider>().centerB = glm::mix(a, b, randomValue) + randomVec + Random::UnitVec3() * 0.3f;
				}
				break;
			}
			case TestCase::CapsuleBoxIntersection:
			{
				glm::quat transformRot = Random::Rotation();
				glm::quat colliderRot = Random::Rotation();
				glm::quat finalRot = transformRot * colliderRot;
				glm::vec3 finalSize = glm::vec3(
					Random::Float() * 1.5f + 0.5f,
					Random::Float() * 1.5f + 0.5f,
					Random::Float() * 1.5f + 0.5f);
				boxes[0].GetComponent<Transform>().position = { 0.0f, 0.0f, 0.0f };
				boxes[0].GetComponent<Transform>().rotation = transformRot;
				boxes[0].GetComponent<Transform>().scale = 1.0f;
				boxes[0].GetComponent<BoxCollider>().center = { 0.0f, 0.0f, 0.0f };
				boxes[0].GetComponent<BoxCollider>().orientation = colliderRot;
				boxes[0].GetComponent<BoxCollider>().size = finalSize;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					int faceAxis = Random::Int(3);
					int faceWhich = Random::Int(2);
					glm::vec3 localPos = glm::vec3{
						faceAxis == 0 ? (faceWhich == 0 ? -finalSize.x * 0.5f : finalSize.x * 0.5f) : (Random::Float() - 0.5f) * finalSize.x,
						faceAxis == 1 ? (faceWhich == 0 ? -finalSize.y * 0.5f : finalSize.y * 0.5f) : (Random::Float() - 0.5f) * finalSize.y,
						faceAxis == 2 ? (faceWhich == 0 ? -finalSize.z * 0.5f : finalSize.z * 0.5f) : (Random::Float() - 0.5f) * finalSize.z };
					capsules[i].GetComponent<Transform>().position = finalRot * localPos + Random::UnitVec3() * 0.3f;
					capsules[i].GetComponent<Transform>().scale = 1.0f;
					float capsuleLength = Random::Float() * 0.3f;
					glm::quat capsuleRot = Random::Rotation();
					capsules[i].GetComponent<CapsuleCollider>().centerA = capsuleRot * glm::vec3(+capsuleLength / 2.0f, 0.0f, 0.0f);
					capsules[i].GetComponent<CapsuleCollider>().centerB = capsuleRot * glm::vec3(-capsuleLength / 2.0f, 0.0f, 0.0f);
					capsules[i].GetComponent<CapsuleCollider>().radius = glm::mix(0.01f, 0.15f, Random::Float());
				}
				break;
			}
			case TestCase::BoxBoxIntersection:
			{
				glm::quat transformRot = Random::Rotation();
				glm::quat colliderRot = Random::Rotation();
				glm::quat finalRot = transformRot * colliderRot;
				glm::vec3 finalSize = glm::vec3(
					Random::Float() * 1.5f + 0.5f,
					Random::Float() * 1.5f + 0.5f,
					Random::Float() * 1.5f + 0.5f);
				boxes[0].GetComponent<Transform>().position = { 0.0f, 0.0f, 0.0f };
				boxes[0].GetComponent<Transform>().rotation = transformRot;
				boxes[0].GetComponent<Transform>().scale = 1.0f;
				boxes[0].GetComponent<BoxCollider>().center = { 0.0f, 0.0f, 0.0f };
				boxes[0].GetComponent<BoxCollider>().orientation = colliderRot;
				boxes[0].GetComponent<BoxCollider>().size = finalSize;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					int faceAxis = Random::Int(3);
					int faceWhich = Random::Int(2);
					glm::vec3 localPos = glm::vec3{
						faceAxis == 0 ? (faceWhich == 0 ? -finalSize.x * 0.5f : finalSize.x * 0.5f) : (Random::Float() - 0.5f) * finalSize.x,
						faceAxis == 1 ? (faceWhich == 0 ? -finalSize.y * 0.5f : finalSize.y * 0.5f) : (Random::Float() - 0.5f) * finalSize.y,
						faceAxis == 2 ? (faceWhich == 0 ? -finalSize.z * 0.5f : finalSize.z * 0.5f) : (Random::Float() - 0.5f) * finalSize.z };
					boxes[i + 1].GetComponent<Transform>().position = finalRot * localPos + Random::UnitVec3() * 0.3f;
					boxes[i + 1].GetComponent<Transform>().rotation = Random::Rotation();
					boxes[i + 1].GetComponent<Transform>().scale = 1.0f;
					boxes[i + 1].GetComponent<BoxCollider>().center = { 0.0f, 0.0f, 0.0f };
					boxes[i + 1].GetComponent<BoxCollider>().orientation = Random::Rotation();
					boxes[i + 1].GetComponent<BoxCollider>().size = glm::vec3(
						Random::Float() * 0.05f + 0.05f,
						Random::Float() * 0.05f + 0.05f,
						Random::Float() * 0.05f + 0.05f);
				}
				break;
			}
			case TestCase::BoxTriangleIntersection:
			{
				glm::quat transformRot = Random::Rotation();
				glm::quat colliderRot = Random::Rotation();
				glm::quat finalRot = transformRot * colliderRot;
				glm::vec3 finalSize = glm::vec3(
					Random::Float() * 1.5f + 0.5f,
					Random::Float() * 1.5f + 0.5f,
					Random::Float() * 1.5f + 0.5f);
				boxes[0].GetComponent<Transform>().position = { 0.0f, 0.0f, 0.0f };
				boxes[0].GetComponent<Transform>().rotation = transformRot;
				boxes[0].GetComponent<Transform>().scale = 1.0f;
				boxes[0].GetComponent<BoxCollider>().center = { 0.0f, 0.0f, 0.0f };
				boxes[0].GetComponent<BoxCollider>().orientation = colliderRot;
				boxes[0].GetComponent<BoxCollider>().size = finalSize;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					int faceAxis = Random::Int(3);
					int faceWhich = Random::Int(2);
					glm::vec3 localPos = glm::vec3{
						faceAxis == 0 ? (faceWhich == 0 ? -finalSize.x * 0.5f : finalSize.x * 0.5f) : (Random::Float() - 0.5f) * finalSize.x,
						faceAxis == 1 ? (faceWhich == 0 ? -finalSize.y * 0.5f : finalSize.y * 0.5f) : (Random::Float() - 0.5f) * finalSize.y,
						faceAxis == 2 ? (faceWhich == 0 ? -finalSize.z * 0.5f : finalSize.z * 0.5f) : (Random::Float() - 0.5f) * finalSize.z };
					glm::vec3 triangleCenter = finalRot * localPos + Random::UnitVec3() * 0.15f;
					triangles[i * 3 + 0] = triangleCenter + Random::UnitVec3() * 0.15f;
					triangles[i * 3 + 1] = triangleCenter + Random::UnitVec3() * 0.15f;
					triangles[i * 3 + 2] = triangleCenter + Random::UnitVec3() * 0.15f;
				}
				break;
			}
			case TestCase::TriangleTriangleIntersection:
			{
				glm::vec3 planeNormal = Random::UnitVec3();
				glm::vec3 planeY = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), planeNormal));
				glm::vec3 planeX = glm::normalize(glm::cross(planeY, planeNormal));

				glm::vec2 triA2d = Random::UnitVec2();
				glm::vec2 triB2d = Random::UnitVec2();
				glm::vec2 triC2d = Random::UnitVec2();
				triangles[0] = planeX * triA2d.x + planeY * triA2d.y;
				triangles[1] = planeX * triB2d.x + planeY * triB2d.y;
				triangles[2] = planeX * triC2d.x + planeY * triC2d.y;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					glm::vec2 pointInPlane = Random::PointInCircle() * 1.2f;
					triangles[i * 3 + 0 + 3] = planeX * pointInPlane.x + planeY * pointInPlane.y + Random::UnitVec3() * 0.3f;
					triangles[i * 3 + 1 + 3] = planeX * pointInPlane.x + planeY * pointInPlane.y + Random::UnitVec3() * 0.3f;
					triangles[i * 3 + 2 + 3] = planeX * pointInPlane.x + planeY * pointInPlane.y + Random::UnitVec3() * 0.3f;
				}
				break;
			}
			case TestCase::MeshSphereIntersection:
			{
				monkey.SetEnabled(true);
				monkey.GetComponent<Transform>().rotation = Random::Rotation();
				monkey.GetComponent<Transform>().scale = Random::Float() * 0.4f + 0.3f;
				monkey.GetComponent<Transform>().position = Random::UnitVec3() * 0.3f;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					spheres[i].GetComponent<Transform>().position = Random::PointInSphere();
					spheres[i].GetComponent<Transform>().scale = Random::Float() * 0.1f + 0.2f;
					spheres[i].GetComponent<SphereCollider>().center = { 0.0f, 0.0f, 0.0f };
					spheres[i].GetComponent<SphereCollider>().radius = Random::Float() * 0.8f;
				}
				break;
			}
			case TestCase::MeshCapsuleIntersection:
			{
				monkey.SetEnabled(true);
				monkey.GetComponent<Transform>().rotation = Random::Rotation();
				monkey.GetComponent<Transform>().scale = Random::Float() * 0.4f + 0.3f;
				monkey.GetComponent<Transform>().position = Random::UnitVec3() * 0.3f;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					glm::quat rotation = Random::Rotation();
					float length = Random::Float() * 0.5f;
					capsules[i].GetComponent<Transform>().position = Random::PointInSphere();
					capsules[i].GetComponent<Transform>().rotation = Random::Rotation();
					capsules[i].GetComponent<Transform>().scale = Random::Float() * 0.1f + 0.2f;
					capsules[i].GetComponent<CapsuleCollider>().centerA = rotation * glm::vec3{ 0.0f, +length * 0.5f, 0.0f };
					capsules[i].GetComponent<CapsuleCollider>().centerB = rotation * glm::vec3{ 0.0f, -length * 0.5f, 0.0f };
					capsules[i].GetComponent<CapsuleCollider>().radius = Random::Float() * 0.5f;
				}
				break;
			}
			case TestCase::MeshBoxIntersection:
			{
				monkey.SetEnabled(true);
				monkey.GetComponent<Transform>().rotation = Random::Rotation();
				monkey.GetComponent<Transform>().scale = Random::Float() * 0.4f + 0.3f;
				monkey.GetComponent<Transform>().position = Random::UnitVec3() * 0.3f;
				for (int i = 0; i < MAX_TEST_COUNT; i++)
				{
					boxes[i].GetComponent<Transform>().position = Random::PointInSphere();
					boxes[i].GetComponent<Transform>().scale = Random::Float() * 0.1f + 0.2f;
					boxes[i].GetComponent<BoxCollider>().center = { 0.0f, 0.0f, 0.0f };
					boxes[i].GetComponent<BoxCollider>().size = glm::abs(Random::UnitVec3());
					boxes[i].GetComponent<BoxCollider>().orientation = Random::Rotation();
				}
				break;
			}
		}
	}

	void Game::Initialize(int argc, char** argv)
	{
		currentTestCase = TestCase::LineAABBClosestPoints;
		testCount = MAX_TEST_COUNT;

		Material materialTemplate("assets/shaders/default.vert", "assets/shaders/solidColor.frag");
		materialTemplate.uniforms["color"] = { (uint32_t)DataType::vec3f32, &materialColorGreen };
		greenMaterial = Renderer::CreateMaterial(materialTemplate);
		materialTemplate.uniforms["color"] = { (uint32_t)DataType::vec3f32, &materialColorRed };
		redMaterial = Renderer::CreateMaterial(materialTemplate);

		aabbMin = glm::vec3(-1.0f, 0.0f, 0.0f);
		aabbMax = glm::vec3(-0.5f, 1.0f, 1.0f);

		ExampleViewer::Initialize(scene);

		points.resize(MAX_TEST_COUNT + 1);
		lines.resize(MAX_TEST_COUNT * 2 + 2);
		triangles.resize(MAX_TEST_COUNT * 3 + 3);

		spheres.resize(MAX_TEST_COUNT + 1);
		capsules.resize(MAX_TEST_COUNT + 1);
		boxes.resize(MAX_TEST_COUNT + 1);

		for (int i = 0; i < MAX_TEST_COUNT + 1; i++)
		{
			spheres[i] = scene.CreateEntity();
			capsules[i] = scene.CreateEntity();
			boxes[i] = scene.CreateEntity();
			spheres[i].AddComponent<Transform>();
			capsules[i].AddComponent<Transform>();
			boxes[i].AddComponent<Transform>();
			spheres[i].AddComponent<SphereCollider>();
			capsules[i].AddComponent<CapsuleCollider>();
			boxes[i].AddComponent<BoxCollider>();
		}

		monkey = scene.CreateEntity();
		monkey.AddComponent<Transform>();
		monkey.AddComponent<Mesh>(&Defaults::MeshDataMonkey());
		monkey.AddComponent<MeshCollider>(&Defaults::MeshDataMonkey());

		GenerateShapesForCurrentCase();
	}
	void Game::Terminate()
	{
		scene.DestroyEntity(monkey);
		for (int i = 0; i < MAX_TEST_COUNT + 1; i++)
		{
			scene.DestroyEntity(spheres[i]);
			scene.DestroyEntity(capsules[i]);
			scene.DestroyEntity(boxes[i]);
		}
		ExampleViewer::Terminate(scene);
	}
	void Game::OnUpdate(float deltaTime, float time)
	{
		if (Input::KeyDown(Input::KeyCode::Space))
			GenerateShapesForCurrentCase();

		ExampleViewer::UpdateCamera(deltaTime);

		switch (currentTestCase)
		{

		case TestCase::PointLineClosestPoint:
		{
			sf::Renderer::AddLine(lines[0], lines[1], COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				glm::vec3 closestPoint = Geometry::ClosestPointPointSegment(points[i], lines[0], lines[1]);
				sf::Renderer::AddLine(closestPoint, points[i], COLOR_RED);
			}
			return;
		}
		case TestCase::PointTriangleClosestPoint:
		{
			sf::Renderer::AddLine(triangles[0], triangles[1], COLOR_BLUE);
			sf::Renderer::AddLine(triangles[1], triangles[2], COLOR_BLUE);
			sf::Renderer::AddLine(triangles[2], triangles[0], COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				glm::vec3 closestPoint = Geometry::ClosestPointPointTriangle(points[i], triangles[0], triangles[1], triangles[2]);
				sf::Renderer::AddLine(closestPoint, points[i], COLOR_RED);
			}
			return;
		}
		case TestCase::PointBoxClosestPoint:
		{
			BoxCollider worldSpaceBox = boxes[0].GetComponent<BoxCollider>().ApplyTransform(boxes[0].GetComponent<Transform>());
			Renderer::DrawBoxCollider(worldSpaceBox, COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				glm::vec3 closestPoint = Geometry::ClosestPointPointBox(points[i], worldSpaceBox);
				sf::Renderer::AddLine(closestPoint, points[i], COLOR_RED);
			}
			return;
		}
		case TestCase::LineLineClosestPoints:
		{
			sf::Renderer::AddLine(lines[0], lines[1], COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				sf::Renderer::AddLine(lines[i * 2 + 2], lines[i * 2 + 2 + 1], COLOR_LIGHT_BLUE);
				glm::vec3 closestPoint0, closestPoint1;
				Geometry::ClosestPointsSegmentSegment(lines[0], lines[1], lines[i * 2 + 2], lines[i * 2 + 2 + 1], closestPoint0, closestPoint1);
				sf::Renderer::AddLine(closestPoint0, closestPoint1, COLOR_RED);
			}
			return;
		}
		case TestCase::LineTriangleClosestPoints:
		{
			sf::Renderer::AddLine(triangles[0], triangles[1], COLOR_BLUE);
			sf::Renderer::AddLine(triangles[1], triangles[2], COLOR_BLUE);
			sf::Renderer::AddLine(triangles[2], triangles[0], COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				sf::Renderer::AddLine(lines[i * 2], lines[i * 2 + 1], COLOR_LIGHT_BLUE);
				glm::vec3 closestPointSeg, closestPointTri;
				Geometry::ClosestPointsSegmentTriangle(lines[i * 2], lines[i * 2 + 1], triangles[0], triangles[1], triangles[2], closestPointSeg, closestPointTri);
				sf::Renderer::AddLine(closestPointSeg, closestPointTri, COLOR_RED);
			}
			return;
		}
		case TestCase::LineAABBClosestPoints:
		{
			sf::Renderer::AddLine(aabbMin, glm::vec3(aabbMax.x, aabbMin.y, aabbMin.z), COLOR_BLUE);
			sf::Renderer::AddLine(aabbMin, glm::vec3(aabbMin.x, aabbMax.y, aabbMin.z), COLOR_BLUE);
			sf::Renderer::AddLine(aabbMin, glm::vec3(aabbMin.x, aabbMin.y, aabbMax.z), COLOR_BLUE);
			sf::Renderer::AddLine(aabbMax, glm::vec3(aabbMin.x, aabbMax.y, aabbMax.z), COLOR_BLUE);
			sf::Renderer::AddLine(aabbMax, glm::vec3(aabbMax.x, aabbMin.y, aabbMax.z), COLOR_BLUE);
			sf::Renderer::AddLine(aabbMax, glm::vec3(aabbMax.x, aabbMax.y, aabbMin.z), COLOR_BLUE);
			sf::Renderer::AddLine(
				glm::vec3(aabbMax.x, aabbMax.y, aabbMin.z),
				glm::vec3(aabbMin.x, aabbMax.y, aabbMin.z), COLOR_BLUE);
			sf::Renderer::AddLine(
				glm::vec3(aabbMin.x, aabbMax.y, aabbMax.z),
				glm::vec3(aabbMin.x, aabbMin.y, aabbMax.z), COLOR_BLUE);
			sf::Renderer::AddLine(
				glm::vec3(aabbMin.x, aabbMax.y, aabbMax.z),
				glm::vec3(aabbMin.x, aabbMax.y, aabbMin.z), COLOR_BLUE);
			sf::Renderer::AddLine(
				glm::vec3(aabbMin.x, aabbMin.y, aabbMax.z),
				glm::vec3(aabbMax.x, aabbMin.y, aabbMax.z), COLOR_BLUE);
			sf::Renderer::AddLine(
				glm::vec3(aabbMax.x, aabbMin.y, aabbMin.z),
				glm::vec3(aabbMax.x, aabbMax.y, aabbMin.z), COLOR_BLUE);
			sf::Renderer::AddLine(
				glm::vec3(aabbMax.x, aabbMin.y, aabbMin.z),
				glm::vec3(aabbMax.x, aabbMin.y, aabbMax.z), COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				bool segmentIntersectsAABB;
				glm::vec3 closestPointLine, closestPointAABB;
				Geometry::ClosestPointsSegmentAABB(lines[i * 2], lines[i * 2 + 1], aabbMin, aabbMax, closestPointLine, closestPointAABB, segmentIntersectsAABB);
				sf::Renderer::AddLine(lines[i * 2], lines[i * 2 + 1], COLOR_LIGHT_BLUE);
				sf::Renderer::AddLine(closestPointLine, closestPointAABB, COLOR_RED);
			}
			return;
		}
		case TestCase::LineTriangleIntersection:
		{
			sf::Renderer::AddLine(triangles[0], triangles[1], COLOR_BLUE);
			sf::Renderer::AddLine(triangles[1], triangles[2], COLOR_BLUE);
			sf::Renderer::AddLine(triangles[2], triangles[0], COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				glm::vec3 intersectionPoint;
				glm::vec3 lineColor = Geometry::IntersectTriangleSegment(triangles[0], triangles[1], triangles[2], lines[i * 2], lines[i * 2 + 1], intersectionPoint) ? glm::vec3 COLOR_RED : glm::vec3 COLOR_LIGHT_BLUE;
				sf::Renderer::AddLine(lines[i * 2], lines[i * 2 + 1], lineColor);
			}
			break;
		}
		case TestCase::SphereSphereIntersection:
		{
			SphereCollider fixedSc = GET_WORLD_SPACE_SPHERE_COLLIDER(spheres[0]);
			Renderer::DrawSphereCollider(fixedSc, COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				SphereCollider sc = GET_WORLD_SPACE_SPHERE_COLLIDER(spheres[i + 1]);
				Renderer::DrawSphereCollider(sc,
					Geometry::IntersectSphereSphere(fixedSc, sc) ? glm::vec3 COLOR_RED : glm::vec3 COLOR_GREEN);
			}
			return;
		}
		case TestCase::SphereCapsuleIntersection:
		{
			CapsuleCollider cc = GET_WORLD_SPACE_CAPSULE_COLLIDER(capsules[0]);
			Renderer::DrawCapsuleCollider(cc, COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				SphereCollider sc = GET_WORLD_SPACE_SPHERE_COLLIDER(spheres[i]);
				Renderer::DrawSphereCollider(sc,
					Geometry::IntersectCapsuleSphere(cc, sc) ? glm::vec3 COLOR_RED : glm::vec3 COLOR_GREEN);
			}
			return;
		}
		case TestCase::SphereBoxIntersection:
		{
			BoxCollider fixedBc = GET_WORLD_SPACE_BOX_COLLIDER(boxes[0]);
			Renderer::DrawBoxCollider(fixedBc, COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				SphereCollider sc = GET_WORLD_SPACE_SPHERE_COLLIDER(spheres[i]);
				Renderer::DrawSphereCollider(sc,
					Geometry::IntersectBoxSphere(fixedBc, sc) ? glm::vec3 COLOR_RED : glm::vec3 COLOR_GREEN);
			}
			return;
		}
		case TestCase::CapsuleCapsuleIntersection:
		{
			CapsuleCollider fixedCc = GET_WORLD_SPACE_CAPSULE_COLLIDER(capsules[0]);
			Renderer::DrawCapsuleCollider(fixedCc, COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				CapsuleCollider cc = GET_WORLD_SPACE_CAPSULE_COLLIDER(capsules[i + 1]);
				Renderer::DrawCapsuleCollider(cc,
					Geometry::IntersectCapsuleCapsule(fixedCc, cc) ? glm::vec3 COLOR_RED : glm::vec3 COLOR_GREEN);
			}
			return;
		}
		case TestCase::CapsuleBoxIntersection:
		{
			BoxCollider fixedBc = GET_WORLD_SPACE_BOX_COLLIDER(boxes[0]);
			Renderer::DrawBoxCollider(fixedBc, COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				CapsuleCollider bc = GET_WORLD_SPACE_CAPSULE_COLLIDER(capsules[i]);
				Renderer::DrawCapsuleCollider(bc,
					Geometry::IntersectBoxCapsule(fixedBc, bc) ? glm::vec3 COLOR_RED : glm::vec3 COLOR_GREEN);
			}
			return;
		}
		case TestCase::BoxBoxIntersection:
		{
			BoxCollider fixedBc = GET_WORLD_SPACE_BOX_COLLIDER(boxes[0]);
			Renderer::DrawBoxCollider(fixedBc, COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				BoxCollider bc = GET_WORLD_SPACE_BOX_COLLIDER(boxes[i + 1]);
				Renderer::DrawBoxCollider(bc,
					Geometry::IntersectBoxBox(fixedBc, bc) ? glm::vec3 COLOR_RED : glm::vec3 COLOR_GREEN);
			}
			return;
		}
		case TestCase::BoxTriangleIntersection:
		{
			BoxCollider fixedBc = GET_WORLD_SPACE_BOX_COLLIDER(boxes[0]);
			Renderer::DrawBoxCollider(fixedBc, COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				glm::vec3 color = Geometry::IntersectBoxTriangle(fixedBc, triangles[i * 3], triangles[i * 3 + 1], triangles[i * 3 + 2]) ?
					glm::vec3 COLOR_RED : glm::vec3 COLOR_GREEN;
				Renderer::AddLine(triangles[i * 3 + 0], triangles[i * 3 + 1], color);
				Renderer::AddLine(triangles[i * 3 + 1], triangles[i * 3 + 2], color);
				Renderer::AddLine(triangles[i * 3 + 2], triangles[i * 3 + 0], color);
			}
			return;
		}
		case TestCase::TriangleTriangleIntersection:
		{
			sf::Renderer::AddLine(triangles[0], triangles[1], COLOR_BLUE);
			sf::Renderer::AddLine(triangles[1], triangles[2], COLOR_BLUE);
			sf::Renderer::AddLine(triangles[2], triangles[0], COLOR_BLUE);
			for (int i = 0; i < testCount; i++)
			{
				glm::vec3 color = Geometry::IntersectTriangleTriangle(
						triangles[0], triangles[1], triangles[2],
						triangles[i * 3 + 3], triangles[i * 3 + 1 + 3], triangles[i * 3 + 2 + 3]) ?
					glm::vec3 COLOR_RED : glm::vec3 COLOR_GREEN;
				Renderer::AddLine(triangles[i * 3 + 0 + 3], triangles[i * 3 + 1 + 3], color);
				Renderer::AddLine(triangles[i * 3 + 1 + 3], triangles[i * 3 + 2 + 3], color);
				Renderer::AddLine(triangles[i * 3 + 2 + 3], triangles[i * 3 + 0 + 3], color);
			}
			break;
			return;
		}
		case TestCase::MeshSphereIntersection:
		{
			for (int i = 0; i < testCount; i++)
			{
				SphereCollider& sc = spheres[i].GetComponent<SphereCollider>();
				Transform& st = spheres[i].GetComponent<Transform>();
				SphereCollider scRelativeToMesh = sc.ApplyTransform(st).ApplyTransformInverse(monkey.GetComponent<Transform>());
				glm::vec3 sphereColor = Geometry::IntersectSphereMesh(scRelativeToMesh, monkey.GetComponent<MeshCollider>()) ?
					glm::vec3 COLOR_RED : glm::vec3 COLOR_GREEN;
				Renderer::DrawSphereCollider(sc.ApplyTransform(st), sphereColor);
			}
			return;
		}
		case TestCase::MeshCapsuleIntersection:
		{
			for (int i = 0; i < testCount; i++)
			{
				CapsuleCollider& cc = capsules[i].GetComponent<CapsuleCollider>();
				Transform& ct = capsules[i].GetComponent<Transform>();
				CapsuleCollider ccRelativeToMesh = cc.ApplyTransform(ct).ApplyTransformInverse(monkey.GetComponent<Transform>());
				glm::vec3 capsuleColor = Geometry::IntersectCapsuleMesh(ccRelativeToMesh, monkey.GetComponent<MeshCollider>()) ?
					glm::vec3 COLOR_RED : glm::vec3 COLOR_GREEN;
				Renderer::DrawCapsuleCollider(cc.ApplyTransform(ct), capsuleColor);
			}
			break;
		}
		case TestCase::MeshBoxIntersection:
		{
			for (int i = 0; i < testCount; i++)
			{
				BoxCollider& bc = boxes[i].GetComponent<BoxCollider>();
				Transform& bt = boxes[i].GetComponent<Transform>();
				BoxCollider bcRelativeToMesh = bc.ApplyTransform(bt).ApplyTransformInverse(monkey.GetComponent<Transform>());
				glm::vec3 boxColor = Geometry::IntersectBoxMesh(bcRelativeToMesh, monkey.GetComponent<MeshCollider>()) ?
					glm::vec3 COLOR_RED : glm::vec3 COLOR_GREEN;
				Renderer::DrawBoxCollider(bc.ApplyTransform(bt), boxColor);
			}
			break;
		}
		default:
			assert(false);
			return;
		}
	}
	void Game::ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Case"))
			{
				if (ImGui::MenuItem("PointLineClosestPoint", NULL, currentTestCase == TestCase::PointLineClosestPoint)) { currentTestCase = TestCase::PointLineClosestPoint; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("PointTriangleClosestPoint", NULL, currentTestCase == TestCase::PointTriangleClosestPoint)) { currentTestCase = TestCase::PointTriangleClosestPoint; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("PointBoxClosestPoint", NULL, currentTestCase == TestCase::PointBoxClosestPoint)) { currentTestCase = TestCase::PointBoxClosestPoint; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("LineLineClosestPoints", NULL, currentTestCase == TestCase::LineLineClosestPoints)) { currentTestCase = TestCase::LineLineClosestPoints; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("LineTriangleClosestPoints", NULL, currentTestCase == TestCase::LineTriangleClosestPoints)) { currentTestCase = TestCase::LineTriangleClosestPoints; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("LineAABBClosestPoints", NULL, currentTestCase == TestCase::LineAABBClosestPoints)) { currentTestCase = TestCase::LineAABBClosestPoints; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("LineTriangleIntersection", NULL, currentTestCase == TestCase::LineTriangleIntersection)) { currentTestCase = TestCase::LineTriangleIntersection; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("SphereSphereIntersection", NULL, currentTestCase == TestCase::SphereSphereIntersection)) { currentTestCase = TestCase::SphereSphereIntersection; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("SphereCapsuleIntersection", NULL, currentTestCase == TestCase::SphereCapsuleIntersection)) { currentTestCase = TestCase::SphereCapsuleIntersection; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("SphereBoxIntersection", NULL, currentTestCase == TestCase::SphereBoxIntersection)) { currentTestCase = TestCase::SphereBoxIntersection; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("CapsuleCapsuleIntersection", NULL, currentTestCase == TestCase::CapsuleCapsuleIntersection)) { currentTestCase = TestCase::CapsuleCapsuleIntersection; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("CapsuleBoxIntersection", NULL, currentTestCase == TestCase::CapsuleBoxIntersection)) { currentTestCase = TestCase::CapsuleBoxIntersection; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("BoxBoxIntersection", NULL, currentTestCase == TestCase::BoxBoxIntersection)) { currentTestCase = TestCase::BoxBoxIntersection; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("BoxTriangleIntersection", NULL, currentTestCase == TestCase::BoxTriangleIntersection)) { currentTestCase = TestCase::BoxTriangleIntersection; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("TriangleTriangleIntersection", NULL, currentTestCase == TestCase::TriangleTriangleIntersection)) { currentTestCase = TestCase::TriangleTriangleIntersection; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("MeshSphereIntersection", NULL, currentTestCase == TestCase::MeshSphereIntersection)) { currentTestCase = TestCase::MeshSphereIntersection; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("MeshCapsuleIntersection", NULL, currentTestCase == TestCase::MeshCapsuleIntersection)) { currentTestCase = TestCase::MeshCapsuleIntersection; GenerateShapesForCurrentCase(); }
				if (ImGui::MenuItem("MeshBoxIntersection", NULL, currentTestCase == TestCase::MeshBoxIntersection)) { currentTestCase = TestCase::MeshBoxIntersection; GenerateShapesForCurrentCase(); }

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Parameters"))
			{
				ImGui::SliderInt("Test count", (int*)&testCount, 0, MAX_TEST_COUNT);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		ExampleViewer::ImGuiCall();
	}
}