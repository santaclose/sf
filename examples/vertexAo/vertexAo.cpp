#include <GLFW/glfw3.h>
#include <iostream>

#include <Game.h>
#include <Material.h>
#include <Mesh.h>
#include <MeshReference.h>
#include <Math.hpp>
#include <Random.h>
#include <Input.h>
#include <Camera.h>
#include <MeshProcessor.h>
#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <VoxelModel.h>

#define MOVE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.06
#define SPEED 15.0f
#define MODEL_OFFSET 50.0

namespace sf
{
	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);
	Camera* camera;
	Entity gimbal;
	float cameraDistance = 3.0;
	bool rotationEnabled = false;

	Shader shader;
	Material material;

	Shader uvShader;
	Material uvMaterial;

	std::vector<Mesh*> models;
	int selectedModel = 0;

	void Game::Initialize(int argc, char** argv)
	{
		CameraSpecs cs;
		cs.farClippingPlane = 100.0f;
		cs.nearClippingPlane = 0.01f;
		cs.fieldOfView = glm::radians(75.0f);
		camera = new Camera(cs);

		shader.CreateFromFiles("assets/shaders/defaultV.shader", "assets/shaders/vertexAoF.shader");
		material.CreateFromShader(&shader, true);

		uvShader.CreateFromFiles("assets/shaders/defaultV.shader", "assets/shaders/uvF.shader");
		uvMaterial.CreateFromShader(&uvShader);

		int gltfid, objid;

		models.emplace_back();
		models.back() = new Mesh();
		objid = ObjImporter::Load("examples/vertexAo/table.obj");
		models.back()->CreateFromObj(objid);
		MeshProcessor::BakeAoToVertices(*models.back());
		models.back()->ReloadVertexData();
		models.back()->SetMaterial(&material, 0);
		models.emplace_back();
		models.back() = new Mesh();
		objid = ObjImporter::Load("examples/vertexAo/ag.obj");
		models.back()->CreateFromObj(objid);
		MeshProcessor::BakeAoToVertices(*models.back());
		models.back()->ReloadVertexData();
		models.back()->SetMaterial(&material, 0);
		models.emplace_back();
		models.back() = new Mesh();
		objid = ObjImporter::Load("examples/vertexAo/shoe.obj");
		models.back()->CreateFromObj(objid);
		MeshProcessor::BakeAoToVertices(*models.back());
		models.back()->ReloadVertexData();
		models.back()->SetMaterial(&material, 0);
		models.emplace_back();
		models.back() = new Mesh();
		objid = ObjImporter::Load("examples/vertexAo/seashell.obj");
		models.back()->CreateFromObj(objid);
		MeshProcessor::BakeAoToVertices(*models.back());
		models.back()->ReloadVertexData();
		models.back()->SetMaterial(&material, 0);
		models.emplace_back();
		models.back() = new Mesh();
		objid = ObjImporter::Load("examples/vertexAo/chestnut.obj");
		models.back()->CreateFromObj(objid);
		MeshProcessor::BakeAoToVertices(*models.back());
		models.back()->ReloadVertexData();
		models.back()->SetMaterial(&material, 0);
		models.back()->SetMaterial(&material, 1);

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

		targetGimbalRotation.y -= Input::MousePosDeltaX() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x += Input::MousePosDeltaY() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

		gimbal.SetRotation(glm::slerp(gimbal.GetRotation(), glm::fquat(targetGimbalRotation), deltaTime * SPEED));
		camera->SetPosition(gimbal.GetPosition() + gimbal.Forward() * cameraDistance);
		camera->LookAt(gimbal.GetPosition(), glm::vec3(0.0, 1.0, 0.0));

		if (rotationEnabled)
			models[selectedModel]->SetRotation(models[selectedModel]->GetRotation() * glm::fquat(glm::vec3(0.0f, 0.07f * deltaTime, 0.0f)));
	}
	void Game::Terminate()
	{
		for (Mesh* m : models)
			delete m;
	}
}