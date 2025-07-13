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
	std::string Game::ConfigFilePath = "examples/spaceship/config.json";

	Scene scene;
	Entity e_ship, e_mainCamera, e_lookBackCamera;

	float shipSpeed;
	
	MeshData shipMesh;
	Entity* things;

	MeshData* generatedMeshes;

	void Game::Initialize(int argc, char** argv)
	{
		shipSpeed = 5.0;

		uint32_t aoMaterial = Renderer::CreateMaterial(Material("assets/shaders/default.vert", "assets/shaders/vertexAo.frag"));
		uint32_t colorsMaterial = Renderer::CreateMaterial(Material("examples/spaceship/randomColors.vert", "examples/spaceship/randomColors.frag"));
		uint32_t uvMaterial = Renderer::CreateMaterial(Material("assets/shaders/default.vert", "assets/shaders/uv.frag"));
		uint32_t noiseMaterial = Renderer::CreateMaterial(Material("examples/spaceship/noise.vert", "examples/spaceship/noise.frag"));

		e_ship = scene.CreateEntity();

		int gltfid = GltfImporter::Load("examples/spaceship/ship.glb");
		GltfImporter::GenerateMeshData(gltfid, shipMesh);
		Mesh& m_ship = e_ship.AddComponent<Mesh>(&shipMesh, uvMaterial);
		Transform& t_ship = e_ship.AddComponent<Transform>();

		e_mainCamera = scene.CreateEntity();
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
		char cahedMeshesPath[] = "examples/spaceship/generatedXX.mesh\0";
		for (unsigned int i = 0; i < UNIQUE_COUNT; i++)
		{
			cahedMeshesPath[28] = '0' + (i / 10);
			cahedMeshesPath[29] = '0' + (i % 10);
			if (generatedMeshes[i].LoadFromFile(cahedMeshesPath))
				continue;
			errt::seed = i;
			MeshProcessor::GenerateMeshWithFunction(generatedMeshes[i], errt::GenerateModel);
			generatedMeshes[i].ChangeVertexLayout(Defaults::VertexLayout());
			MeshProcessor::BakeAoToVertices(generatedMeshes[i]);
			generatedMeshes[i].SaveToFile(cahedMeshesPath);
		}

		things = new Entity[COUNT];
		uint32_t materialsToChooseFrom[3] = { colorsMaterial, noiseMaterial, aoMaterial };
		for (unsigned int i = 0; i < COUNT; i++)
		{
			things[i] = scene.CreateEntity();
			Mesh& m_thing = things[i].AddComponent<Mesh>(&generatedMeshes[Random::Int(UNIQUE_COUNT)], materialsToChooseFrom[Random::Int(3)]);

			Transform& t_thing = things[i].AddComponent<Transform>();

			t_thing.scale = Random::Float() * (COUNT - i) / 100.0 + 1.0;
			t_thing.position = glm::vec3((Random::Float() - 0.5) * SPAWN_RANGE, (Random::Float() - 0.5) * SPAWN_RANGE, (i * -2.0f));
			t_thing.rotation = glm::quat(glm::vec3(0.0, glm::radians(180.0), glm::radians(Random::Float() * 360.0)));
		}
	}

	void Game::Terminate()
	{
		scene.DestroyEntity(e_ship);
		scene.DestroyEntity(e_mainCamera);
		scene.DestroyEntity(e_lookBackCamera);
		for (unsigned int i = 0; i < COUNT; i++)
			scene.DestroyEntity(things[i]);

		delete[] things;
		delete[] generatedMeshes;
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (Input::KeyDown(Input::KeyCode::Space))
		{
			if (Renderer::GetActiveCameraEntity() == e_mainCamera)
				Renderer::SetActiveCameraEntity(e_lookBackCamera);
			else
				Renderer::SetActiveCameraEntity(e_mainCamera);
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