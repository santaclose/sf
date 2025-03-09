
#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>
#include <Components/SphereCollider.h>
#include <Components/CapsuleCollider.h>
#include <Components/BoxCollider.h>

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
#define COLOR_BLUE { 0.0f, 0.0f, 1.0f }
#define COLOR_GREEN { 0.0f, 1.0f, 0.0f }

namespace sf
{
	std::string Game::ConfigFilePath = "examples/pbr/config.json";

	enum class TestCase
	{
		PointLineClosestPoint,
		PointTriangleClosestPoint,
		LineLineClosestPoints,
		LineAABBClosestPoints,
		SphereSphereIntersection,
		SphereCapsuleIntersection,
		SphereBoxIntersection,
		CapsuleCapsuleIntersection,
		CapsuleBoxIntersection,
		BoxBoxIntersection
	};
	TestCase currentTestCase;

	glm::vec3 point;
	glm::vec3 line0A;
	glm::vec3 line0B;
	glm::vec3 line1A;
	glm::vec3 line1B;
	glm::vec3 triangle0A;
	glm::vec3 triangle0B;
	glm::vec3 triangle0C;
	glm::vec3 aabbMin;
	glm::vec3 aabbMax;

	Scene scene;
	Entity e_Sphere[2];
	Entity e_Capsule[2];
	Entity e_Box[2];

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

	void UpdateEnabledShapes()
	{
		switch (currentTestCase)
		{
			case TestCase::PointLineClosestPoint:
			case TestCase::PointTriangleClosestPoint:
			case TestCase::LineLineClosestPoints:
			case TestCase::LineAABBClosestPoints:
				e_Sphere[0].SetEnabled(false);
				e_Sphere[1].SetEnabled(false);
				e_Capsule[0].SetEnabled(false);
				e_Capsule[1].SetEnabled(false);
				e_Box[0].SetEnabled(false);
				e_Box[1].SetEnabled(false);
				break;
			case TestCase::SphereSphereIntersection:
				e_Sphere[0].SetEnabled(true);
				e_Sphere[1].SetEnabled(true);
				e_Capsule[0].SetEnabled(false);
				e_Capsule[1].SetEnabled(false);
				e_Box[0].SetEnabled(false);
				e_Box[1].SetEnabled(false);
				break;
			case TestCase::SphereCapsuleIntersection:
				e_Sphere[0].SetEnabled(true);
				e_Sphere[1].SetEnabled(false);
				e_Capsule[0].SetEnabled(true);
				e_Capsule[1].SetEnabled(false);
				e_Box[0].SetEnabled(false);
				e_Box[1].SetEnabled(false);
				break;
			case TestCase::SphereBoxIntersection:
				e_Sphere[0].SetEnabled(true);
				e_Sphere[1].SetEnabled(false);
				e_Capsule[0].SetEnabled(false);
				e_Capsule[1].SetEnabled(false);
				e_Box[0].SetEnabled(true);
				e_Box[1].SetEnabled(false);
				break;
			case TestCase::CapsuleCapsuleIntersection:
				e_Sphere[0].SetEnabled(false);
				e_Sphere[1].SetEnabled(false);
				e_Capsule[0].SetEnabled(true);
				e_Capsule[1].SetEnabled(true);
				e_Box[0].SetEnabled(false);
				e_Box[1].SetEnabled(false);
				break;
			case TestCase::CapsuleBoxIntersection:
				e_Sphere[0].SetEnabled(false);
				e_Sphere[1].SetEnabled(false);
				e_Capsule[0].SetEnabled(true);
				e_Capsule[1].SetEnabled(false);
				e_Box[0].SetEnabled(true);
				e_Box[1].SetEnabled(false);
				break;
			case TestCase::BoxBoxIntersection:
				e_Sphere[0].SetEnabled(false);
				e_Sphere[1].SetEnabled(false);
				e_Capsule[0].SetEnabled(false);
				e_Capsule[1].SetEnabled(false);
				e_Box[0].SetEnabled(true);
				e_Box[1].SetEnabled(true);
				break;
		}
	}

	void Game::Initialize(int argc, char** argv)
	{
		currentTestCase = TestCase::SphereSphereIntersection;

		point = glm::vec3(-1.0f, 0.0f, 0.0f);
		line0A = glm::vec3(1.0f, -1.0f, 0.0f);
		line0B = glm::vec3(1.0f, 1.0f, 0.0f);
		line1A = glm::vec3(-1.0f, 1.0f, 0.0f);
		line1B = glm::vec3(-1.0f, -1.0f, 0.0f);
		triangle0A = glm::vec3(1.0f, -1.0f, 0.0f);
		triangle0B = glm::vec3(1.0f, 1.0f, 0.0f);
		triangle0C = glm::vec3(0.0f, 0.0f, 0.0f);
		aabbMin = glm::vec3(-1.0f, 0.0f, 0.0f);
		aabbMax = glm::vec3(-0.5f, 1.0f, 1.0f);

		Renderer::SetDebugDrawEnabled(true);

		ExampleViewer::Initialize(scene);

		for (int i = 0; i < 2; i++)
		{
			e_Sphere[i] = scene.CreateEntity();
			e_Sphere[i].AddComponent<Transform>();
			e_Sphere[i].AddComponent<SphereCollider>().radius = 0.5f;
		}
		for (int i = 0; i < 2; i++)
		{
			e_Capsule[i] = scene.CreateEntity();
			e_Capsule[i].AddComponent<Transform>();
			CapsuleCollider& cc_Capsule = e_Capsule[i].AddComponent<CapsuleCollider>();
		}
		for (int i = 0; i < 2; i++)
		{
			e_Box[i] = scene.CreateEntity();
			e_Box[i].AddComponent<Transform>();
			BoxCollider& bc_Box = e_Box[i].AddComponent<BoxCollider>();
		}

		UpdateEnabledShapes();
	}
	void Game::Terminate()
	{
		ExampleViewer::Terminate(scene);
		for (int i = 0; i < 2; i++)
			scene.DestroyEntity(e_Sphere[i]);
		for (int i = 0; i < 2; i++)
			scene.DestroyEntity(e_Capsule[i]);
		for (int i = 0; i < 2; i++)
			scene.DestroyEntity(e_Box[i]);
	}
	void Game::OnUpdate(float deltaTime, float time)
	{
		ExampleViewer::UpdateCamera(deltaTime);
		Transform* transformA;
		Transform* transformB;

		switch (currentTestCase)
		{

		case TestCase::PointLineClosestPoint:
		{
			glm::vec3 closestPoint = Geometry::ClosestPointPointSegment(point, line0A, line0B);
			sf::Renderer::AddLine(line0A, line0B, COLOR_BLUE);
			sf::Renderer::AddLine(closestPoint, point, COLOR_RED);
			return;
		}
		case TestCase::PointTriangleClosestPoint:
		{
			glm::vec3 closestPoint = Geometry::ClosestPointPointTriangle(point, triangle0A, triangle0B, triangle0C);
			sf::Renderer::AddLine(triangle0A, triangle0B, COLOR_BLUE);
			sf::Renderer::AddLine(triangle0B, triangle0C, COLOR_BLUE);
			sf::Renderer::AddLine(triangle0C, triangle0A, COLOR_BLUE);
			sf::Renderer::AddLine(closestPoint, point, COLOR_RED);
			return;
		}
		case TestCase::LineLineClosestPoints:
		{
			glm::vec3 closestPoint0, closestPoint1;
			Geometry::ClosestPointsSegmentSegment(line0A, line0B, line1A, line1B, closestPoint0, closestPoint1);
			sf::Renderer::AddLine(line0A, line0B, COLOR_BLUE);
			sf::Renderer::AddLine(line1A, line1B, COLOR_BLUE);
			sf::Renderer::AddLine(closestPoint0, closestPoint1, COLOR_RED);
			return;
		}
		case TestCase::LineAABBClosestPoints:
		{
			bool segmentIntersectsAABB;
			glm::vec3 closestPointLine, closestPointAABB, test;
			Geometry::ClosestPointsSegmentAABB(line0A, line0B, aabbMin, aabbMax, closestPointLine, closestPointAABB, segmentIntersectsAABB);
			sf::Renderer::AddLine(closestPointLine, closestPointAABB, COLOR_RED);
			sf::Renderer::AddLine(line0A, line0B, COLOR_BLUE);
			if (segmentIntersectsAABB)
				Renderer::SetClearColor(COLOR_RED);
			else
				Renderer::SetClearColor(COLOR_WHITE);
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
			return;
		}
		case TestCase::SphereSphereIntersection:
			transformA = &e_Sphere[0].GetComponent<Transform>();
			transformB = &e_Sphere[1].GetComponent<Transform>();
			break;
		case TestCase::SphereCapsuleIntersection:
			transformA = &e_Sphere[0].GetComponent<Transform>();
			transformB = &e_Capsule[0].GetComponent<Transform>();
			break;
		case TestCase::SphereBoxIntersection:
			transformA = &e_Sphere[0].GetComponent<Transform>();
			transformB = &e_Box[0].GetComponent<Transform>();
			break;
		case TestCase::CapsuleCapsuleIntersection:
			transformA = &e_Capsule[0].GetComponent<Transform>();
			transformB = &e_Capsule[1].GetComponent<Transform>();
			break;
		case TestCase::CapsuleBoxIntersection:
			transformA = &e_Capsule[0].GetComponent<Transform>();
			transformB = &e_Box[0].GetComponent<Transform>();
			break;
		case TestCase::BoxBoxIntersection:
			transformA = &e_Box[0].GetComponent<Transform>();
			transformB = &e_Box[1].GetComponent<Transform>();
			break;
		default:
			assert(false);
			return;
		}

		if (transformA)
		{
			transformA->position = positionA;
			transformA->rotation = glm::normalize(glm::quat(rotationA));
			transformA->scale = scaleA;
		}
		if (transformB)
		{
			transformB->position = positionB;
			transformB->rotation = glm::normalize(glm::quat(rotationB));
			transformB->scale = scaleB;
		}

		bool intersects;
		switch (currentTestCase)
		{
		case TestCase::SphereSphereIntersection:
			intersects = Geometry::IntersectSphereSphere(
				e_Sphere[0].GetComponent<SphereCollider>().ApplyTransform(e_Sphere[0].GetComponent<Transform>()),
				e_Sphere[1].GetComponent<SphereCollider>().ApplyTransform(e_Sphere[1].GetComponent<Transform>()));
			break;
		case TestCase::SphereCapsuleIntersection:
			intersects = Geometry::IntersectSphereCapsule(
				e_Sphere[0].GetComponent<SphereCollider>().ApplyTransform(e_Sphere[0].GetComponent<Transform>()),
				e_Capsule[0].GetComponent<CapsuleCollider>().ApplyTransform(e_Capsule[0].GetComponent<Transform>()));
			break;
		case TestCase::SphereBoxIntersection:
			intersects = Geometry::IntersectSphereBox(
				e_Sphere[0].GetComponent<SphereCollider>().ApplyTransform(e_Sphere[0].GetComponent<Transform>()),
				e_Box[0].GetComponent<BoxCollider>().ApplyTransform(e_Box[0].GetComponent<Transform>()));
			break;
		case TestCase::CapsuleCapsuleIntersection:
			intersects = Geometry::IntersectCapsuleCapsule(
				e_Capsule[0].GetComponent<CapsuleCollider>().ApplyTransform(e_Capsule[0].GetComponent<Transform>()),
				e_Capsule[1].GetComponent<CapsuleCollider>().ApplyTransform(e_Capsule[1].GetComponent<Transform>()));
			break;
		case TestCase::CapsuleBoxIntersection:
			intersects = Geometry::IntersectCapsuleBox(
				e_Capsule[0].GetComponent<CapsuleCollider>().ApplyTransform(e_Capsule[0].GetComponent<Transform>()),
				e_Box[0].GetComponent<BoxCollider>().ApplyTransform(e_Box[0].GetComponent<Transform>()));
			break;
		case TestCase::BoxBoxIntersection:
			intersects = Geometry::IntersectBoxBox(
				e_Box[0].GetComponent<BoxCollider>().ApplyTransform(e_Box[0].GetComponent<Transform>()),
				e_Box[1].GetComponent<BoxCollider>().ApplyTransform(e_Box[1].GetComponent<Transform>()));
			break;

		}
		if (intersects)
			Renderer::SetClearColor(COLOR_RED);
		else
			Renderer::SetClearColor(COLOR_WHITE);
	}
	void Game::ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Case"))
			{
				if (ImGui::MenuItem("PointLineClosestPoint")) { currentTestCase = TestCase::PointLineClosestPoint; UpdateEnabledShapes(); }
				if (ImGui::MenuItem("PointTriangleClosestPoint")) { currentTestCase = TestCase::PointTriangleClosestPoint; UpdateEnabledShapes(); }
				if (ImGui::MenuItem("LineLineClosestPoints")) { currentTestCase = TestCase::LineLineClosestPoints; UpdateEnabledShapes(); }
				if (ImGui::MenuItem("LineAABBClosestPoints")) { currentTestCase = TestCase::LineAABBClosestPoints; UpdateEnabledShapes(); }
				if (ImGui::MenuItem("SphereSphereIntersection")) { currentTestCase = TestCase::SphereSphereIntersection; UpdateEnabledShapes(); }
				if (ImGui::MenuItem("SphereCapsuleIntersection")) { currentTestCase = TestCase::SphereCapsuleIntersection; UpdateEnabledShapes(); }
				if (ImGui::MenuItem("SphereBoxIntersection")) { currentTestCase = TestCase::SphereBoxIntersection; UpdateEnabledShapes(); }
				if (ImGui::MenuItem("CapsuleCapsuleIntersection")) { currentTestCase = TestCase::CapsuleCapsuleIntersection; UpdateEnabledShapes(); }
				if (ImGui::MenuItem("CapsuleBoxIntersection")) { currentTestCase = TestCase::CapsuleBoxIntersection; UpdateEnabledShapes(); }
				if (ImGui::MenuItem("BoxBoxIntersection")) { currentTestCase = TestCase::BoxBoxIntersection; UpdateEnabledShapes(); }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Parameters"))
			{
				switch (currentTestCase)
				{
				case TestCase::PointLineClosestPoint:
					ImGui::DragFloat3("Point", &point.x, 0.01f);
					ImGui::DragFloat3("Line A", &line0A.x, 0.01f);
					ImGui::DragFloat3("Line B", &line0B.x, 0.01f);
					break;
				case TestCase::PointTriangleClosestPoint:
					ImGui::DragFloat3("Point", &point.x, 0.01f);
					ImGui::DragFloat3("Triangle A", &triangle0A.x, 0.01f);
					ImGui::DragFloat3("Triangle B", &triangle0B.x, 0.01f);
					ImGui::DragFloat3("Triangle C", &triangle0C.x, 0.01f);
					break;
				case TestCase::LineLineClosestPoints:
					ImGui::DragFloat3("Line 0 A", &line0A.x, 0.01f);
					ImGui::DragFloat3("Line 0 B", &line0B.x, 0.01f);
					ImGui::DragFloat3("Line 1 A", &line1A.x, 0.01f);
					ImGui::DragFloat3("Line 1 B", &line1B.x, 0.01f);
					break;
				case TestCase::LineAABBClosestPoints:
					ImGui::DragFloat3("Line 0 A", &line0A.x, 0.01f);
					ImGui::DragFloat3("Line 0 B", &line0B.x, 0.01f);
					ImGui::DragFloat3("AABB Min", &aabbMin.x, 0.01f);
					ImGui::DragFloat3("AABB Max", &aabbMax.x, 0.01f);
					break;
				default:
					ImGui::DragFloat3("Position A", &positionA.x, 0.01f);
					ImGui::DragFloat3("Rotation A", &rotationA.x, 0.01f);
					ImGui::DragFloat("Scale A", &scaleA, 0.01f);
					ImGui::DragFloat3("Position B", &positionB.x, 0.01f);
					ImGui::DragFloat3("Rotation B", &rotationB.x, 0.01f);
					ImGui::DragFloat("Scale B", &scaleB, 0.01f);
					switch (currentTestCase)
					{
					case TestCase::SphereSphereIntersection:
					{
						SphereCollider& sca = e_Sphere[0].GetComponent<SphereCollider>();
						SphereCollider& scb = e_Sphere[1].GetComponent<SphereCollider>();
						ImGui::DragFloat3("Sphere A center", &sca.center.x, 0.01f);
						ImGui::DragFloat("Sphere A radius", &sca.radius, 0.01f);
						ImGui::DragFloat3("Sphere B center", &scb.center.x, 0.01f);
						ImGui::DragFloat("Sphere B radius", &scb.radius, 0.01f);
						break;
					}
					case TestCase::SphereCapsuleIntersection:
					{
						SphereCollider& sc = e_Sphere[0].GetComponent<SphereCollider>();
						CapsuleCollider& cc = e_Capsule[0].GetComponent<CapsuleCollider>();
						ImGui::DragFloat3("Sphere center", &sc.center.x, 0.01f);
						ImGui::DragFloat("Sphere radius", &sc.radius, 0.01f);
						ImGui::DragFloat3("Capsule center A", &cc.centerA.x, 0.01f);
						ImGui::DragFloat3("Capsule center B", &cc.centerB.x, 0.01f);
						ImGui::DragFloat("Capsule radius", &cc.radius, 0.01f);
						break;
					}
					case TestCase::SphereBoxIntersection:
					{
						//static glm::vec3 boxOrientation;
						SphereCollider& sc = e_Sphere[0].GetComponent<SphereCollider>();
						BoxCollider& bc = e_Box[0].GetComponent<BoxCollider>();
						ImGui::DragFloat3("Sphere center", &sc.center.x, 0.01f);
						ImGui::DragFloat("Sphere radius", &sc.radius, 0.01f);
						ImGui::DragFloat3("Box center", &bc.center.x, 0.01f);
						ImGui::DragFloat3("Box size", &bc.size.x, 0.01f);
						ImGui::DragFloat3("Box orientation", &boxOrientationA.x, 0.01f);
						bc.orientation = glm::quat(boxOrientationA);
						break;
					}
					case TestCase::CapsuleCapsuleIntersection:
					{
						CapsuleCollider& cca = e_Capsule[0].GetComponent<CapsuleCollider>();
						CapsuleCollider& ccb = e_Capsule[1].GetComponent<CapsuleCollider>();
						ImGui::DragFloat3("Capsule A center A", &cca.centerA.x, 0.01f);
						ImGui::DragFloat3("Capsule A center B", &cca.centerB.x, 0.01f);
						ImGui::DragFloat("Capsule A radius", &cca.radius, 0.01f);
						ImGui::DragFloat3("Capsule B center A", &ccb.centerA.x, 0.01f);
						ImGui::DragFloat3("Capsule B center B", &ccb.centerB.x, 0.01f);
						ImGui::DragFloat("Capsule B radius", &ccb.radius, 0.01f);
						break;
					}
					case TestCase::CapsuleBoxIntersection:
					{
						//static glm::vec3 boxOrientation;
						CapsuleCollider& cc = e_Capsule[0].GetComponent<CapsuleCollider>();
						BoxCollider& bc = e_Box[0].GetComponent<BoxCollider>();
						ImGui::DragFloat3("Capsule center A", &cc.centerA.x, 0.01f);
						ImGui::DragFloat3("Capsule center B", &cc.centerB.x, 0.01f);
						ImGui::DragFloat("Capsule radius", &cc.radius, 0.01f);
						ImGui::DragFloat3("Box center", &bc.center.x, 0.01f);
						ImGui::DragFloat3("Box size", &bc.size.x, 0.01f);
						ImGui::DragFloat3("Box orientation", &boxOrientationA.x, 0.01f);
						bc.orientation = glm::quat(boxOrientationA);
						break;
					}
					case TestCase::BoxBoxIntersection:
					{
						//static glm::vec3 boxOrientationA;
						//static glm::vec3 boxOrientationB;
						BoxCollider& bca = e_Box[0].GetComponent<BoxCollider>();
						BoxCollider& bcb = e_Box[1].GetComponent<BoxCollider>();
						ImGui::DragFloat3("Box A center", &bca.center.x, 0.01f);
						ImGui::DragFloat3("Box A size", &bca.size.x, 0.01f);
						ImGui::DragFloat3("Box A orientation", &boxOrientationA.x, 0.01f);
						ImGui::DragFloat3("Box B center", &bcb.center.x, 0.01f);
						ImGui::DragFloat3("Box B size", &bcb.size.x, 0.01f);
						ImGui::DragFloat3("Box B orientation", &boxOrientationB.x, 0.01f);
						bca.orientation = glm::quat(boxOrientationA);
						bcb.orientation = glm::quat(boxOrientationB);
						break;
					}
					}
					break;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		ExampleViewer::ImGuiCall();
	}
}