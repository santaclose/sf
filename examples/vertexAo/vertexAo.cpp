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

#define MOUSE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.12
#define MODEL_ROTATION_SENSITIVITY 10.0
#define PAN_SENSITIVITY 0.1f
#define GIMBAL_MOVEMENT_SPEED 7.5

#define GIMBAL_ROTATION_SPEED 15.0f
#define MODEL_OFFSET 50.0
#define MIN_CAMERA_DISTANCE 0.5f

namespace sf
{
	std::string Game::ConfigFilePath = "examples/vertexAo/config.json";

	Scene scene;
	Entity gimbal, cameraObject;
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

		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();

		gimbal.AddComponent<Transform>();

		cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();

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

		gimbal.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 0.0);
		gimbal.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		cameraObject.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		cameraObject.GetComponent<Camera>().farClippingPlane = 5000.0f;
	}

	void UpdateCamera(float deltaTime)
	{
		if (Input::Key(Input::KeyCode::F))
			gimbal.GetComponent<Transform>().position = glm::vec3(0.0f);
		if (Input::Key(Input::KeyCode::E))
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Up() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::Q))
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Up() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::W))
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Forward() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::S))
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Forward() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::D))
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Right() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::A))
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Right() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;

		cameraDistance -= glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollUp() ? SCROLL_SENSITIVITY : 0.0f);
		cameraDistance += glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollDown() ? SCROLL_SENSITIVITY : 0.0f);
		cameraDistance = glm::max(MIN_CAMERA_DISTANCE, cameraDistance);

		if (Input::MouseButton(2))
		{
			if (Input::Key(Input::KeyCode::LeftShift))
				gimbal.GetComponent<Transform>().position +=
				-cameraObject.GetComponent<Transform>().Right() * GIMBAL_MOVEMENT_SPEED * MOUSE_SENSITIVITY * PAN_SENSITIVITY * Input::MousePosDeltaX() * glm::sqrt(cameraDistance) +
				cameraObject.GetComponent<Transform>().Up() * GIMBAL_MOVEMENT_SPEED * MOUSE_SENSITIVITY * PAN_SENSITIVITY * Input::MousePosDeltaY() * glm::sqrt(cameraDistance);
			else
			{
				targetGimbalRotation.y -= Input::MousePosDeltaX() * MOUSE_SENSITIVITY;
				targetGimbalRotation.x += Input::MousePosDeltaY() * MOUSE_SENSITIVITY;
			}
		}

		targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

		gimbal.GetComponent<Transform>().rotation = glm::slerp(gimbal.GetComponent<Transform>().rotation, glm::quat(targetGimbalRotation), deltaTime * GIMBAL_ROTATION_SPEED);
		cameraObject.GetComponent<Transform>().position = glm::vec3(gimbal.GetComponent<Transform>().position + gimbal.GetComponent<Transform>().Forward() * cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(gimbal.GetComponent<Transform>().position, glm::vec3(0.0, 1.0, 0.0));
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
		if (Input::KeyDown(Input::KeyCode::Space))
			rotationEnabled = !rotationEnabled;
		else if (Input::KeyDown(Input::KeyCode::Right))
			GalleryChange(true);
		else if (Input::KeyDown(Input::KeyCode::Left))
			GalleryChange(false);

		UpdateCamera(deltaTime);

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
			if (ImGui::BeginMenu("Camera"))
			{
				if (ImGui::MenuItem("Center", "F"))
					gimbal.GetComponent<Transform>().position = glm::vec3(0.0f);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void Game::Terminate()
	{
		delete[] sampleMeshes;
	}
}