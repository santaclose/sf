#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>

#include <Game.h>
#include <Math.hpp>
#include <Random.h>
#include <Input.h>
#include <MeshProcessor.h>

#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/Camera.h>
#include <Components/Mesh.h>
#include <Components/Transform.h>

#define MOVE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.06
#define SPEED 15.0f
#define MODEL_OFFSET 50.0

namespace sf
{
	Scene scene;
	Entity gimbal, cameraObject;
	std::vector<Entity> meshObjects;

	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);

	float cameraDistance = 3.0;
	bool rotationEnabled = false;

	MeshData* sampleMeshes;

	int selectedModel = 0;

	void Game::Initialize(int argc, char** argv)
	{
		std::ifstream f("examples/vertexAo/sponza.obj");
		if (!f.good()) // download file if not there
			system("curl https://raw.githubusercontent.com/jimmiebergmann/Sponza/master/sponza.obj --output examples/vertexAo/sponza.obj");

		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();
		Renderer::activeCameraEntity = cameraObject;

		gimbal.AddComponent<Transform>();

		cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();

		uint32_t aoMaterial = Renderer::CreateMaterial(Material("assets/shaders/defaultV.shader", "assets/shaders/vertexAoF.shader", true));
		
		std::vector<std::string> meshFilePaths = { "examples/vertexAo/sponza.obj", "assets/meshes/monke.obj"};
		sampleMeshes = new MeshData[meshFilePaths.size()];
		for (int i = 0; i < meshFilePaths.size(); i++)
		{
			std::string& filePath = meshFilePaths[i];
			meshObjects.push_back(scene.CreateEntity());

			int objid = ObjImporter::Load(filePath);
			ObjImporter::GenerateMeshData(objid, sampleMeshes[i]);
			MeshProcessor::BakeAoToVertices(sampleMeshes[i]);

			meshObjects.back().AddComponent<Transform>();
			Mesh& objectMesh = meshObjects.back().AddComponent<Mesh>(&(sampleMeshes[i]));
			Renderer::SetMeshMaterial(objectMesh, aoMaterial);

			if (i != selectedModel)
				meshObjects[i].SetEnabled(false);
		}

		gimbal.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 0.0);
		gimbal.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		cameraObject.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		cameraObject.GetComponent<Camera>().farClippingPlane = 5000.0f;
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (Input::KeyDown(Input::KeyCode::Space))
			rotationEnabled = !rotationEnabled;
		else if (Input::KeyDown(Input::KeyCode::Right))
		{
			int prevModel = selectedModel;
			selectedModel++;
			selectedModel = Math::Mod(selectedModel, (int)meshObjects.size());
			meshObjects[prevModel].SetEnabled(false);
			meshObjects[selectedModel].SetEnabled(true);
		}
		else if (Input::KeyDown(Input::KeyCode::Left))
		{
			int prevModel = selectedModel;
			selectedModel--;
			selectedModel = Math::Mod(selectedModel, (int)meshObjects.size());
			meshObjects[prevModel].SetEnabled(false);
			meshObjects[selectedModel].SetEnabled(true);
		}
		if (Input::Key(Input::KeyCode::E))
		{
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Up();
		}
		if (Input::Key(Input::KeyCode::Q))
		{
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Up();
		}
		if (Input::Key(Input::KeyCode::W))
		{
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Forward();
		}
		if (Input::Key(Input::KeyCode::S))
		{
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Forward();
		}
		if (Input::Key(Input::KeyCode::D))
		{
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Right();
		}
		if (Input::Key(Input::KeyCode::A))
		{
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Right();
		}

		cameraDistance -= SCROLL_SENSITIVITY * (Input::MouseScrollUp() ? 1.0f : 0.0f);
		cameraDistance += SCROLL_SENSITIVITY * (Input::MouseScrollDown() ? 1.0f : 0.0f);

		targetGimbalRotation.y -= Input::MousePosDeltaX() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x += Input::MousePosDeltaY() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

		gimbal.GetComponent<Transform>().rotation = glm::slerp(gimbal.GetComponent<Transform>().rotation, glm::quat(targetGimbalRotation), deltaTime * SPEED);
		cameraObject.GetComponent<Transform>().position = gimbal.GetComponent<Transform>().position + gimbal.GetComponent<Transform>().Forward() * cameraDistance;
		cameraObject.GetComponent<Transform>().LookAt(gimbal.GetComponent<Transform>().position, glm::vec3(0.0, 1.0, 0.0));

		if (rotationEnabled)
		{
			Transform& objectTransform = meshObjects[selectedModel].GetComponent<Transform>();
			objectTransform.rotation = objectTransform.rotation * glm::quat(glm::vec3(0.0f, 0.07f * deltaTime, 0.0f));
		}
	}

	void Game::Terminate()
	{
		delete[] sampleMeshes;
	}
}