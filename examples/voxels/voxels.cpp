#include <imgui.h>
#include <iostream>

#include <Game.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Random.h>
#include <Input.h>
#include <Defaults.h>
#include <GameInitializationData.h>

#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/VoxelBox.h>
#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>

#include "../Viewer.hpp"

#define MODEL_ROTATION_SENSITIVITY 10.0
#define MODEL_OFFSET 50.0

namespace sf
{
	std::string Game::ConfigFilePath = "examples/voxels/config.json";

	Scene scene;
	std::vector<Entity> galleryObjects;

	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);

	glm::quat modelRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	float modelRotationY = 0.0f;

	float cameraDistance = 3.0;
	bool rotationEnabled = false;

	VoxelBoxData* monkevbd;
	VoxelBoxData* monkevbd2;
	VoxelBoxData* monkevbd3;

	int selectedModel = 0;

	void Game::Initialize(int argc, char** argv)
	{
		ExampleViewer::Initialize(scene);

		monkevbd = new VoxelBoxData(Defaults::monkeMeshData, 0.007f);
		monkevbd2 = new VoxelBoxData(Defaults::monkeMeshData, 0.02f);
		monkevbd3 = new VoxelBoxData(Defaults::monkeMeshData, 0.07);

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			VoxelBox& objectVoxelBox = galleryObjects.back().AddComponent<VoxelBox>(monkevbd);
		}

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			VoxelBox& objectVoxelBox = galleryObjects.back().AddComponent<VoxelBox>(monkevbd2);
		}

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			VoxelBox& objectVoxelBox = galleryObjects.back().AddComponent<VoxelBox>(monkevbd3);
		}

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			Mesh& objectMesh = galleryObjects.back().AddComponent<Mesh>(&Defaults::monkeMeshData);
		}

		for (int i = 0; i < galleryObjects.size(); i++)
			galleryObjects[i].SetEnabled(i == selectedModel);
	}

	void GalleryChange(bool next)
	{
		int prevModel = selectedModel;
		selectedModel += next ? 1 : -1;
		selectedModel = Math::Mod(selectedModel, (int)galleryObjects.size());
		galleryObjects[prevModel].SetEnabled(false);
		galleryObjects[selectedModel].SetEnabled(true);
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
			ImGui::EndMainMenuBar();
		}
		ExampleViewer::ImGuiCall();
	}

	void Game::Terminate()
	{
		delete monkevbd;
		delete monkevbd2;
		delete monkevbd3;
	}
}