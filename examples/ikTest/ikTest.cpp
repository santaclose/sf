#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>
#include <fstream>

#include <Defaults.h>
#include <Game.h>
#include <MeshProcessor.h>
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

#include "../Viewer.hpp"

#define MODEL_ROTATION_SENSITIVITY 10.0
#define MODEL_OFFSET 50.0
#define RIGHT_FOOT_ROTATION_OFFSET 0

namespace sf
{
	namespace Game
	{
		Scene scene;

		BufferLayout vertexLayout = BufferLayout({
			BufferComponent::Position,
			BufferComponent::Normal,
			BufferComponent::BoneIndices,
			BufferComponent::BoneWeights
		});

		Material meshMaterial;

		Entity humanModel;
		Entity boneLocalizer;
		Entity leftHandIkTarget;
		Entity rightFootIkTarget;

		SkeletonData humanSkeleton;
		MeshData humanMesh;

		bool rotationEnabled;
		glm::quat modelRotation;
		float modelRotationY;

		glm::vec3 ikLeftHandTargetPos = glm::vec3(0.244f, 1.359f, 0.345f);
		float ikLeftArmRotOffset = -0.283;
		glm::vec3 ikLeftHandRotEuler = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat ikLeftHandRot = glm::quat(ikLeftHandRotEuler);
		const uint32_t leftElbowBone = 10;

		glm::vec3 ikRightFootTargetPos = glm::vec3(-0.143f, 0.432f, 0.345f);
#if RIGHT_FOOT_ROTATION_OFFSET
		float ikRightLegRotOffset = -0.6;
#endif
		glm::vec3 ikRightFootRotEuler = glm::vec3(1.258f, 0.149f, 178.940f);
		glm::quat ikRightFootRot = glm::quat(ikRightFootRotEuler);
		const uint32_t rightKneeBone = 62;

		uint32_t boneFinder = 0;
	}

	Game::InitData Game::GetInitData()
	{
		return InitData();
	}

	void Game::Initialize(int argc, char** argv)
	{
		FileUtils::CreateFolder("assets/examples");
		FileUtils::DownloadFiles({
			"https://github.com/santaclose/sample_models/raw/master/mannequin_tpose.zip"
			}, "assets/examples/");

		modelRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		modelRotationY = 0.0f;
		rotationEnabled = false;

		ExampleViewer::Initialize(scene);
		meshMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		meshMaterial.fragShaderFilePath = "assets/shaders/default.frag";

		boneLocalizer = scene.CreateEntity();
		Transform& boneLocalizerT = boneLocalizer.AddComponent<Transform>();
		boneLocalizerT.scale = 0.2;
		boneLocalizer.AddComponent<Mesh>(&Defaults::MeshDataSphere(), &meshMaterial);

		leftHandIkTarget = scene.CreateEntity();
		Transform& leftHandIkTargetT = leftHandIkTarget.AddComponent<Transform>();
		leftHandIkTargetT.scale = 0.05;
		leftHandIkTarget.AddComponent<Mesh>(&Defaults::MeshDataSphere(), &meshMaterial);

		rightFootIkTarget = scene.CreateEntity();
		Transform& rightFootIkTargetT = rightFootIkTarget.AddComponent<Transform>();
		rightFootIkTargetT.scale = 0.05;
		rightFootIkTarget.AddComponent<Mesh>(&Defaults::MeshDataSphere(), &meshMaterial);

		int gltfid;
		{
			humanModel = scene.CreateEntity();
			Transform& e_t = humanModel.AddComponent<Transform>();
			gltfid = GltfImporter::Load("assets/examples/mannequin_tpose/mannequin_tpose.gltf");
			GltfImporter::GenerateSkeleton(gltfid, humanSkeleton);
			humanMesh = MeshData(&vertexLayout);
			GltfImporter::GenerateMeshData(gltfid, humanMesh);
			MeshProcessor::ComputeNormals(humanMesh);
			SkinnedMesh& objectMesh = humanModel.AddComponent<SkinnedMesh>(&humanMesh, &meshMaterial, &humanSkeleton);
			for (int i = 0; i < humanSkeleton.m_animations.size(); i++)
				humanSkeleton.AddNodeSingle(i);
			humanSkeleton.SetAnimate(true);
		}

		humanSkeleton.AddTwoBoneIkData(leftElbowBone, &ikLeftHandTargetPos, &ikLeftHandRot, &ikLeftArmRotOffset);
#if RIGHT_FOOT_ROTATION_OFFSET
		humanSkeleton.AddTwoBoneIkData(rightKneeBone, &ikRightFootTargetPos, &ikRightFootRot, &ikRightLegRotOffset);
#else
		humanSkeleton.AddTwoBoneIkData(rightKneeBone, &ikRightFootTargetPos, &ikRightFootRot);
#endif
	}

	void Game::Terminate()
	{
		scene.DestroyEntity(humanModel);
		scene.DestroyEntity(boneLocalizer);
		scene.DestroyEntity(leftHandIkTarget);
		scene.DestroyEntity(rightFootIkTarget);
		ExampleViewer::Terminate(scene);
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		ExampleViewer::UpdateCamera(deltaTime);

		if (Input::KeyDown(Input::KeyCode::Space))
			rotationEnabled = !rotationEnabled;
		else if (Input::KeyDown(Input::KeyCode::KP5))
			humanSkeleton.SetAnimate(!humanSkeleton.GetAnimate());

		if (humanSkeleton.GetAnimate())
			humanSkeleton.UpdateAnimation(deltaTime);

		modelRotationY += Input::MousePosDeltaX() * MOUSE_SENSITIVITY * MODEL_ROTATION_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		modelRotationY = glm::mix(modelRotationY, 0.0f, deltaTime * 7.0f);
		modelRotation = glm::quat(glm::vec3(0.0f, modelRotationY * deltaTime, 0.0f));
		if (rotationEnabled)
			modelRotation *= glm::quat(glm::vec3(0.0f, 1.5f * deltaTime, 0.0f));

		Transform& objectTransform = humanModel.GetComponent<Transform>();
		objectTransform.rotation = modelRotation * objectTransform.rotation;

		Transform& leftHandIkTargetT = leftHandIkTarget.GetComponent<Transform>();
		leftHandIkTargetT.position = objectTransform.rotation * ikLeftHandTargetPos;

		Transform& rightFootIkTargetT = rightFootIkTarget.GetComponent<Transform>();
		rightFootIkTargetT.position = objectTransform.rotation * ikRightFootTargetPos;

		Transform boneFinderTransform = humanSkeleton.GetBoneTransform(boneFinder);
		Transform& boneLocalizerT = boneLocalizer.GetComponent<Transform>();
		boneLocalizerT.position = objectTransform.rotation * boneFinderTransform.position;
	}
	void Game::ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Options"))
			{
				if (ImGui::MenuItem("Toggle rotation", "Space")) { rotationEnabled = !rotationEnabled; }
				if (ImGui::MenuItem("Animate", "NumPad5")) { humanSkeleton.SetAnimate(!humanSkeleton.GetAnimate()); }
				ImGui::DragScalar("Find bone", ImGuiDataType_U32, &boneFinder);

				ImGui::DragFloat3("Left hand target position", &ikLeftHandTargetPos.x, 0.001);
				ImGui::DragFloat3("Left hand rotation", &ikLeftHandRotEuler.x, 0.001);
				ImGui::DragFloat("Left arm rotation offset", &ikLeftArmRotOffset, 0.001);
				ikLeftHandRot = glm::quat(ikLeftHandRotEuler);

				ImGui::DragFloat3("Right foot target position", &ikRightFootTargetPos.x, 0.001);
				ImGui::DragFloat3("Right foot rotation", &ikRightFootRotEuler.x, 0.001);
#if RIGHT_FOOT_ROTATION_OFFSET
				ImGui::DragFloat("Right leg rotation offset", &ikRightLegRotOffset, 0.001);
#endif
				ikRightFootRot = glm::quat(ikRightFootRotEuler);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		ExampleViewer::ImGuiCall();
	}
}