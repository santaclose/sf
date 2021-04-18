#include <GLFW/glfw3.h>

#include <Game.h>
#include <Texture.h>
#include <Material.h>
#include <Mesh.h>
#include <MeshReference.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Random.h>
#include <Camera.h>
#include <Input.h>
#include <Importer/GltfImporter.h>

#include "errt.h"

#define SENSITIVITY 0.007

#define UNIQUE_COUNT 50
#define COUNT 4000
#define SPAWN_RANGE 700.0

namespace sf
{
	float shipSpeed = 5.0;
	glm::fquat targetShipRotation = glm::fquat(1.0, 0.0, 0.0, 0.0);
	Camera* theCamera;
	Camera* lookBackCamera;

	Shader aoShader;
	Shader uvShader;
	Shader colorShader;
	Shader noiseShader;

	Material colorsMaterial;
	Material noiseMaterial;
	Material uvMaterial;
	Material aoMaterial;

	Mesh ship;
	Mesh uniqueThings[UNIQUE_COUNT];
	MeshReference* things;

	void Game::Initialize(int argc, char** argv)
	{
		CameraSpecs specs;
		specs.perspective = true;
		specs.farClippingPlane = 1000.0f;
		specs.nearClippingPlane = 0.1f;
		specs.fieldOfView = glm::radians(90.0f);
		theCamera = new Camera(specs);
		specs.fieldOfView = glm::radians(120.0f);
		lookBackCamera = new Camera(specs);

		aoShader.CreateFromFiles("assets/shaders/defaultV.shader", "assets/shaders/vertexAoF.shader");
		colorShader.CreateFromFiles("examples/spaceship/randomColorsV.shader", "examples/spaceship/randomColorsF.shader");
		uvShader.CreateFromFiles("assets/shaders/defaultV.shader", "assets/shaders/uvF.shader");
		noiseShader.CreateFromFiles("examples/spaceship/noiseV.shader", "examples/spaceship/noiseF.shader");

		aoMaterial.CreateFromShader(&aoShader);
		colorsMaterial.CreateFromShader(&colorShader);
		uvMaterial.CreateFromShader(&uvShader);
		noiseMaterial.CreateFromShader(&noiseShader);

		int gltfid = GltfImporter::Load("examples/spaceship/ship.glb");
		ship.CreateFromGltf(gltfid);
		ship.SetMaterial(&uvMaterial, 0);
		targetShipRotation = ship.GetRotation();

		theCamera->SetPosition(ship.GetPosition() - (ship.Forward() * 4.0) + (ship.Up()));
		theCamera->LookAt(ship.GetPosition(), ship.Up());

		for (unsigned int i = 0; i < UNIQUE_COUNT; i++)
		{
			Models::seed = i;
			uniqueThings[i].CreateFromCode(Models::GenerateModel);

			switch (i % 3)
			{
			case 0:
				MeshProcessor::BakeAoToVertices(uniqueThings[i]);
				uniqueThings[i].ReloadVertexData();
				uniqueThings[i].SetMaterial(&aoMaterial, 0);
				break;
			case 1:
				uniqueThings[i].SetMaterial(&colorsMaterial, 0);
				break;
			case 2:
				uniqueThings[i].SetMaterial(&noiseMaterial, 0);
				break;
			}
		}

		things = new MeshReference[COUNT];
		for (int i = 0; i < COUNT + UNIQUE_COUNT; i++)
		{
			if (i < UNIQUE_COUNT)
			{
				uniqueThings[i].SetScale(Random::Float() * (COUNT + UNIQUE_COUNT - i) / 100.0 + 1.0);
				uniqueThings[i].SetPosition(glm::vec3((Random::Float() - 0.5) * SPAWN_RANGE, (Random::Float() - 0.5) * SPAWN_RANGE, (float)(i * -2)));
				uniqueThings[i].SetRotation(glm::fquat(glm::vec3(0.0, glm::radians(180.0), glm::radians(Random::Float() * 360.0))));
			}
			else
			{
				things[i - UNIQUE_COUNT].CreateFomMesh(uniqueThings[rand() % UNIQUE_COUNT]);
				things[i - UNIQUE_COUNT].SetScale(Random::Float() * (COUNT + UNIQUE_COUNT - i) / 100.0 + 1.0);
				things[i - UNIQUE_COUNT].SetPosition(glm::vec3((Random::Float() - 0.5) * SPAWN_RANGE, (Random::Float() - 0.5) * SPAWN_RANGE, (float)(i * -2)));
				things[i - UNIQUE_COUNT].SetRotation(glm::fquat(glm::vec3(0.0, glm::radians(180.0), glm::radians(Random::Float() * 360.0))));
			}
		}
		//std::cout << theCamera->GetPosition().x << ", " << theCamera->GetPosition().y << ", " << theCamera->GetPosition().z << std::endl;
		//std::cout << ship.GetPosition().x << ", " << ship.GetPosition().y << ", " << ship.GetPosition().z << std::endl;
	}

	void Game::Terminate()
	{
		delete theCamera;
		delete lookBackCamera;
		delete[] things;
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
		{
			if (Camera::boundCamera == theCamera)
				lookBackCamera->Bind();
			else
				theCamera->Bind();
		}
		shipSpeed += Input::MouseScrollUp() ? 1.0f : 0.0f;
		shipSpeed -= Input::MouseScrollDown() ? 1.0f : 0.0f;
		targetShipRotation *= glm::fquat(glm::vec3(Input::MousePosDeltaY() * SENSITIVITY, 0.0, -Input::MousePosDeltaX() * SENSITIVITY));

		ship.SetPosition(ship.GetPosition() + ship.Forward() * shipSpeed * deltaTime);
		ship.SetRotation(targetShipRotation);

		theCamera->SetPosition(glm::mix(theCamera->GetPosition(), ship.GetPosition() - (ship.Forward() * 4.0) + (ship.Up()), (float)(deltaTime * 2.0)));
		theCamera->LookAt(ship.GetPosition(), ship.Up());

		lookBackCamera->SetPosition(ship.GetPosition() + ship.Forward() * 4.0);
		lookBackCamera->LookAt(ship.GetPosition(), ship.Up());
	}
}