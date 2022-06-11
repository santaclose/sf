#include <iostream>

#include <Game.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Random.h>
#include <Skybox.h>
#include <Input.h>
#include <Defaults.h>

#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/VoxelBox.h>
#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>

#define MOVE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.06
#define SPEED 15.0f
#define MODEL_OFFSET 50.0

namespace sf
{
	Scene scene;
	Entity gimbal, cameraObject;
	std::vector<Entity> galleryObjects;

	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);

	float cameraDistance = 3.0;
	bool rotationEnabled = false;

	VoxelBoxData* monkevbd;
	VoxelBoxData* monkevbd2;
	VoxelBoxData* monkevbd3;

	int selectedModel = 0;

	void Game::Initialize(int argc, char** argv)
	{
		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();
		Renderer::activeCameraEntity = cameraObject;

		gimbal.AddComponent<Transform>();

		cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();

		int gltfid, objid;
		objid = ObjImporter::Load("assets/monke.obj");
		ObjImporter::GenerateMeshData(objid, Defaults::monkeMeshData);
		monkevbd = new VoxelBoxData(Defaults::monkeMeshData, 0.007f);
		monkevbd2 = new VoxelBoxData(Defaults::monkeMeshData, 0.02f);
		monkevbd3 = new VoxelBoxData(Defaults::monkeMeshData, 0.07);

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			VoxelBox& objectVoxelBox = galleryObjects.back().AddComponent<VoxelBox>(monkevbd);
			galleryObjects.back().SetEnabled(true);
		}

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			VoxelBox& objectVoxelBox = galleryObjects.back().AddComponent<VoxelBox>(monkevbd2);
			galleryObjects.back().SetEnabled(false);
		}

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			VoxelBox& objectVoxelBox = galleryObjects.back().AddComponent<VoxelBox>(monkevbd3);
			galleryObjects.back().SetEnabled(false);
		}

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			Mesh& objectMesh = galleryObjects.back().AddComponent<Mesh>(&Defaults::monkeMeshData);
			galleryObjects.back().SetEnabled(false);
		}

		gimbal.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 0.0);
		gimbal.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		cameraObject.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (Input::KeyDown(Input::KeyCode::Space))
			rotationEnabled = !rotationEnabled;
		else if (Input::KeyDown(Input::KeyCode::Right))
		{
			int prevModel = selectedModel;
			selectedModel++;
			selectedModel = Math::Mod(selectedModel, (int)galleryObjects.size());
			galleryObjects[prevModel].SetEnabled(false);
			galleryObjects[selectedModel].SetEnabled(true);
		}
		else if (Input::KeyDown(Input::KeyCode::Left))
		{
			int prevModel = selectedModel;
			selectedModel--;
			selectedModel = Math::Mod(selectedModel, (int)galleryObjects.size());
			galleryObjects[prevModel].SetEnabled(false);
			galleryObjects[selectedModel].SetEnabled(true);
		}
		cameraDistance -= SCROLL_SENSITIVITY * (Input::MouseScrollUp() ? 1.0f : 0.0f);
		cameraDistance += SCROLL_SENSITIVITY * (Input::MouseScrollDown() ? 1.0f : 0.0f);

		targetGimbalRotation.y -= Input::MousePosDeltaX() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x += Input::MousePosDeltaY() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

		gimbal.GetComponent<Transform>().rotation = glm::slerp(gimbal.GetComponent<Transform>().rotation, glm::fquat(targetGimbalRotation), deltaTime * SPEED);
		cameraObject.GetComponent<Transform>().position = glm::vec3(gimbal.GetComponent<Transform>().position + gimbal.GetComponent<Transform>().Forward() * cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(gimbal.GetComponent<Transform>().position, glm::vec3(0.0, 1.0, 0.0));

		if (rotationEnabled)
		{
			Transform& objectTransform = galleryObjects[selectedModel].GetComponent<Transform>();
			objectTransform.rotation = objectTransform.rotation * glm::fquat(glm::vec3(0.0f, 0.07f * deltaTime, 0.0f));
		}
	}
	void Game::Terminate()
	{
	}
}