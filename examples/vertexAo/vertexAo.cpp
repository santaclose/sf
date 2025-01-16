#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>
#include <fstream>

#include <Game.h>
#include <Math.hpp>
#include <Random.h>
#include <Input.h>
#include <MeshProcessor.h>
#include <GameInitializationData.h>
#include <FileUtils.h>

#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/Camera.h>
#include <Components/Mesh.h>
#include <Components/Transform.h>

#include "../Viewer.hpp"

#define MODEL_ROTATION_SENSITIVITY 10.0
#define MODEL_OFFSET 50.0

namespace sf
{
	std::string Game::ConfigFilePath = "examples/vertexAo/config.json";

	Scene scene;
	std::vector<Entity> galleryObjects;

	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);

	glm::quat modelRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	float modelRotationY = 0.0f;

	float cameraDistance = 3.0;
	bool rotationEnabled = false;

	MeshData* sampleMeshes;

	int selectedModel = 0;

	void Game::Initialize(int argc, char** argv)
	{
		FileUtils::DownloadFiles({ "https://casual-effects.com/g3d/data10/research/model/bunny/bunny.zip" }, "examples/vertexAo/");

		ExampleViewer::Initialize(scene);

		uint32_t aoMaterial = Renderer::CreateMaterial(Material("assets/shaders/default.vert", "assets/shaders/vertexAo.frag", true));

		std::vector<std::string> meshFilePaths = { "examples/vertexAo/bunny/bunny.obj", "assets/meshes/monke.obj" };
		sampleMeshes = new MeshData[meshFilePaths.size()];
		for (int i = 0; i < meshFilePaths.size(); i++)
		{
			std::string& filePath = meshFilePaths[i];
			galleryObjects.push_back(scene.CreateEntity());

			int objid = ObjImporter::Load(filePath);
			ObjImporter::GenerateMeshData(objid, sampleMeshes[i]);
			MeshProcessor::BakeAoToVertices(sampleMeshes[i]);

			Transform& e_t = galleryObjects.back().AddComponent<Transform>();
			if (i == 0)
				e_t.position += glm::vec3(0.3f, -0.6f, 0.0f);

			Mesh& objectMesh = galleryObjects.back().AddComponent<Mesh>(&(sampleMeshes[i]));
			Renderer::SetMeshMaterial(objectMesh, aoMaterial);

			if (i != selectedModel)
				galleryObjects[i].SetEnabled(false);
		}
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
		delete[] sampleMeshes;
	}
}