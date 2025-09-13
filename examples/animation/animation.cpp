#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>
#include <fstream>

#include <Defaults.h>
#include <Game.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Input.h>
#include <GameInitializationData.h>
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
	std::string Game::ConfigFilePath = "examples/animation/config.json";

	Scene scene;
	std::vector<Entity> galleryObjects;

	glm::quat modelRotation;
	float modelRotationY;

	bool rotationEnabled;

	SkeletonData* skeletons;
	MeshData* meshes;

	int selectedModel;

	BufferLayout vertexLayout = BufferLayout({
		BufferComponent::VertexPosition,
		BufferComponent::VertexNormal,
		BufferComponent::VertexBoneIndices,
		BufferComponent::VertexBoneWeights
	});

	void Game::Initialize(int argc, char** argv)
	{
		modelRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		modelRotationY = 0.0f;
		rotationEnabled = false;
		selectedModel = 0;

		FileUtils::DownloadFiles({
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/Fox/glTF-Binary/Fox.glb",
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/BrainStem/glTF-Binary/BrainStem.glb"
			}, "examples/animation/");

		ExampleViewer::Initialize(scene);
		uint32_t meshMaterial = Renderer::CreateMaterial(Material("assets/shaders/default.vert", "assets/shaders/default.frag"), vertexLayout);

		int gltfid;
		skeletons = new SkeletonData[2];
		meshes = new MeshData[2];
		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& e_t = galleryObjects.back().AddComponent<Transform>();
			e_t.scale = 0.025f;
			e_t.position.y -= 1.0f;

			gltfid = GltfImporter::Load("examples/animation/Fox.glb");
			GltfImporter::GenerateSkeleton(gltfid, skeletons[0]);
			meshes[0] = MeshData(vertexLayout);
			GltfImporter::GenerateMeshData(gltfid, meshes[0]);
			MeshProcessor::ComputeNormals(meshes[0]);
			SkinnedMesh& objectMesh = galleryObjects.back().AddComponent<SkinnedMesh>(&(meshes[0]), meshMaterial, &(skeletons[0]));
		}
		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& e_t = galleryObjects.back().AddComponent<Transform>();
			e_t.rotation = glm::quat(glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f));
			e_t.scale = 1.7f;
			e_t.position.y -= 1.5f;

			gltfid = GltfImporter::Load("examples/animation/BrainStem.glb");
			GltfImporter::GenerateSkeleton(gltfid, skeletons[1]);
			meshes[1] = MeshData(vertexLayout);
			GltfImporter::GenerateMeshData(gltfid, meshes[1]);
			SkinnedMesh& objectMesh = galleryObjects.back().AddComponent<SkinnedMesh>(&(meshes[1]), meshMaterial, &(skeletons[1]));
		}

		for (int i = 0; i < galleryObjects.size(); i++)
			galleryObjects[i].SetEnabled(i == selectedModel);
	}

	void Game::Terminate()
	{
		delete[] meshes;
		delete[] skeletons;
		for (Entity e : galleryObjects)
			scene.DestroyEntity(e);
		galleryObjects.clear();
		ExampleViewer::Terminate(scene);
	}

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
		skeletons[selectedModel].SetAnimationIndex(Math::Mod(
			skeletons[selectedModel].GetAnimationIndex() + (next ? 1 : -1),
			skeletons[selectedModel].GetAnimationCount()));
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

		modelRotationY += Input::MousePosDeltaX() * MOUSE_SENSITIVITY * MODEL_ROTATION_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		modelRotationY = glm::mix(modelRotationY, 0.0f, deltaTime * 7.0f);
		modelRotation = glm::quat(glm::vec3(0.0f, modelRotationY * deltaTime, 0.0f));
		if (rotationEnabled)
			modelRotation *= glm::quat(glm::vec3(0.0f, 1.5f * deltaTime, 0.0f));

		Transform& objectTransform = galleryObjects[selectedModel].GetComponent<Transform>();
		objectTransform.rotation = modelRotation * objectTransform.rotation;
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
			ImGui::EndMainMenuBar();
		}
		ExampleViewer::ImGuiCall();
	}
}