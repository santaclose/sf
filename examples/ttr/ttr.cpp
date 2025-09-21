#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>

#include <Game.h>
#include <Math.hpp>
#include <Random.h>
#include <Input.h>
#include <GameInitializationData.h>

#include <Components/Transform.h>
#include <Components/Mesh.h>
#include <Components/Camera.h>

#include <Renderer/Renderer.h>

#include <MeshProcessor.h>

#include "../errt.hpp"

#define SENSITIVITY 0.007

#define UNIQUE_COUNT 60
#define ERRT_COUNT 6000

#define PHI_MAX 3.14159265/2.0;
#define PHI_MIN -3.14159265/2.0;

#define CONTANTEA 825
#define CONTANTEB 465

#define LERP_TIME 0.05

#define INITIAL_FREQUENCY 0.00559973
#define INITIAL_STRENGTH 12.4
#define INITIAL_SPEED 0.1231

namespace sf
{
	std::string Game::ConfigFilePath = "examples/ttr/config.json";

	Scene scene;
	Entity e_camera;

	float animation1A, animation2A;
	float animation1B, animation2B;
	float animation1C, animation2C;
	float animation1D, animation2D;
	MeshData uniqueErrts[UNIQUE_COUNT];
	Entity* errts;

	float tFreq;
	float tStrength;
	float zFrequency;
	float xFrequency;
	float zWaveStrength;
	float xWaveStrength;

	float cameraRot;

	float tSpeed;
	float speed;
	float posY;
	float cameraRadius;

	void Game::Initialize(int argc, char** argv)
	{
		animation1A = 0.36, animation2A = 0.36;
		animation1B = 0.62, animation2B = 0.62;
		animation1C = 2.22, animation2C = 2.22;
		animation1D = 3.96, animation2D = 3.96;
		tFreq = INITIAL_FREQUENCY;
		tStrength = INITIAL_STRENGTH;
		zFrequency = INITIAL_FREQUENCY;
		xFrequency = INITIAL_FREQUENCY;
		zWaveStrength = INITIAL_STRENGTH;
		xWaveStrength = INITIAL_STRENGTH;
		cameraRot = 0.0;
		tSpeed = INITIAL_SPEED;
		speed = INITIAL_SPEED;
		posY = 50.0f;
		cameraRadius = 650.0f;

		e_camera = scene.CreateEntity();
		Camera& c_camera = e_camera.AddComponent<Camera>();
		c_camera.fieldOfView = 0.5f;
		c_camera.nearClippingPlane = 0.01f;
		c_camera.farClippingPlane = 1000.0f;
		Transform& t_camera = e_camera.AddComponent<Transform>();
		t_camera.position = { -15.0f, 15.0f, 5.0f };
		t_camera.rotation = glm::quat(glm::vec3(-0.1f, -0.4f, 0.0f));

		for (int i = 0; i < UNIQUE_COUNT; i++)
		{
			errt::seed = i;
			errt::GenerateModel(uniqueErrts[i]);
		}

		uint32_t whiteMaterial, blackMaterial;
		{
			static glm::vec3 color(1.0, 1.0, 1.0);
			static glm::vec3 colorb(0.0, 0.0, 0.0);
			Material materialTemplate("assets/shaders/default.vert", "assets/shaders/solidColor.frag");
			materialTemplate.uniforms["color"] = { (uint32_t)DataType::vec3f32, &color };
			whiteMaterial = Renderer::CreateMaterial(materialTemplate, uniqueErrts[0].vertexBufferLayout);
			materialTemplate.uniforms["color"] = { (uint32_t)DataType::vec3f32, &colorb };
			blackMaterial = Renderer::CreateMaterial(materialTemplate, uniqueErrts[0].vertexBufferLayout);
		}

		errts = new Entity[ERRT_COUNT];
		for (int i = 0; i < ERRT_COUNT; i++)
		{
			errts[i] = scene.CreateEntity();
			Transform& t_errt = errts[i].AddComponent<Transform>();
			Mesh& m_errt = errts[i].AddComponent<Mesh>(&uniqueErrts[Random::Int(UNIQUE_COUNT)], Random::Float() > 0.5f ? whiteMaterial : blackMaterial);

			glm::vec2 randCircle = Random::PointInCircle();
			t_errt.position.x = randCircle.x * 200.0f;
			t_errt.position.z = randCircle.y * 200.0f;

			float randomRotZ = Random::Float() * glm::pi<float>() * 2.0f;
			t_errt.rotation = glm::quat(glm::vec3(0.0f, randomRotZ, 0.0f));
		}
	}

	void Game::Terminate()
	{
		scene.DestroyEntity(e_camera);
		for (int i = 0; i < ERRT_COUNT; i++)
			scene.DestroyEntity(errts[i]);
		delete[] errts;
	}

	inline void Animate(Transform& transform, float time, float deltaTime)
	{
		animation1D = time * speed * 10;
		animation1B = time * speed * 3.21;
		animation2D = time * speed * 12.32;
		animation2B = time * speed * 2.23;

		float x = transform.position.z * zFrequency;
		float x2 = transform.position.x * xFrequency;
		transform.position.y =
			zWaveStrength * sin(CONTANTEA + x) * sin(x * animation1A + animation1B) * sin(x * animation1C + animation1B) *
			xWaveStrength * sin(CONTANTEB + x2) * sin(x2 * animation2A + animation2B) * sin(x2 * animation2C + animation2B);
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		cameraRot = time * 0.01;

		zFrequency = glm::mix(zFrequency, tFreq, deltaTime * LERP_TIME);
		xFrequency = glm::mix(xFrequency, tFreq, deltaTime * LERP_TIME);
		xWaveStrength = glm::mix(xWaveStrength, tStrength, deltaTime * LERP_TIME);
		zWaveStrength = glm::mix(zWaveStrength, tStrength, deltaTime * LERP_TIME);
		speed = glm::mix(speed, tSpeed, deltaTime * LERP_TIME);

		Transform& t_camera = e_camera.GetComponent<Transform>();
		t_camera.position = glm::vec3(cos(cameraRot) * cameraRadius, posY, sin(cameraRot) * cameraRadius);
		t_camera.LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

		for (int i = 0; i < ERRT_COUNT; i++)
			Animate(errts[i].GetComponent<Transform>(), time, deltaTime);
	}

	void Game::ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Animation"))
			{
				ImGui::DragFloat("Frequency", &tFreq, 0.00012);
				ImGui::DragFloat("Strength", &tStrength, 0.07);
				ImGui::DragFloat("Speed", &tSpeed, 0.00007);
				if (ImGui::Button("Reset"))
				{
					tFreq = INITIAL_FREQUENCY;
					tStrength = INITIAL_STRENGTH;
					tSpeed = INITIAL_SPEED;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
}