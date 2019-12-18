#include "sfGame.h"
#include <GLFW/glfw3.h>
#include "../Texture.h"
#include "../Material.h"
#include "../Model.h"
#include "../ModelReference.h"
#include "../Math.h"
#include "errt.h"

#define SENSITIVITY 0.007

namespace User
{
	float Game::shipSpeed = 5.0;
	glm::fquat Game::targetShipRotation = glm::fquat(1.0, 0.0, 0.0, 0.0);
	Camera Game::theCamera = Camera(1.7777777777777, 90.0, 0.1, 10000.0);
	Camera Game::lookBackCamera = Camera(1.7777777777777, 120.0, 0.1, 1000.0);

	Shader theShader;
	Shader colorShader;
	Texture shipTexture;
	Texture shipNormalmap;
	Texture shipRoughness;
	Texture shipMetallic;
	Texture butterflyTexture;
	Texture butterflyNormalmap;
	Texture butterflyRoughness;
	Texture butterflyMetallic;
	Material shipMaterial;
	Material butterflyMaterial;

	Material colorsMaterial;

	Model ship;
	Model monkey;
	ModelReference* monkeys;

	Model errt;

	void Game::Initialize()
	{
		theShader.CreateFromFiles("res/shaders/pbrV.shader", "res/shaders/gpbrF.shader");
		colorShader.CreateFromFiles("user/noiseV.shader", "user/noiseF.shader");

		colorsMaterial.CreateFromShader(&colorShader);

		shipTexture.CreateFromFile("user/64/wolfen/Wolfen_plate_BaseColor.png", Texture::Type::ColorData);
		shipNormalmap.CreateFromFile("user/64/wolfen/Wolfen_plate_Normal.png", Texture::Type::NonColorData);
		shipRoughness.CreateFromFile("user/64/wolfen/Wolfen_plate_Roughness.png", Texture::Type::NonColorData);
		shipMetallic.CreateFromFile("user/64/wolfen/Wolfen_plate_Metallic.png", Texture::Type::NonColorData);
		butterflyTexture.CreateFromFile("user/64/butterfly/Butterfly Fighter_pink_BaseColor.png", Texture::Type::ColorData);
		butterflyNormalmap.CreateFromFile("user/64/butterfly/Butterfly Fighter_pink_Normal.png", Texture::Type::NonColorData);
		butterflyRoughness.CreateFromFile("user/64/butterfly/Butterfly Fighter_pink_Roughness.png", Texture::Type::NonColorData);
		butterflyMetallic.CreateFromFile("user/64/butterfly/Butterfly Fighter_pink_Metallic.png", Texture::Type::NonColorData);
		shipMaterial.CreateFromShader(&theShader);
		shipMaterial.SetUniform("albedoTexture", &shipTexture, Material::UniformType::_Texture);
		shipMaterial.SetUniform("normalTexture", &shipNormalmap, Material::UniformType::_Texture);
		shipMaterial.SetUniform("roughnessTexture", &shipRoughness, Material::UniformType::_Texture);
		shipMaterial.SetUniform("metalnessTexture", &shipMetallic, Material::UniformType::_Texture);
		butterflyMaterial.CreateFromShader(&theShader);
		butterflyMaterial.SetUniform("albedoTexture", &butterflyTexture, Material::UniformType::_Texture);
		butterflyMaterial.SetUniform("normalTexture", &butterflyNormalmap, Material::UniformType::_Texture);
		butterflyMaterial.SetUniform("roughnessTexture", &butterflyRoughness, Material::UniformType::_Texture);
		butterflyMaterial.SetUniform("metalnessTexture", &butterflyMetallic, Material::UniformType::_Texture);

		ship.CreateFromOBJ("user/64/wolfen/Wolfen.obj", 0.3, true);
		ship.SetMaterial(&shipMaterial);
		targetShipRotation = ship.GetRotation();

		monkey.CreateFromOBJ("user/64/Butterfly Fighter.obj", 1.0, true);
		monkey.SetMaterial(&butterflyMaterial);

		errt.CreateFromCode(User::GenerateModel, false);
		errt.SetMaterial(&colorsMaterial);

		monkeys = new ModelReference[4000];
		for (int i = 0; i < 4000; i++)
		{
			monkeys[i].CreateFomModel(monkey);
			monkeys[i].SetScale(Math::Random() * 20.0 + 1.0);
			//monkeys[i].CreateFromOBJ("user/monkey.obj", RandomValue() * 20.0 + 1.0);
			monkeys[i].SetPosition(glm::vec3((Math::Random() - 0.5) * 500.0, (Math::Random() - 0.5) * 500.0, (float)(i * -5)));
			monkeys[i].SetRotation(glm::fquat(glm::vec3(0.0, glm::radians(180.0), glm::radians(Math::Random() * 360.0))));
		}
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		ship.SetPosition(ship.GetPosition() + ship.Forward() * shipSpeed * deltaTime);
		//ship.SetRotation(glm::mix(ship.GetRotation(), targetShipRotation, (float)(deltaTime * 3.0)));
		//ship.SetRotation(glm::slerp(ship.GetRotation(), targetShipRotation, (float)(deltaTime * 7.0)));
		ship.SetRotation(targetShipRotation);
		theCamera.SetPosition(glm::mix(theCamera.GetPosition(), ship.GetPosition() - (ship.Forward() * 4.0) + (ship.Up()), (float)(deltaTime * 2.0)));
		theCamera.LookAt(ship.GetPosition(), ship.Up());
		lookBackCamera.SetPosition(ship.GetPosition() + ship.Forward() * 4.0);
		lookBackCamera.LookAt(ship.GetPosition(), ship.Up());

		for (int i = 0; i < 4000; i++)
		{
			monkeys[i].SetRotation(monkeys[i].GetRotation() * glm::fquat(glm::vec3(0, 0, sqrt(time) * 0.00001 * i)));
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
			case GLFW_KEY_SPACE:
				if (Camera::boundCamera == &theCamera)
					lookBackCamera.Bind();
				else
					theCamera.Bind();
				break;
			}
		}
	}

	void Game::OnScroll(double xoffset, double yoffset)
	{
		shipSpeed += yoffset;
	}

	void Game::OnMouseMove(double xpos, double ypos, const glm::vec2& mousePosDelta)
	{
		targetShipRotation *= glm::fquat(glm::vec3(mousePosDelta[1] * SENSITIVITY, 0.0, -mousePosDelta[0] * SENSITIVITY));
	}
}