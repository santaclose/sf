#include "../../user/Game.h"
#include <GLFW/glfw3.h>
#include "../../src/Texture.h"
#include "../../src/Material.h"
#include "../../src/Model.h"
#include "../../src/ModelReference.h"
#include "../../src/Math.h"
#include "../../src/Camera.h"
#include "errt.h"
#include <iostream>

#define SENSITIVITY 0.007

#define UNIQUE_COUNT 60
#define COPY_COUNT 100

#define PHI_MAX 3.14159265/2.0;
#define PHI_MIN -3.14159265/2.0;

#define CONTANTEA 825
#define CONTANTEB 465

#define LERP_TIME 0.05

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

	//Camera theCamera = Camera(1.7777777777777, 10, 0.1, 10000.0);
	Camera theCamera = Camera(1.7777777777777, 5, 0.1, 10000.0);

	float animation1A = 0.36, animation2A = 0.36;
	float animation1B = 0.62, animation2B = 0.62;
	float animation1C = 2.22, animation2C = 2.22;
	float animation1D = 3.96, animation2D = 3.96;

	Model errt[UNIQUE_COUNT];
	ModelReference* errts;


	float tFreq = 0.2;
	float tStrength = 4.0;
	float zFrequency = 0.2;
	float xFrequency = 0.2;
	float zWaveStrength = 4.0;
	float xWaveStrength = 4.0;
	//float zWaveStrength = 10.0;
	//float xWaveStrength = 10.0;

	float cameraRot = 0.0;

	float tSpeed = 0.1;
	float speed = 0.1;
	//float speed = 0.2;
	float posY = 0.0f;
	float cameraRadius = 650.0f;

	int selectedVariable = 0;

	void Game::Initialize()
	{
		theCamera.SetPosition(-15, 15, 5);
		theCamera.SetRotation(-0.1, -0.4, 0);

		colorShader.CreateFromFiles("assets/shaders/pbrV.shader", "examples/ttr/solidColorF.shader");

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
			float randomAngle = Math::Random() * 3.14159265*2.0;
			float randomDistance = Math::Random() * 200.0;

			errt[i].SetPosition(glm::vec3(cos(randomAngle) * randomDistance, 0.0, sin(randomAngle) * randomDistance));

			for (int j = 0; j < COPY_COUNT; j++)
			{
				errts[j + i * COPY_COUNT].CreateFomModel(errt[i]);
				errts[j + i * COPY_COUNT].SetScale(Math::Random() + 0.1);

				float randomAngle = Math::Random() * 3.14159265*2.0;
				float randomDistance = Math::Random() * 200.0;
				errts[j + i * COPY_COUNT].SetPosition(glm::vec3(cos(randomAngle) * randomDistance, 0.0, sin(randomAngle) * randomDistance));
				float randomRotZ = Math::Random() * 3.14159265 * 2.0;
				errts[j + i * COPY_COUNT].SetRotation(glm::vec3(0.0, randomRotZ, 0.0));
			}
		}
	}

	void Game::Terminate()
	{

	}

	inline void Animate(Entity& entity, float time, float deltaTime)
	{
		// Sin[x] * Sin[x * a + b] * Sin[x * c + d]

		/*animation1D += deltaTime * speed * 0.001f;
		animation1B += deltaTime * speed * 0.000321f;
		animation2D += deltaTime * speed * 0.001232f;
		animation2B += deltaTime * speed * 0.000223f;*/
		animation1D = time * speed * 10;
		animation1B = time * speed * 3.21;
		animation2D = time * speed * 12.32;
		animation2B = time * speed * 2.23;

		float x = entity.GetPosition().z * zFrequency;
		float x2 = entity.GetPosition().x * xFrequency;
		entity.SetPosition(glm::vec3(entity.GetPosition().x,
									zWaveStrength * sin(CONTANTEA + x) * sin(x * animation1A + animation1B) * sin(x * animation1C + animation1B)*
									xWaveStrength * sin(CONTANTEB + x2) * sin(x2 * animation2A + animation2B) * sin(x2 * animation2C + animation2B),
									entity.GetPosition().z));
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		cameraRot = time * 0.01;

		zFrequency = glm::mix(zFrequency, tFreq, deltaTime * LERP_TIME);
		xFrequency = glm::mix(xFrequency, tFreq, deltaTime * LERP_TIME);
		xWaveStrength = glm::mix(xWaveStrength, tStrength, deltaTime * LERP_TIME);
		zWaveStrength = glm::mix(zWaveStrength, tStrength, deltaTime * LERP_TIME);
		speed = glm::mix(speed, tSpeed, deltaTime * LERP_TIME);

		glm::vec3 cameraPos(cos(cameraRot) * cameraRadius, posY, sin(cameraRot) * cameraRadius);
		cameraRadius += deltaTime*10.0;
		posY += deltaTime;
		
		theCamera.SetPosition(cameraPos);
		theCamera.LookAt(glm::vec3(0.0,0.0,0.0), glm::vec3(0.0,1.0,0.0));

		for (int i = 0; i < UNIQUE_COUNT; i++)
		{
			Animate(errt[i], time, deltaTime);

			for (int j = 0; j < COPY_COUNT; j++)
			{
				Animate(errts[j + i * COPY_COUNT], time, deltaTime);
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
			case GLFW_KEY_RIGHT:
				selectedVariable++;
				break;
			case GLFW_KEY_LEFT:
				selectedVariable--;
				break;
			}
		}
	}

	void Game::OnMouseScroll(double xoffset, double yoffset)
	{

		switch (selectedVariable % 3)
		{
		case 0:
			tFreq += yoffset * 0.0012;
			std::cout << "new freq: " << tFreq << std::endl;
			break;
		case 1:
			tStrength += yoffset * 0.7;
			std::cout << "new strength: " << tStrength << std::endl;
			break;
		case 2:
			tSpeed += yoffset * 0.0007;
			std::cout << "new speed: " << tSpeed << std::endl;
			break;

		}

		/*xWaveStrength += yoffset * 0.7;
		zWaveStrength += yoffset * 0.7;*/
	}

	void Game::OnMouseMove(double xpos, double ypos, const glm::vec2& mousePosDelta)
	{
	}
}