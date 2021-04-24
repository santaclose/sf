#include <GLFW/glfw3.h>
#include <iostream>

#include <Game.h>
#include <Math.hpp>
#include <Random.h>
#include <Input.h>
#include <MeshProcessor.h>

#include <Renderer/Material.h>
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

	Shader shader;
	Material material;

	Shader uvShader;
	Material uvMaterial;


	int selectedModel = 0;

	void Game::Initialize(int argc, char** argv)
	{
		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();
		Renderer::activeCameraEntity = cameraObject;

		gimbal.AddComponent<Transform>();

		cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();

		shader.CreateFromFiles("assets/shaders/defaultV.shader", "assets/shaders/vertexAoF.shader");
		material.CreateFromShader(&shader, true);

		uvShader.CreateFromFiles("assets/shaders//defaultV.shader", "assets/shaders/uvF.shader");
		uvMaterial.CreateFromShader(&uvShader);
		
		std::vector<std::string> meshFilePaths = { "examples/vertexAo/table.obj", "examples/vertexAo/ag.obj", "examples/vertexAo/shoe.obj", "examples/vertexAo/chestnut.obj"	};
		for (int i = 0; i < meshFilePaths.size(); i++)
		{
			std::string& filePath = meshFilePaths[i];
			meshObjects.push_back(scene.CreateEntity());

			meshObjects.back().AddComponent<Transform>();
			Mesh& objectMesh = meshObjects.back().AddComponent<Mesh>();

			int objid = ObjImporter::Load(filePath);
			ObjImporter::GetMesh(objid, objectMesh);
			MeshProcessor::BakeAoToVertices(objectMesh);
			objectMesh.vertexReloadPending = true;
			objectMesh.SetMaterial(&material);

			if (i != selectedModel)
				meshObjects[i].SetEnabled(false);
		}

		gimbal.GetComponent<Transform>().SetPosition(glm::vec3(0.0, 0.0, 0.0));
		gimbal.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		cameraObject.GetComponent<Transform>().SetPosition(glm::vec3(0.0, 0.0, cameraDistance));
		cameraObject.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (Input::KeyDown(Input::KeyCode::M))
			glPolygonMode(GL_FRONT, GL_POINT);
		else if (Input::KeyDown(Input::KeyCode::N))
			glPolygonMode(GL_FRONT, GL_LINE);
		else if (Input::KeyDown(Input::KeyCode::B))
			glPolygonMode(GL_FRONT, GL_FILL);
		else if (Input::KeyDown(Input::KeyCode::Space))
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
		cameraDistance -= SCROLL_SENSITIVITY * (Input::MouseScrollUp() ? 1.0f : 0.0f);
		cameraDistance += SCROLL_SENSITIVITY * (Input::MouseScrollDown() ? 1.0f : 0.0f);

		targetGimbalRotation.y -= Input::MousePosDeltaX() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x += Input::MousePosDeltaY() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

		gimbal.GetComponent<Transform>().SetRotation(glm::slerp(gimbal.GetComponent<Transform>().GetRotation(), glm::fquat(targetGimbalRotation), deltaTime * SPEED));
		cameraObject.GetComponent<Transform>().SetPosition(gimbal.GetComponent<Transform>().GetPosition() + gimbal.GetComponent<Transform>().Forward() * cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(gimbal.GetComponent<Transform>().GetPosition(), glm::vec3(0.0, 1.0, 0.0));

		if (rotationEnabled)
		{
			Transform& objectTransform = meshObjects[selectedModel].GetComponent<Transform>();
			objectTransform.SetRotation(objectTransform.GetRotation() * glm::fquat(glm::vec3(0.0f, 0.07f * deltaTime, 0.0f)));
		}
	}
	void Game::Terminate()
	{
	}
}