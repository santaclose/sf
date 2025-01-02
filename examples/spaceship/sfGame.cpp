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
#include <Components/SphereCollider.h>

#include <MeshProcessor.h>
#include <Math.hpp>
#include <Random.h>

#include <Input.h>
#include <Importer/GltfImporter.h>

#include "errt.h"

#define SENSITIVITY 0.007

#define BULLET_COUNT 50
// #define BULLET_COUNT 1
#define BULLET_SPEED 10.0f
#define BULLET_TIME 10.0f

// #define THINGS_UNIQUE_COUNT 10
// #define THINGS_COUNT 400
#define THINGS_UNIQUE_COUNT 1
#define THINGS_COUNT 50
#define THINGS_SPAWN_RANGE 700.0

namespace sf
{
	std::string Game::ConfigFilePath = "examples/spaceship/config.json";

	Scene scene;
	Entity e_ship, e_mainCamera, e_lookBackCamera;

	int shipSpeed = 0;
	
	MeshData shipMesh;
	Entity* things;

	Entity* bullets;
	uint32_t currentBullet = 0;
	std::vector<float> bulletTimers;
	std::vector<float> bulletSpeeds;

	MeshData* generatedMeshes;

	uint32_t aoMaterial;
	uint32_t colorsMaterial;
	uint32_t uvMaterial;
	uint32_t noiseMaterial;

	void Game::Initialize(int argc, char** argv)
	{
		aoMaterial = Renderer::CreateMaterial(Material("assets/shaders/default.vert", "assets/shaders/vertexAo.frag", false));
		colorsMaterial = Renderer::CreateMaterial(Material("examples/spaceship/randomColors.vert", "examples/spaceship/randomColors.frag", false));
		uvMaterial = Renderer::CreateMaterial(Material("assets/shaders/default.vert", "assets/shaders/uv.frag", false));
		noiseMaterial = Renderer::CreateMaterial(Material("examples/spaceship/noise.vert", "examples/spaceship/noise.frag", false));

		e_ship = scene.CreateEntity();

		int gltfid = GltfImporter::Load("examples/spaceship/ship.glb");
		GltfImporter::GenerateMeshData(gltfid, shipMesh);
		Mesh& m_ship = e_ship.AddComponent<Mesh>(&shipMesh);
		Renderer::SetMeshMaterial(m_ship, uvMaterial);
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
		
		generatedMeshes = new MeshData[THINGS_UNIQUE_COUNT];
		for (unsigned int i = 0; i < THINGS_UNIQUE_COUNT; i++)
		{
			errt::seed = i;
			MeshProcessor::GenerateMeshWithFunction(generatedMeshes[i], errt::GenerateModel);
			generatedMeshes[i].ChangeVertexLayout(Defaults::defaultVertexLayout);
			// MeshProcessor::BakeAoToVertices(generatedMeshes[i]);
		}

		bullets = new Entity[BULLET_COUNT];
		for (unsigned int i = 0; i < BULLET_COUNT; i++)
		{
			bullets[i] = scene.CreateEntity();
			Mesh& m_thing = bullets[i].AddComponent<Mesh>(&Defaults::sphereMeshData);
			Renderer::SetMeshMaterial(m_thing, noiseMaterial);

			Transform& t_bullet = bullets[i].AddComponent<Transform>();
			t_bullet.scale = 0.15f;
			bullets[i].SetEnabled(false);

			SphereCollider& sc_bullet = bullets[i].AddComponent<SphereCollider>();
			sc_bullet.radius = 0.15f * 0.5f;
		}
		bulletTimers = std::vector(BULLET_COUNT, -1.0f);
		bulletSpeeds = std::vector(BULLET_COUNT, 0.0f);

		things = new Entity[THINGS_COUNT];
		for (unsigned int i = 0; i < THINGS_COUNT; i++)
		{
			things[i] = scene.CreateEntity();
			// Mesh& m_thing = things[i].AddComponent<Mesh>(&(generatedMeshes[Random::Int(THINGS_UNIQUE_COUNT)]));
			Mesh& m_thing = things[i].AddComponent<Mesh>(&Defaults::sphereMeshData);
			switch (Random::Int(3))
			{
			case 0:
				// Renderer::SetMeshMaterial(m_thing, colorsMaterial);
				// break;
			case 1:
				// Renderer::SetMeshMaterial(m_thing, noiseMaterial);
				// break;
			case 2:
				Renderer::SetMeshMaterial(m_thing, aoMaterial);
				break;
			}

			float scaleValue = Random::Float();
			Transform& t_thing = things[i].AddComponent<Transform>();
			t_thing.scale = scaleValue * (THINGS_COUNT - i) / 10.0 + 1.0;
			t_thing.position = glm::vec3((Random::Float() - 0.5) * THINGS_SPAWN_RANGE, (Random::Float() - 0.5) * THINGS_SPAWN_RANGE, (i * -2.0f));
			// t_thing.position = glm::vec3(0.0f, 0.0f, (i * -2.0f));
			t_thing.rotation = glm::quat(glm::vec3(0.0, glm::radians(180.0), glm::radians(Random::Float() * 360.0)));

			SphereCollider& sc_thing = things[i].AddComponent<SphereCollider>();
			sc_thing.radius = scaleValue;// * 0.5f;
		}
	}

	void Game::Terminate()
	{
		delete[] things;
		delete[] generatedMeshes;
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		Transform& t_ship = e_ship.GetComponent<Transform>();
		Transform& t_mainCamera = e_mainCamera.GetComponent<Transform>();
		Transform& t_lookBackCamera = e_lookBackCamera.GetComponent<Transform>();

		for (uint32_t i = 0; i < BULLET_COUNT; i++)
		{
			if (bulletTimers[i] < 0.0f)
				continue;
			bulletTimers[i] -= deltaTime;
			if (bulletTimers[i] < 0.0f)
				bullets[i].SetEnabled(false);
			else
			{
				Transform& t_bullet = bullets[i].GetComponent<Transform>();
				t_bullet.position += t_bullet.Forward() * deltaTime * bulletSpeeds[i];
			}
		}
		if (Input::MouseButtonDown(0))
		{
			bullets[currentBullet].SetEnabled(true);
			Transform& t_bullet = bullets[currentBullet].GetComponent<Transform>();
			t_bullet.position = t_ship.position + t_ship.Forward();
			t_bullet.rotation = t_ship.rotation;
			bulletTimers[currentBullet] = BULLET_TIME;
			bulletSpeeds[currentBullet] = ((float)shipSpeed) + BULLET_SPEED;
			currentBullet = (currentBullet + 1) % BULLET_COUNT;
		}

		if (Input::KeyDown(Input::KeyCode::Space))
		{
			if (Renderer::GetActiveCameraEntity() == e_mainCamera)
				Renderer::SetActiveCameraEntity(e_lookBackCamera);
			else
				Renderer::SetActiveCameraEntity(e_mainCamera);
		}
		shipSpeed += Input::MouseScrollUp() ? 1 : 0;
		shipSpeed -= Input::MouseScrollDown() ? 1 : 0;

		t_ship.rotation *= glm::quat(glm::vec3(Input::MousePosDeltaY() * SENSITIVITY, 0.0, -Input::MousePosDeltaX() * SENSITIVITY));
		t_ship.position += t_ship.Forward() * ((float)shipSpeed) * deltaTime;

		t_mainCamera.position = glm::mix(t_mainCamera.position, t_ship.position - (t_ship.Forward() * 4.0) + (t_ship.Up()), (float)(deltaTime * 2.0));
		t_mainCamera.LookAt(t_ship.position, t_ship.Up());

		t_lookBackCamera.position = t_ship.position + t_ship.Forward() * 4.0;
		t_lookBackCamera.LookAt(t_ship.position, t_ship.Up());
	}

	void Game::OnCollision(Entity entity)
	{
		Mesh& m_thing = entity.GetComponent<Mesh>();
		// Mesh& m_thing = sf::Scene::activeScene->GetRegistry().get<Mesh>(entity);
		Renderer::SetMeshMaterial(m_thing, noiseMaterial);
	}

	void Game::ImGuiCall() {}
}