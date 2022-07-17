#include <GLFW/glfw3.h>

#include <Game.h>

#include <Defaults.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Renderer/Renderer.h>

#include <MeshData.h>
#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>

#include <MeshProcessor.h>
#include <Math.hpp>
#include <Random.h>

#include <Input.h>
#include <Importer/GltfImporter.h>

#include "errt.h"

#define SENSITIVITY 0.007

#define UNIQUE_COUNT 50
#define COUNT 4000
#define SPAWN_RANGE 700.0

namespace sf
{
	Scene scene;
	Entity e_ship, e_mainCamera, e_lookBackCamera;

	float shipSpeed = 5.0;
	
	MeshData shipMesh;
	Entity* things;


	MeshData* generatedMeshes;

	void Game::Initialize(int argc, char** argv)
	{
		uint32_t aoMaterial = Renderer::CreateMaterial(Material("assets/shaders/defaultV.glsl", "assets/shaders/vertexAoF.glsl", false));
		uint32_t colorsMaterial = Renderer::CreateMaterial(Material("examples/spaceship/randomColorsV.glsl", "examples/spaceship/randomColorsF.glsl", false));
		uint32_t uvMaterial = Renderer::CreateMaterial(Material("assets/shaders/defaultV.glsl", "assets/shaders/uvF.glsl", false));
		uint32_t noiseMaterial = Renderer::CreateMaterial(Material("examples/spaceship/noiseV.glsl", "examples/spaceship/noiseF.glsl", false));

		e_ship = scene.CreateEntity();

		int gltfid = GltfImporter::Load("examples/spaceship/ship.glb");
		GltfImporter::GenerateMeshData(gltfid, shipMesh);
		Mesh& m_ship = e_ship.AddComponent<Mesh>(&shipMesh);
		Renderer::SetMeshMaterial(m_ship, uvMaterial);
		Transform& t_ship = e_ship.AddComponent<Transform>();

		e_mainCamera = scene.CreateEntity();
		Renderer::activeCameraEntity = e_mainCamera;
		e_lookBackCamera = scene.CreateEntity();

		Camera& c_mainCamera = e_mainCamera.AddComponent<Camera>();
		Camera& c_lookBackCamera = e_lookBackCamera.AddComponent<Camera>();
		c_mainCamera.perspective = c_lookBackCamera.perspective = true;
		c_mainCamera.farClippingPlane = c_lookBackCamera.farClippingPlane = 1000.0f;
		c_mainCamera.nearClippingPlane = c_lookBackCamera.nearClippingPlane = 0.1f;
		c_mainCamera.fieldOfView = glm::radians(90.0f);
		c_lookBackCamera.fieldOfView = glm::radians(120.0f);


		Transform& t_mainCamera = e_mainCamera.AddComponent<Transform>();
		Transform& t_lookBackCamera = e_lookBackCamera.AddComponent<Transform>();

		t_mainCamera.position = t_ship.position - (t_ship.Forward() * 4.0) + (t_ship.Up());
		t_mainCamera.LookAt(t_ship.position, t_ship.Up());

		
		generatedMeshes = new MeshData[UNIQUE_COUNT];
		for (unsigned int i = 0; i < UNIQUE_COUNT; i++)
		{
			errt::seed = i;
			MeshProcessor::GenerateMeshWithFunction(generatedMeshes[i], errt::GenerateModel);
			generatedMeshes[i].ChangeVertexLayout(Defaults::defaultVertexLayout);
			MeshProcessor::BakeAoToVertices(generatedMeshes[i]);
		}

		things = new Entity[COUNT];
		for (unsigned int i = 0; i < COUNT; i++)
		{
			things[i] = scene.CreateEntity();
			Mesh& m_thing = things[i].AddComponent<Mesh>(&(generatedMeshes[Random::Int(UNIQUE_COUNT)]));
			switch (Random::Int(3))
			{
			case 0:
				Renderer::SetMeshMaterial(m_thing, colorsMaterial);
				break;
			case 1:
				Renderer::SetMeshMaterial(m_thing, noiseMaterial);
				break;
			case 2:
				Renderer::SetMeshMaterial(m_thing, aoMaterial);
				break;
			}

			Transform& t_thing = things[i].AddComponent<Transform>();

			t_thing.scale = Random::Float() * (COUNT - i) / 100.0 + 1.0;
			t_thing.position = glm::vec3((Random::Float() - 0.5) * SPAWN_RANGE, (Random::Float() - 0.5) * SPAWN_RANGE, (i * -2.0f));
			t_thing.rotation = glm::quat(glm::vec3(0.0, glm::radians(180.0), glm::radians(Random::Float() * 360.0)));
		}
	}

	void Game::Terminate()
	{
		delete[] things;
		delete[] generatedMeshes;
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (Input::KeyDown(Input::KeyCode::Space))
		{
			if (Renderer::activeCameraEntity.GetComponent<Camera>().id == e_mainCamera.GetComponent<Camera>().id)
				Renderer::activeCameraEntity = e_lookBackCamera;
			else
				Renderer::activeCameraEntity = e_mainCamera;
		}
		shipSpeed += Input::MouseScrollUp() ? 1.0f : 0.0f;
		shipSpeed -= Input::MouseScrollDown() ? 1.0f : 0.0f;


		Transform& t_ship = e_ship.GetComponent<Transform>();
		Transform& t_mainCamera = e_mainCamera.GetComponent<Transform>();
		Transform& t_lookBackCamera = e_lookBackCamera.GetComponent<Transform>();

		t_ship.rotation *= glm::quat(glm::vec3(Input::MousePosDeltaY() * SENSITIVITY, 0.0, -Input::MousePosDeltaX() * SENSITIVITY));
		t_ship.position += t_ship.Forward() * shipSpeed * deltaTime;

		t_mainCamera.position = glm::mix(t_mainCamera.position, t_ship.position - (t_ship.Forward() * 4.0) + (t_ship.Up()), (float)(deltaTime * 2.0));
		t_mainCamera.LookAt(t_ship.position, t_ship.Up());

		t_lookBackCamera.position = t_ship.position + t_ship.Forward() * 4.0;
		t_lookBackCamera.LookAt(t_ship.position, t_ship.Up());
	}

	void Game::ImGuiCall() {}
}