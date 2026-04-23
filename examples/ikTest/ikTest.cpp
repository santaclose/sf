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

namespace sf
{
	namespace Game
	{
		Scene scene;
		std::vector<Entity> galleryObjects;
		Entity boneLocalizer;
		Entity ikTarget;
		Entity ikPole;

		glm::quat modelRotation;
		float modelRotationY;

		bool rotationEnabled;

		Material meshMaterial;
		SkeletonData* skeletons;
		MeshData* meshes;
		int* currentAnimation;

		int selectedModel;

		glm::vec3 ikTargetPos = glm::vec3(0.244f, 1.235f, -0.012f);
		glm::vec3 ikPolePos = glm::vec3(0.647f, -0.112f, -0.661f);
		glm::vec3 ikLeafRotEuler;
		glm::quat ikLeafRot;
		uint32_t boneFinder = 0;

		BufferLayout vertexLayout = BufferLayout({
			BufferComponent::Position,
			BufferComponent::Normal,
			BufferComponent::BoneIndices,
			BufferComponent::BoneWeights
		});

		void GalleryChange(bool next)
		{
			int prevModel = selectedModel;
			selectedModel += next ? 1 : -1;
			selectedModel = Math::Mod(selectedModel, (int)galleryObjects.size());
			galleryObjects[prevModel].SetEnabled(false);
			galleryObjects[selectedModel].SetEnabled(true);
		}

		void AnimationChange(bool next)
		{
			currentAnimation[selectedModel] = Math::Mod(currentAnimation[selectedModel] + (next ? 1 : -1), skeletons[selectedModel].m_animations.size());
			for (int i = 0; i < skeletons[selectedModel].m_nodes.size(); i++)
				skeletons[selectedModel].m_nodes[i].single.weight = i == currentAnimation[selectedModel] ? 1.0f : 0.0f;
		}
	}

	Game::InitData Game::GetInitData()
	{
		return InitData();
	}

	void Game::Initialize(int argc, char** argv)
	{
		modelRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		modelRotationY = 0.0f;
		rotationEnabled = false;
		selectedModel = 0;

		ExampleViewer::Initialize(scene);
		meshMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		meshMaterial.fragShaderFilePath = "assets/shaders/default.frag";

		boneLocalizer = scene.CreateEntity();
		Transform& boneLocalizerT = boneLocalizer.AddComponent<Transform>();
		boneLocalizerT.scale = 0.2;
		boneLocalizer.AddComponent<Mesh>(&Defaults::MeshDataSphere(), &meshMaterial);
		ikTarget = scene.CreateEntity();
		Transform& ikTargetT = ikTarget.AddComponent<Transform>();
		ikTargetT.scale = 0.05;
		ikTarget.AddComponent<Mesh>(&Defaults::MeshDataSphere(), &meshMaterial);
		ikPole = scene.CreateEntity();
		Transform& ikPoleT = ikPole.AddComponent<Transform>();
		ikPoleT.scale = 0.05;
		ikPole.AddComponent<Mesh>(&Defaults::MeshDataSphere(), &meshMaterial);

		int gltfid;
		skeletons = new SkeletonData[2];
		meshes = new MeshData[2];
		currentAnimation = new int[2];
		memset(currentAnimation, 0, sizeof(int) * 2);
		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& e_t = galleryObjects.back().AddComponent<Transform>();
			// e_t.scale = 0.025f;
			// e_t.position.y -= 1.0f;

			// gltfid = GltfImporter::Load("examples/ikTest/twoBoneMesh.glb");
			gltfid = GltfImporter::Load("/home/san/Downloads/Ch36_nonPBR_out/Ch36_nonPBR.gltf");
			GltfImporter::GenerateSkeleton(gltfid, skeletons[0]);
			meshes[0] = MeshData(&vertexLayout);
			GltfImporter::GenerateMeshData(gltfid, meshes[0]);
			MeshProcessor::ComputeNormals(meshes[0]);
			SkinnedMesh& objectMesh = galleryObjects.back().AddComponent<SkinnedMesh>(&(meshes[0]), &meshMaterial, &(skeletons[0]));
			for (int i = 0; i < skeletons[0].m_animations.size(); i++)
				skeletons[0].AddNodeSingle(i);
			skeletons[0].SetAnimate(true);
		}

		skeletons[0].AddTwoBoneIkData(10, &ikTargetPos, &ikLeafRot, &ikPolePos);

		for (int i = 0; i < galleryObjects.size(); i++)
			galleryObjects[i].SetEnabled(i == selectedModel);
	}

	void Game::Terminate()
	{
		delete[] meshes;
		delete[] skeletons;
		delete[] currentAnimation;
		for (Entity e : galleryObjects)
			scene.DestroyEntity(e);
		galleryObjects.clear();
		ExampleViewer::Terminate(scene);
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		ExampleViewer::UpdateCamera(deltaTime);

		if (Input::KeyDown(Input::KeyCode::Space))
			rotationEnabled = !rotationEnabled;
		else if (Input::KeyDown(Input::KeyCode::Right))
			GalleryChange(true);
		else if (Input::KeyDown(Input::KeyCode::Left))
			GalleryChange(false);
		else if (Input::KeyDown(Input::KeyCode::KP5))
			skeletons[selectedModel].SetAnimate(!skeletons[selectedModel].GetAnimate());
		else if (Input::KeyDown(Input::KeyCode::KP6))
			AnimationChange(true);
		else if (Input::KeyDown(Input::KeyCode::KP4))
			AnimationChange(false);

		if (skeletons[selectedModel].GetAnimate())
			skeletons[selectedModel].UpdateAnimation(deltaTime);
		Transform boneFinderTransform = skeletons[selectedModel].GetBoneTransform(boneFinder);
		Transform& boneLocalizerT = boneLocalizer.GetComponent<Transform>();
		boneLocalizerT.position = boneFinderTransform.position;

		modelRotationY += Input::MousePosDeltaX() * MOUSE_SENSITIVITY * MODEL_ROTATION_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		modelRotationY = glm::mix(modelRotationY, 0.0f, deltaTime * 7.0f);
		modelRotation = glm::quat(glm::vec3(0.0f, modelRotationY * deltaTime, 0.0f));
		if (rotationEnabled)
			modelRotation *= glm::quat(glm::vec3(0.0f, 1.5f * deltaTime, 0.0f));

		Transform& objectTransform = galleryObjects[selectedModel].GetComponent<Transform>();
		objectTransform.rotation = modelRotation * objectTransform.rotation;

		Transform& ikTargetT = ikTarget.GetComponent<Transform>();
		ikTargetT.position = ikTargetPos;

		Transform& ikPoleT = ikPole.GetComponent<Transform>();
		ikPoleT.position = ikPolePos;
		// sf::Renderer::AddLine(ikTargetPos, ikTargetPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	}
	void Game::ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Gallery"))
			{
				if (ImGui::MenuItem("Previous", "Left arrow")) { GalleryChange(false); }
				if (ImGui::MenuItem("Next", "Right arrow")) { GalleryChange(true); }
				if (ImGui::MenuItem("Toggle rotation", "Space")) { rotationEnabled = !rotationEnabled; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Animation"))
			{
				if (ImGui::MenuItem("Animate", "NumPad5")) { skeletons[selectedModel].SetAnimate(!skeletons[selectedModel].GetAnimate()); }
				if (ImGui::MenuItem("Previous", "NumPad4")) { AnimationChange(false); }
				if (ImGui::MenuItem("Next", "NumPad6")) { AnimationChange(true); }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Ik test"))
			{
				ImGui::DragScalar("Find bone", ImGuiDataType_U32, &boneFinder);
				ImGui::DragFloat3("Target position", &ikTargetPos.x, 0.001);
				ImGui::DragFloat3("Pole position", &ikPolePos.x, 0.001);
				ImGui::DragFloat3("Leaf rotation", &ikLeafRotEuler.x, 0.001);
				ikLeafRot = glm::quat(ikLeafRotEuler);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		ExampleViewer::ImGuiCall();
	}
}