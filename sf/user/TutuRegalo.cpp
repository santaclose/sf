#include "TutuRegalo.h"
#include <GLFW/glfw3.h>
#include "../Texture.h"
#include "../Material.h"
#include "../Model.h"
#include "../ModelReference.h"
#include "../Math.h"
#include "errt.h"
#include <iostream>

#define SENSITIVITY 0.007

#define UNIQUE_COUNT 60
#define COPY_COUNT 20

#define PHI_MAX 3.14159265/2.0;
#define PHI_MIN -3.14159265/2.0;

namespace User
{
	Shader pbrShader;
	Shader noiseShader;
	Shader randomColorShader;
	Shader colorShader;

	Material noiseMaterial;
	Material colorMaterial;
	Material whiteMaterial;
	Material blackMaterial;

	Camera theCamera = Camera(1.7777777777777, 10, 0.1, 10000.0);

	/*Model ship;
	Model monkey;
	ModelReference* monkeys;*/

	Model errt[UNIQUE_COUNT];
	ModelReference* errts;

	float zFrequency = 0.1;
	float waveStrength = 25.0;

	float cameraRadius = UNIQUE_COUNT / 2.0 + 5.0;

	glm::fquat cameraRot(1.0, 0.0, 0.0, 0.0);
	float speed = 5.0;

	void Game::Initialize()
	{
		theCamera.SetPosition(-15, 10, 5);
		theCamera.SetRotation(-0.1, -0.4, 0);
		//theCamera.SetPosition(-20, 20, 20);
		//theCamera.SetRotation(10, 10, 0);

		pbrShader.CreateFromFiles("res/shaders/pbrV.shader", "res/shaders/gpbrF.shader");
		noiseShader.CreateFromFiles("user/noiseV.shader", "user/noiseF.shader");
		colorShader.CreateFromFiles("user/noiseV.shader", "user/solidColorF.shader");
		randomColorShader.CreateFromFiles("user/randomColorsV.shader", "user/randomColorsF.shader");

		noiseMaterial.CreateFromShader(&noiseShader);
		colorMaterial.CreateFromShader(&randomColorShader);

		blackMaterial.CreateFromShader(&colorShader);
		whiteMaterial.CreateFromShader(&colorShader);

		static glm::vec4 theColor(1.0, 1.0, 1.0, 1.0);
		static glm::vec4 theColorb(0.0, 0.0, 0.0, 1.0);
		blackMaterial.SetUniform("theColor", &theColorb, Material::UniformType::_Color);
		whiteMaterial.SetUniform("theColor", &theColor, Material::UniformType::_Color);


		errts = new ModelReference[COPY_COUNT * UNIQUE_COUNT];

		for (int i = 0; i < UNIQUE_COUNT; i++)
		{
			User::Models::seed = i;
			errt[i].CreateFromCode(User::Models::GenerateModel, true);
			errt[i].SetMaterial(rand() % 2 == 1 ? &blackMaterial : &whiteMaterial);
			errt[i].SetPosition(glm::vec3((Math::Random() - 0.5) * 200.0, 0.0, (Math::Random() - 0.5) * 200.0));

			for (int j = 0; j < COPY_COUNT; j++)
			{
				errts[j + i * COPY_COUNT].CreateFomModel(errt[i]);
				errts[j + i * COPY_COUNT].SetScale(Math::Random() + 0.1);
				errts[j + i * COPY_COUNT].SetPosition(glm::vec3((Math::Random() - 0.5) * 200.0, 0.0, (Math::Random() - 0.5) * 200.0));
			}
		}
	}

	inline void Animate(Entity& entity, float time, float deltaTime)
	{
		entity.SetPosition(glm::vec3(entity.GetPosition().x, waveStrength * sin(entity.GetPosition().z * zFrequency + time * speed) * deltaTime, entity.GetPosition().z));
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		glm::vec3 cameraDirection(0.0, 0.0, -1.0);
		cameraDirection = cameraRot * cameraDirection;

		glm::vec3 target(0.0, 0.0, -UNIQUE_COUNT / 2.0);
		theCamera.SetPosition(target + glm::normalize(cameraDirection) * cameraRadius);
		theCamera.LookAt(target, glm::vec3(0.0,1.0,0.0));

		for (int i = 0; i < UNIQUE_COUNT; i++)
		{
			Animate(errt[i], time, deltaTime);
			//errt[i].SetPosition(errt[i].GetPosition() + glm::vec3(0.0, (float)sin(errt[i].GetPosition().z*5.0 + time),0.0));

			for (int j = 0; j < COPY_COUNT; j++)
			{
				Animate(errts[j + i * COPY_COUNT], time, deltaTime);
				//errts[j + i * COPY_COUNT].SetPosition(errt[j + i * COPY_COUNT].GetPosition() + glm::vec3(0.0, (float)sin(errt[j + i * COPY_COUNT].GetPosition().z*5.0 + time),0.0));
			}
		}
	}

	void Game::OnKey(int key, int action)
	{
		if (action == GLFW_PRESS)
		{
			switch (key)
			{
			case GLFW_KEY_M:
				glPolygonMode(GL_FRONT, GL_POINT);
				break;
			case GLFW_KEY_N:
				glPolygonMode(GL_FRONT, GL_LINE);
				break;
			case GLFW_KEY_B:
				glPolygonMode(GL_FRONT, GL_FILL);
				break;
			case GLFW_KEY_UP:
				zFrequency += 0.1;
				break;
			case GLFW_KEY_DOWN:
				zFrequency -= 0.1;
				break;
			case GLFW_KEY_RIGHT:
				waveStrength += 0.1;
				break;
			case GLFW_KEY_LEFT:
				waveStrength -= 0.1;
				break;
			}
		}
	}

	void Game::OnScroll(double xoffset, double yoffset)
	{
		cameraRadius += yoffset*2.0;
	}

	void Game::OnMouseMove(double xpos, double ypos, const glm::vec2& mousePosDelta)
	{
		cameraRot.y += mousePosDelta.x * 0.007;
		cameraRot.x += mousePosDelta.y * 0.007;
	}
}