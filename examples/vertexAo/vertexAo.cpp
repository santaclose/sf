#include <GLFW/glfw3.h>
#include <iostream>
#include "../../src/Game.h"
#include "../../src/Material.h"
#include "../../src/Model.h"
#include "../../src/ModelReference.h"
#include "../../src/Math.h"
#include "../../src/Random.h"
#include "../../src/Input.h"
#include "../../src/Camera.h"
#include "../../src/GltfController.h"
#include "../../src/ObjController.h"
#include "../../src/ModelProcessor.h"

#define MOVE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.06
#define SPEED 15.0f
#define MODEL_OFFSET 50.0

namespace User
{
	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);
	Camera* camera;
	Entity gimbal;
	float cameraDistance = 3.0;
	bool rotationEnabled = false;

	Shader shader;
	Material material;

	std::vector<Model*> models;
	int selectedModel = 0;

	void Game::Initialize(int argc, char** argv)
	{
		int rayCount;
		bool onlyUpwards;
		if (argc > 1)
			rayCount = std::stoi(argv[1]);
		else
		{
			rayCount = 15;
		}
		if (argc > 2)
			onlyUpwards = std::stoi(argv[2]);
		else
			onlyUpwards = false;

		CameraSpecs cs;
		cs.aspectRatio = 16.0f / 9.0f;
		cs.farClippingPlane = 100.0f;
		cs.nearClippingPlane = 0.01f;
		cs.fieldOfView = glm::radians(75.0f);
		camera = new Camera(cs);

		shader.CreateFromFiles("assets/shaders/defaultV.shader", "assets/shaders/vertexAoF.shader");
		material.CreateFromShader(&shader);

		int gltfid, objid;

		//models.emplace_back();
		//models.back() = new Model();
		//objid = ObjController::Load("examples/vertexAo/nemotree.obj");
		//models.back()->CreateFromObj(objid, 0);
		//ModelProcessor::BakeAo(*models.back(), 30, true);
		//models.back()->ReloadVertexData();
		//models.back()->SetMaterial(&material);

		//models.emplace_back();
		//models.back() = new Model();
		//objid = ObjController::Load("examples/vertexAo/ct.obj");
		//models.back()->CreateFromObj(objid, 0);
		//ModelProcessor::BakeAo(*models.back(), 30, true);
		//models.back()->ReloadVertexData();
		//models.back()->SetMaterial(&material);

		models.emplace_back();
		models.back() = new Model();
		objid = ObjController::Load("examples/vertexAo/seashell.obj");
		models.back()->CreateFromObj(objid, 0);
		ModelProcessor::BakeAo(*models.back(), 50, false, true, 0.001, 5.0f);
		//ModelProcessor::BakeAo(*models.back());
		models.back()->ReloadVertexData();
		models.back()->SetMaterial(&material);

		//models.emplace_back();
		//models.back() = new Model();
		//objid = ObjController::Load("examples/vertexAo/seashell.obj");
		//models.back()->CreateFromObj(objid, 0);
		//ModelProcessor::BakeAo(*models.back(), 30, true);
		//models.back()->ReloadVertexData();
		//models.back()->SetMaterial(&material);

		for (int i = 0; i < models.size(); i++)
		{
			std::cout << "setting model position " << i << std::endl;
			models[i]->SetPosition(glm::vec3(i * MODEL_OFFSET, 0.0, 0.0));
			if (i > 0)
				models[i]->SetScale(0.0);
		}

		std::cout << "setting gimbal position" << std::endl;
		gimbal.SetPosition(glm::vec3(0.0, 0.0, 0.0));
		gimbal.LookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		std::cout << "setting camera position" << std::endl;
		camera->SetPosition(glm::vec3(0.0, 0.0, cameraDistance));
		camera->LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
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
			glm::fquat theRot = models[selectedModel]->GetRotation();
			models[selectedModel]->SetScale(0.0);
			selectedModel++;
			selectedModel %= models.size();
			models[selectedModel]->SetScale(1.0);
			models[selectedModel]->SetRotation(theRot);
			gimbal.SetPosition(glm::vec3(selectedModel * MODEL_OFFSET, 0.0, 0.0));
		}
		else if (Input::KeyDown(Input::KeyCode::Left))
		{
			glm::fquat theRot = models[selectedModel]->GetRotation();
			models[selectedModel]->SetScale(0.0);
			selectedModel--;
			selectedModel = (selectedModel + models.size()) % models.size();
			models[selectedModel]->SetScale(1.0);
			models[selectedModel]->SetRotation(theRot);
			gimbal.SetPosition(glm::vec3(selectedModel * MODEL_OFFSET, 0.0, 0.0));
		}
		cameraDistance -= SCROLL_SENSITIVITY * (Input::MouseScrollUp() ? 1.0f : 0.0f);
		cameraDistance += SCROLL_SENSITIVITY * (Input::MouseScrollDown() ? 1.0f : 0.0f);

		targetGimbalRotation.y -= Input::MousePosDeltaX() * MOVE_SENSITIVITY;
		targetGimbalRotation.x += Input::MousePosDeltaY() * MOVE_SENSITIVITY;

		gimbal.SetRotation(glm::slerp(gimbal.GetRotation(), glm::fquat(targetGimbalRotation), deltaTime * SPEED));
		camera->SetPosition(gimbal.GetPosition() + gimbal.Forward() * cameraDistance);
		camera->LookAt(gimbal.GetPosition(), glm::vec3(0.0, 1.0, 0.0));

		if (rotationEnabled)
			models[selectedModel]->SetRotation(models[selectedModel]->GetRotation() * glm::fquat(glm::vec3(0.0f, 0.07f * deltaTime, 0.0f)));
	}
	void Game::Terminate()
	{
		for (Model* m : models)
			delete m;
	}
}