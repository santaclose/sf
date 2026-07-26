#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>
#include <fstream>

#include <Defaults.h>
#include <Game.h>
#include <Math.hpp>
#include <Input.h>
#include <FileUtils.h>
#include <Random.h>
#include <EntityIntersect.h>

#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>


#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>
#include <Components/MeshCollider.h>

#include "../Terrain.hpp"

#define MOUSE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.12

#define GIMBAL_ROTATION_SPEED 15.0f
#define MODEL_OFFSET 50.0
#define MIN_CAMERA_DISTANCE 0.5f

#define GIMBAL_OFFSET_SHANYUNG 1.0f
#define GIMBAL_OFFSET_FOX 0.5f

#define HOUSE_COUNT 400

namespace sf
{
	namespace Game
	{
		Scene scene;
		Entity gimbal, cameraObject;
		Entity shanyung, fox, foxCol;

		Entity houses[HOUSE_COUNT];
		MeshData* houseMesh;
		MeshData* houseColMesh;

		glm::vec3 targetGimbalRotation;

		float cameraDistance;

		BufferLayout collisionVertexLayout = BufferLayout({
			BufferComponent::Position,
		});

		BufferLayout staticVertexLayout = BufferLayout({
			BufferComponent::Position,
			BufferComponent::Normal,
		});

		BufferLayout characterVertexLayout = BufferLayout({
			BufferComponent::Position,
			BufferComponent::Normal,
			BufferComponent::BoneIndices,
			BufferComponent::BoneWeights
		});
		SkeletonData* shanyungSkeleton;
		MeshData* shanyungMesh;
		uint32_t shanyungBlendSpace;
		glm::vec2 shanyungBlendSpaceCurrentPos;
		std::vector<float> shanyungWeights;
		std::vector<glm::vec2> shanyungSpeedPerAnimation;
		float shanyungVerticalSpeed = 0.0f;
		float shanyungCapsuleSeparation;

		SkeletonData* foxSkeleton;
		MeshData* foxMesh;
		uint32_t foxBlendSpace;
		float foxBlendSpaceCurrentX;
		std::vector<float> foxWeights;
		std::vector<float> foxSpeedPerAnimation;
		float foxVerticalSpeed = 0.0f;
		float foxSphereSeparation;

		Material characterMaterial;
		Material vertexAoMaterial;
		Terrain terrain;

		float shanyungLodDistances[] = { 5.0f, 20.0f, 45.0f, 1000.0f };
		float shanyungLodRatios[] = { 1.0f, 0.25f, 0.0625f, 0.001f };
		float houseLodDistances[] = { 50.0f, 1000.0f };
		float houseLodRatios[] = { 1.0f, 0.5f };

		void UpdateCamera(float deltaTime, Entity targetCharacter, float gimbalOffsetY)
		{
			Transform& gimbalT = gimbal.GetComponent<Transform>();
			Transform& camT = cameraObject.GetComponent<Transform>();

			cameraDistance -= glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollUp() ? SCROLL_SENSITIVITY : 0.0f);
			cameraDistance += glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollDown() ? SCROLL_SENSITIVITY : 0.0f);
			cameraDistance = glm::max(MIN_CAMERA_DISTANCE, cameraDistance);

			gimbalT.position = targetCharacter.GetComponent<Transform>().position + glm::vec3(0.0f, gimbalOffsetY, 0.0f);
			targetGimbalRotation.y -= Input::MousePosDeltaX() * MOUSE_SENSITIVITY;
			targetGimbalRotation.x += Input::MousePosDeltaY() * MOUSE_SENSITIVITY;
			targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

			gimbalT.rotation = glm::slerp(gimbalT.rotation, glm::quat(targetGimbalRotation), deltaTime * GIMBAL_ROTATION_SPEED);

			camT.position = gimbalT.position + gimbalT.Forward() * cameraDistance;

			Geometry::RayHit rh;
			for (uint32_t i = 0; i < HOUSE_COUNT; i++)
			{
				if (EntityIntersect::MovingSphereMesh(gimbal, camT.position - gimbalT.position, houses[i], &rh))
					camT.position = gimbalT.position + gimbalT.Forward() * rh.distance;
			}

			camT.LookAt(gimbalT.position, glm::vec3(0.0, 1.0, 0.0));
			float terrainY;
			terrain.Sample(camT.position, terrainY);
			camT.position.y = glm::max(camT.position.y, terrainY);
		}

		void SwitchCharacter()
		{
			bool f = fox.IsEnabled();
			fox.SetEnabled(!f);
			foxCol.SetEnabled(!f);
			shanyung.SetEnabled(f);
		}

		float CharacterStepCastShanyung(Entity charEntity)
		{
			CapsuleCollider& cc = charEntity.GetComponent<CapsuleCollider>();

			bool hitMesh = false;
			Geometry::RayHit rh;
			for (int i = 0; i < HOUSE_COUNT; i++)
			{
				if (EntityIntersect::MovingCapsuleMesh(charEntity, glm::vec3(0.0f, -shanyungCapsuleSeparation * 2.0f, 0.0f), houses[i], &rh))
				{
					hitMesh = true;
					break;
				}
			}
			CapsuleCollider cc_world = EntityIntersect::WorldSpace<CapsuleCollider>(charEntity);
			glm::vec3 capsuleBottom = cc_world.centerA - glm::vec3(0.0f, cc_world.radius, 0.0f);
			float terrainY;
			terrain.Sample(capsuleBottom, terrainY);
			float terrainDis = capsuleBottom.y - terrainY;
			if (hitMesh)
				return glm::min(terrainDis, rh.distance);
			else
				return terrainDis;
		}

		float CharacterStepCastFox(Entity charEntity)
		{
			bool hitMesh = false;
			Geometry::RayHit rh;
			for (int i = 0; i < HOUSE_COUNT; i++)
			{
				if (EntityIntersect::MovingSphereMesh(charEntity, glm::vec3(0.0f, -foxSphereSeparation * 2.0f, 0.0f), houses[i], &rh))
				{
					hitMesh = true;
					break;
				}
			}
			SphereCollider sc_world = EntityIntersect::WorldSpace<SphereCollider>(charEntity);
			glm::vec3 sphereBottom = sc_world.center - glm::vec3(0.0f, sc_world.radius, 0.0f);
			float terrainY;
			terrain.Sample(sphereBottom, terrainY);
			float terrainDis = sphereBottom.y - terrainY;
			if (hitMesh)
				return glm::min(terrainDis, rh.distance);
			else
				return terrainDis;
		}
	}

	Game::InitData Game::GetInitData()
	{
		InitData id;
		id.cursorRequired = false;
		id.toolBarEnabled = false;
		return id;
	}

	void Game::Initialize(int argc, char** argv)
	{
		FileUtils::CreateFolder("assets/examples");
		FileUtils::DownloadFiles({
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/Fox/glTF-Binary/Fox.glb",
			"https://us.v-cdn.net/5021068/uploads/editor/ha/7frj09nru4zu.png",
			"https://github.com/santaclose/sample_models/raw/master/shanyung_blendspace2d.glb"
			}, "assets/examples/");

		characterMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		characterMaterial.fragShaderFilePath = "assets/shaders/default.frag";
		vertexAoMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		vertexAoMaterial.fragShaderFilePath = "assets/shaders/vertexAo.frag";

		targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);
		cameraDistance = 3.0;
		shanyungBlendSpaceCurrentPos = { 0.0f, 0.0f };
		foxBlendSpaceCurrentX = 0.0f;

		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();

		gimbal.AddComponent<Transform>();
		SphereCollider& c_sc = gimbal.AddComponent<SphereCollider>();
		c_sc.radius = 0.15;

		Camera& cam = cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();

		terrain.Create(scene, "../Downloads/Telegram Desktop/test.r16", 0.5566f, 152.0f, 41,
			glm::vec3(-(float)(1025 - 1) * 0.5f * 0.5566f, 0.0f, (float)(1025 - 1) * 0.5f * 0.5566f));

		// Houses
		{
			houseMesh = new MeshData(&staticVertexLayout);
			int objid = ObjImporter::Load("assets/examples/house.obj");
			ObjImporter::GenerateMeshData(objid, *houseMesh);
			MeshProcessor::ComputeNormals(*houseMesh);
			std::vector<LOD> houseLods = GenerateLODs(*houseMesh, houseLodRatios, houseLodDistances, 2);

			houseColMesh = new MeshData(&collisionVertexLayout);
			objid = ObjImporter::Load("assets/examples/houseCol.obj");
			ObjImporter::GenerateMeshData(objid, *houseColMesh);

			for (int i = 0; i < HOUSE_COUNT; i++)
			{
				houses[i] = scene.CreateEntity();
				houses[i].AddComponent<Mesh>(houseLods, &characterMaterial);
				houses[i].AddComponent<MeshCollider>(houseColMesh);
				Transform& houseT = houses[i].AddComponent<Transform>();
				houseT.rotation = glm::quat(glm::vec3(0.0f, Random::Float() * 3.14159265 * 2.0f, 0.0f));
				if (i == 0)
				{
					houseT.rotation = glm::quat(glm::vec3(0.0f, glm::radians(-10.0f) * 2.0f, 0.0f));
					houseT.position.x = 10.0f;
					terrain.Sample(houseT.position, houseT.position.y);
					continue;
				}
				bool collidesWithOtherHouse;
				do
				{
					houseT.position.x = (Random::Float() - 0.5f) * 1025.0f * 0.5566f;
					houseT.position.z = (Random::Float() - 0.5f) * 1025.0f * 0.5566f;
					terrain.Sample(houseT.position, houseT.position.y);
					collidesWithOtherHouse = false;
					for (int j = 0; j < i; j++)
					{
						if (EntityIntersect::MeshBoundingSpheres(houses[i], houses[j]))
						{
							collidesWithOtherHouse = true;
							break;
						}
					}
				} while (collidesWithOtherHouse);
			}
		}

		{
			shanyung = scene.CreateEntity();
			shanyung.AddComponent<Transform>();

			shanyungSkeleton = new SkeletonData();
			shanyungMesh = new MeshData(&characterVertexLayout);
			int gltfid = GltfImporter::Load("assets/examples/shanyung_blendspace2d.glb");
			GltfImporter::GenerateSkeleton(gltfid, *shanyungSkeleton);
			GltfImporter::GenerateMeshData(gltfid, *shanyungMesh);
			MeshProcessor::RemoveUnusedBones(*shanyungMesh, *shanyungSkeleton);
			{
				Transform shanyungImportTransform;
				shanyungImportTransform.rotation = glm::quat(glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f));
				shanyungImportTransform.rotation *= glm::quat(glm::vec3(0.0f, 0.0f, glm::radians(180.0f)));
				MeshProcessor::TransformMesh(*shanyungMesh, shanyungImportTransform);
				MeshProcessor::TransformSkeleton(*shanyungSkeleton, shanyungImportTransform);
			}

			shanyung.AddComponent<SkinnedMesh>(GenerateLODs(*shanyungMesh, shanyungLodRatios, shanyungLodDistances, 4), shanyungSkeleton, &characterMaterial);

			CapsuleCollider& e_cc = shanyung.AddComponent<CapsuleCollider>();
			e_cc.radius = 0.3;
			e_cc.centerA = glm::vec3(0.0f, 0.7f, 0.0f);
			e_cc.centerB = glm::vec3(0.0f, 1.2f, 0.0f);
			shanyungCapsuleSeparation = e_cc.centerA.y - e_cc.radius;

			shanyungWeights.resize(10);
			shanyungSpeedPerAnimation = { {0.0f, 0.0f}, {0.0f, 6.0f}, {-6.0f, 0.0f}, {6.0f, 0.0f}, {0.0f, 1.68f}, {0.0f, -1.08f}, {-0.763f, -0.763f}, {0.763f, -0.763f}, {-1.68f, 0.0f}, {1.68f, 0.0f} };
			shanyungBlendSpace = shanyungSkeleton->AddNodeBlendSpace2D(
				{ {0, 1.0f, {0.0f, 0.0f}}, {1, 1.0f, {0.0f, 1.0f}}, {2, 1.0f, {-1.0f, 0.0f}}, {3, 1.0f, {1.0f, 0.0f}}, {4, 1.0f, {0.0f, 0.5f}}, {5, 1.0f, {0.0f, -0.5f}}, {6, 1.0f, {-0.5f, -0.5f}}, {7, 1.0f, {0.5f, -0.5f}}, {8, 1.0f, {-0.5f, 0.0f}}, {9, 1.0f, {0.5f, 0.0f}} },
				{ 0.0f, 0.0f },
				shanyungWeights.data());
			shanyungSkeleton->SetAnimate(true);
		}
		{
			fox = scene.CreateEntity();
			fox.AddComponent<Transform>();

			foxSkeleton = new SkeletonData();
			foxMesh = new MeshData(&characterVertexLayout);
			int gltfid = GltfImporter::Load("assets/examples/Fox.glb");
			GltfImporter::GenerateSkeleton(gltfid, *foxSkeleton);
			GltfImporter::GenerateMeshData(gltfid, *foxMesh);
			MeshProcessor::RemoveUnusedBones(*foxMesh, *foxSkeleton);
			MeshProcessor::ComputeNormals(*foxMesh);
			{
				Transform foxImportTransform;
				foxImportTransform.rotation = glm::quat(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
				foxImportTransform.scale = 0.01f;
				MeshProcessor::TransformMesh(*foxMesh, foxImportTransform);
				MeshProcessor::TransformSkeleton(*foxSkeleton, foxImportTransform);
			}
			fox.AddComponent<SkinnedMesh>(foxMesh, foxSkeleton, &characterMaterial);

			foxCol = scene.CreateEntity();
			foxCol.AddComponent<Transform>();
			SphereCollider& col_sc = foxCol.AddComponent<SphereCollider>();
			col_sc.radius = 0.3;
			col_sc.center = glm::vec3(0.0f, 0.6f, 0.0f);
			foxSphereSeparation = col_sc.center.y - col_sc.radius;

			foxWeights.resize(4);
			foxSpeedPerAnimation = { 0.0f, 1.5f, 6.0f };
			foxBlendSpace = foxSkeleton->AddNodeBlendSpace1D({ {0, 1.0f, 0.0f}, {1, 1.0f, 0.5f}, {2, 2.3f, 1.0f} }, 0.0f, foxWeights.data());
			foxSkeleton->SetAnimate(true);
		}

		fox.SetEnabled(false);
		foxCol.SetEnabled(false);

		gimbal.GetComponent<Transform>().position = glm::vec3(0.0, GIMBAL_OFFSET_SHANYUNG, 0.0);
		gimbal.GetComponent<Transform>().LookAt(glm::vec3(0.0, 1.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		cameraObject.GetComponent<Transform>().position = glm::vec3(0.0, GIMBAL_OFFSET_SHANYUNG, cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(glm::vec3(0.0, GIMBAL_OFFSET_SHANYUNG, 0.0), glm::vec3(0.0, 1.0, 0.0));
	}

	void Game::Terminate()
	{
		scene.DestroyEntity(gimbal);
		scene.DestroyEntity(cameraObject);
		scene.DestroyEntity(shanyung);
		scene.DestroyEntity(fox);
		terrain.Destroy(scene);

		delete shanyungMesh;
		delete shanyungSkeleton;
		delete foxMesh;
		delete foxSkeleton;
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (Input::KeyDown(Input::KeyCode::Left) || Input::KeyDown(Input::KeyCode::Right))
			SwitchCharacter();

		if (shanyung.IsEnabled())
		{
			glm::vec2 targetBlendSpacePos = { 0.0f, 0.0f };
			if (Input::Key(Input::KeyCode::D))
				targetBlendSpacePos += glm::vec2(0.5f, 0.0f);
			if (Input::Key(Input::KeyCode::A))
				targetBlendSpacePos -= glm::vec2(0.5f, 0.0f);
			if (Input::Key(Input::KeyCode::W))
				targetBlendSpacePos += glm::vec2(0.0f, 0.5f);
			if (Input::Key(Input::KeyCode::S))
				targetBlendSpacePos -= glm::vec2(0.0f, 0.5f);
			if (Input::Key(Input::KeyCode::LeftShift) && targetBlendSpacePos.y >= 0.0f)
				targetBlendSpacePos *= 2.0f;

			shanyungBlendSpaceCurrentPos = glm::mix(shanyungBlendSpaceCurrentPos, targetBlendSpacePos, deltaTime * 7.0f);
			shanyungSkeleton->SetBlendSpace2DPosition(shanyungBlendSpace, shanyungBlendSpaceCurrentPos);
			shanyungSkeleton->UpdateAnimation(deltaTime);
			Transform& e_t = shanyung.GetComponent<Transform>();
			glm::vec2 targetSpeed;
			Math::WeightedBlend(shanyungSpeedPerAnimation.data(), shanyungWeights.data(), shanyungWeights.size(), targetSpeed);
			e_t.rotation = glm::slerp(e_t.rotation, glm::quat(glm::vec3(0.0f, glm::radians(180.0f) + targetGimbalRotation.y, 0.0f)), deltaTime * (glm::length(targetSpeed) > 0.01f ? 1.0f : 0.0f) * 4.0f);
			glm::vec3 disp = (e_t.Forward() * targetSpeed.y + e_t.Right() * targetSpeed.x) * deltaTime;

			uint32_t outNCount;
			Geometry::ContactData outCd[4];
			for (int i = 0; i < HOUSE_COUNT; i++)
			{
				if (EntityIntersect::CapsuleMesh(shanyung, houses[i], 4, &outNCount, outCd))
				{
					for (uint32_t i = 0; i < outNCount; i++)
					{
						glm::vec3 flatCapsuleNormal = glm::normalize(glm::vec3(outCd[i].normalA.x, 0.0f, outCd[i].normalA.z));
						glm::vec3 flatTriNormal = glm::normalize(glm::vec3(outCd[i].normalB.x, 0.0f, outCd[i].normalB.z));

						Renderer::AddLine(e_t.position + glm::vec3(0.0f, 0.3f, 0.0f), e_t.position + glm::vec3(0.0f, 0.3f, 0.0f) + flatTriNormal, glm::vec3(0.0f, 0.0f, 1.0f));
						Renderer::AddLine(e_t.position + glm::vec3(0.0f, 0.4f, 0.0f), e_t.position + glm::vec3(0.0f, 0.4f, 0.0f) + flatCapsuleNormal, glm::vec3(1.0f, 0.0f, 0.0f));
						if (glm::dot(disp, flatTriNormal) < 0.0f && glm::dot(flatCapsuleNormal, flatTriNormal) < -0.777f)
							disp -= glm::dot(disp, flatTriNormal) * flatTriNormal;
					}
				}
			}
			e_t.position += disp;

			float outT = CharacterStepCastShanyung(shanyung);
			if (outT <= (shanyungVerticalSpeed != 0.0f ? shanyungCapsuleSeparation : (shanyungCapsuleSeparation + 0.3f)))
			{
				e_t.position.y += shanyungCapsuleSeparation - outT;
				shanyungVerticalSpeed = 0.0f;
			}
			else
			{
				shanyungVerticalSpeed += 9.8f * deltaTime;
				e_t.position.y -= shanyungVerticalSpeed * deltaTime;
			}

			UpdateCamera(deltaTime, shanyung, GIMBAL_OFFSET_SHANYUNG);
		}
		else if (fox.IsEnabled())
		{
			glm::vec2 inputVector = { 0.0f, 0.0f };
			if (Input::Key(Input::KeyCode::A))
				inputVector.x = -1.0f;
			if (Input::Key(Input::KeyCode::D))
				inputVector.x = 1.0f;
			if (Input::Key(Input::KeyCode::S))
				inputVector.y = -1.0f;
			if (Input::Key(Input::KeyCode::W))
				inputVector.y = 1.0f;
			glm::normalize(inputVector);
			float targetBlendSpaceX = 0.0f;
			if (Input::Key(Input::KeyCode::A) || Input::Key(Input::KeyCode::D) || Input::Key(Input::KeyCode::W) || Input::Key(Input::KeyCode::S))
				targetBlendSpaceX += 0.5;
			if (Input::Key(Input::KeyCode::LeftShift))
				targetBlendSpaceX *= 2.0f;

			foxBlendSpaceCurrentX = glm::mix(foxBlendSpaceCurrentX, targetBlendSpaceX, deltaTime * 7.0f);
			foxSkeleton->SetBlendSpace1DPosition(foxBlendSpace, foxBlendSpaceCurrentX);
			foxSkeleton->UpdateAnimation(deltaTime);
			Transform& e_t = fox.GetComponent<Transform>();

			float targetSpeed;
			Math::WeightedBlend(foxSpeedPerAnimation.data(), foxWeights.data(), foxWeights.size(), targetSpeed);
			glm::vec3 camForwardFlat = cameraObject.GetComponent<Transform>().Forward();
			camForwardFlat.y = 0;
			glm::normalize(camForwardFlat);
			glm::vec3 camRightFlat = glm::cross(camForwardFlat, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec3 disp = e_t.Forward() * targetSpeed * deltaTime;

			bool collidedWithWall = false;
			uint32_t outNCount;
			Geometry::ContactData outCd[4];
			for (int i = 0; i < HOUSE_COUNT; i++)
			{
				if (EntityIntersect::SphereMesh(foxCol, houses[i], 4, &outNCount, outCd))
				{
					for (uint32_t i = 0; i < outNCount; i++)
					{
						glm::vec3 flatSphereNormal = glm::normalize(glm::vec3(outCd[i].normalA.x, 0.0f, outCd[i].normalA.z));
						glm::vec3 flatTriNormal = glm::normalize(glm::vec3(outCd[i].normalB.x, 0.0f, outCd[i].normalB.z));

						Renderer::AddLine(e_t.position + glm::vec3(0.0f, 0.3f, 0.0f), e_t.position + glm::vec3(0.0f, 0.3f, 0.0f) + flatTriNormal, glm::vec3(0.0f, 0.0f, 1.0f));
						Renderer::AddLine(e_t.position + glm::vec3(0.0f, 0.4f, 0.0f), e_t.position + glm::vec3(0.0f, 0.4f, 0.0f) + flatSphereNormal, glm::vec3(1.0f, 0.0f, 0.0f));
						if (glm::dot(disp, flatTriNormal) < 0.0f && glm::dot(flatSphereNormal, flatTriNormal) < -0.777f)
						{
							disp -= glm::dot(disp, flatTriNormal) * flatTriNormal;
							collidedWithWall = true;
						}
					}
				}
			}
			e_t.position += disp;

			Transform& col_t = foxCol.GetComponent<Transform>();
			col_t.position = e_t.position;
			float outT = CharacterStepCastFox(foxCol);
			if (outT <= (foxVerticalSpeed != 0.0f ? foxSphereSeparation : (foxSphereSeparation + 0.3f)))
			{
				e_t.position.y += foxSphereSeparation - outT;
				foxVerticalSpeed = 0.0f;

				// sample again for inclination
				col_t.position = e_t.position + e_t.Forward() * 0.001f;
				outT = CharacterStepCastFox(foxCol);
				col_t.position.y += foxSphereSeparation - outT;

				glm::vec3 deltaForward = col_t.position - e_t.position;
				if (collidedWithWall)
					deltaForward.y = 0.0f;

				if (targetBlendSpaceX > 0.00001f)
					e_t.rotation = glm::slerp(e_t.rotation,
						glm::quatLookAt(camForwardFlat * inputVector.y + camRightFlat * inputVector.x, glm::vec3(0.0f, 1.0f, 0.0f)) *
						glm::quatLookAt(glm::normalize(glm::vec3(0.0f, deltaForward.y, -0.001f)),
							glm::vec3(0.0f, 1.0f, 0.0f)), deltaTime * 5.0f);

			}
			else
			{
				foxVerticalSpeed += 9.8f * deltaTime;
				e_t.position.y -= foxVerticalSpeed * deltaTime;
			}


			UpdateCamera(deltaTime, fox, GIMBAL_OFFSET_FOX);
		}
	}

	void Game::ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Character"))
			{
				if (ImGui::MenuItem("Shanyung", "Right/Left arrow", shanyung.IsEnabled()))
					SwitchCharacter();
				if (ImGui::MenuItem("Fox", "Right/Left arrow", fox.IsEnabled()))
					SwitchCharacter();
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
}