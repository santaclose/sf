#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>
#include <fstream>

#include <Defaults.h>
#include <Game.h>
#include <Math.hpp>
#include <Input.h>
#include <FileUtils.h>

#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>

#include "../Terrain.hpp"

#define MOUSE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.12

#define GIMBAL_ROTATION_SPEED 15.0f
#define MODEL_OFFSET 50.0
#define MIN_CAMERA_DISTANCE 0.5f

#define GIMBAL_OFFSET_SHANYUNG 1.0f
#define GIMBAL_OFFSET_FOX 0.5f

namespace sf
{
	namespace Game
	{
		Scene scene;
		Entity gimbal, cameraObject;
		Entity shanyung, fox;

		glm::vec3 targetGimbalRotation;

		float cameraDistance;

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

		SkeletonData* foxSkeleton;
		MeshData* foxMesh;
		uint32_t foxBlendSpace;
		float foxBlendSpaceCurrentX;
		std::vector<float> foxWeights;
		std::vector<float> foxSpeedPerAnimation;

		Material characterMaterial;
		Terrain terrain;

		void UpdateCamera(float deltaTime, Entity targetCharacter, float gimbalOffsetY)
		{
			cameraDistance -= glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollUp() ? SCROLL_SENSITIVITY : 0.0f);
			cameraDistance += glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollDown() ? SCROLL_SENSITIVITY : 0.0f);
			cameraDistance = glm::max(MIN_CAMERA_DISTANCE, cameraDistance);

			gimbal.GetComponent<Transform>().position = targetCharacter.GetComponent<Transform>().position + glm::vec3(0.0f, gimbalOffsetY, 0.0f);
			targetGimbalRotation.y -= Input::MousePosDeltaX() * MOUSE_SENSITIVITY;
			targetGimbalRotation.x += Input::MousePosDeltaY() * MOUSE_SENSITIVITY;
			targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

			gimbal.GetComponent<Transform>().rotation = glm::slerp(gimbal.GetComponent<Transform>().rotation, glm::quat(targetGimbalRotation), deltaTime * GIMBAL_ROTATION_SPEED);
			Transform& co_t = cameraObject.GetComponent<Transform>();
			co_t.position = glm::vec3(gimbal.GetComponent<Transform>().position + gimbal.GetComponent<Transform>().Forward() * cameraDistance);
			co_t.LookAt(gimbal.GetComponent<Transform>().position, glm::vec3(0.0, 1.0, 0.0));

			float terrainY;
			terrain.Sample(co_t.position, terrainY);
			co_t.position.y = glm::max(co_t.position.y, terrainY);
		}

		void SwitchCharacter()
		{
			bool f = fox.IsEnabled();
			fox.SetEnabled(!f);
			shanyung.SetEnabled(f);
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

		targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);
		cameraDistance = 3.0;
		shanyungBlendSpaceCurrentPos = { 0.0f, 0.0f };
		foxBlendSpaceCurrentX = 0.0f;

		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();

		gimbal.AddComponent<Transform>();

		cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();
		characterMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		characterMaterial.fragShaderFilePath = "assets/shaders/default.frag";

		terrain.Create(scene, "../Downloads/Telegram Desktop/test.r16", 0.5566f, 152.0f, 41,
			glm::vec3(-(float)(1025 - 1) * 0.5f * 0.5566f, 0.0f, (float)(1025 - 1) * 0.5f * 0.5566f));

		int gltfid;
		{
			shanyung = scene.CreateEntity();
			Transform& e_t = shanyung.AddComponent<Transform>();
			e_t.rotation = glm::quat(glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f));
			e_t.rotation *= glm::quat(glm::vec3(0.0f, 0.0f, glm::radians(180.0f)));

			shanyungSkeleton = new SkeletonData();
			shanyungMesh = new MeshData(&characterVertexLayout);
			gltfid = GltfImporter::Load("assets/examples/shanyung_blendspace2d.glb");
			GltfImporter::GenerateSkeleton(gltfid, *shanyungSkeleton);
			GltfImporter::GenerateMeshData(gltfid, *shanyungMesh);
			MeshProcessor::RemoveUnusedBones(*shanyungMesh, *shanyungSkeleton);
			shanyung.AddComponent<SkinnedMesh>(shanyungMesh, &characterMaterial, shanyungSkeleton);

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
			Transform& e_t = fox.AddComponent<Transform>();
			e_t.rotation = glm::quat(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
			e_t.scale = 0.01f;

			foxSkeleton = new SkeletonData();
			foxMesh = new MeshData(&characterVertexLayout);
			gltfid = GltfImporter::Load("assets/examples/Fox.glb");
			GltfImporter::GenerateSkeleton(gltfid, *foxSkeleton);
			GltfImporter::GenerateMeshData(gltfid, *foxMesh);
			MeshProcessor::RemoveUnusedBones(*foxMesh, *foxSkeleton);
			MeshProcessor::ComputeNormals(*foxMesh);
			fox.AddComponent<SkinnedMesh>(foxMesh, &characterMaterial, foxSkeleton);

			foxWeights.resize(4);
			foxSpeedPerAnimation = { 0.0f, 1.5f, 6.0f };
			foxBlendSpace = foxSkeleton->AddNodeBlendSpace1D({ {0, 1.0f, 0.0f}, {1, 1.0f, 0.5f}, {2, 2.3f, 1.0f} }, 0.0f, foxWeights.data());
			foxSkeleton->SetAnimate(true);
			fox.SetEnabled(false);
		}

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
			e_t.rotation = glm::slerp(e_t.rotation, glm::quat(glm::vec3(0.0f, targetGimbalRotation.y, 0.0f)) * glm::quat(glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f)), deltaTime * (glm::length(targetSpeed) > 0.01f ? 1.0f : 0.0f) * 4.0f);
			e_t.position += -e_t.Up() * targetSpeed.y * deltaTime;
			e_t.position += -e_t.Right() * targetSpeed.x * deltaTime;

			terrain.Sample(e_t.position, e_t.position.y);

			UpdateCamera(deltaTime, shanyung, GIMBAL_OFFSET_SHANYUNG);
		}
		if (fox.IsEnabled())
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
			e_t.position += -e_t.Forward() * targetSpeed * deltaTime;

			terrain.Sample(e_t.position, e_t.position.y);
			glm::vec3 deltaForward = e_t.position + e_t.Forward() * 0.001f;
			terrain.Sample(deltaForward, deltaForward.y);
			deltaForward -= e_t.position;
			if (targetBlendSpaceX > 0.0f)
				e_t.rotation = glm::slerp(e_t.rotation,
					glm::quatLookAt(camForwardFlat * -inputVector.y + camRightFlat * -inputVector.x, glm::vec3(0.0f, 1.0f, 0.0f)) *
					glm::quatLookAt(glm::normalize(glm::vec3(0.0f, deltaForward.y, -0.001f)),
						glm::vec3(0.0f, 1.0f, 0.0f)), deltaTime * 5.0f);

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