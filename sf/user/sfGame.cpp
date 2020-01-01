#include "sfGame.h"
#include <GLFW/glfw3.h>
#include "../Texture.h"
#include "../Material.h"
#include "../Model.h"
#include "../ModelReference.h"
#include "../Math.h"
#include "errt.h"

#define SENSITIVITY 0.007

#define COUNT 4000
#define SPAWN_RANGE 700.0

namespace User
{
	float Game::shipSpeed = 5.0;
	glm::fquat Game::targetShipRotation = glm::fquat(1.0, 0.0, 0.0, 0.0);
	Camera Game::theCamera = Camera(1.7777777777777, 90.0, 0.1, 10000.0);
	Camera Game::lookBackCamera = Camera(1.7777777777777, 120.0, 0.1, 1000.0);

	Shader pbrShader;
	Shader colorShader;
	Shader noiseShader;

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
	Material noiseMaterial;

	Model ship;
	Model butterfly[3];
	ModelReference* butterflies;

	void Game::Initialize()
	{
		pbrShader.CreateFromFiles("res/shaders/pbrV.shader", "res/shaders/gpbrF.shader");
		colorShader.CreateFromFiles("user/randomColorsV.shader", "user/randomColorsF.shader");
		noiseShader.CreateFromFiles("user/noiseV.shader", "user/noiseF.shader");

		colorsMaterial.CreateFromShader(&colorShader);
		noiseMaterial.CreateFromShader(&noiseShader);

		shipTexture.CreateFromFile("user/64/wolfen/Wolfen_plate_BaseColor.png", Texture::Type::ColorData);
		shipNormalmap.CreateFromFile("user/64/wolfen/Wolfen_plate_Normal.png", Texture::Type::NonColorData);
		shipRoughness.CreateFromFile("user/64/wolfen/Wolfen_plate_Roughness.png", Texture::Type::NonColorData);
		shipMetallic.CreateFromFile("user/64/wolfen/Wolfen_plate_Metallic.png", Texture::Type::NonColorData);
		butterflyTexture.CreateFromFile("user/64/butterfly/Butterfly Fighter_pink_BaseColor.png", Texture::Type::ColorData);
		butterflyNormalmap.CreateFromFile("user/64/butterfly/Butterfly Fighter_pink_Normal.png", Texture::Type::NonColorData);
		butterflyRoughness.CreateFromFile("user/64/butterfly/Butterfly Fighter_pink_Roughness.png", Texture::Type::NonColorData);
		butterflyMetallic.CreateFromFile("user/64/butterfly/Butterfly Fighter_pink_Metallic.png", Texture::Type::NonColorData);
		shipMaterial.CreateFromShader(&pbrShader);
		shipMaterial.SetUniform("albedoTexture", &shipTexture, Material::UniformType::_Texture);
		shipMaterial.SetUniform("normalTexture", &shipNormalmap, Material::UniformType::_Texture);
		shipMaterial.SetUniform("roughnessTexture", &shipRoughness, Material::UniformType::_Texture);
		shipMaterial.SetUniform("metalnessTexture", &shipMetallic, Material::UniformType::_Texture);
		butterflyMaterial.CreateFromShader(&pbrShader);
		butterflyMaterial.SetUniform("albedoTexture", &butterflyTexture, Material::UniformType::_Texture);
		butterflyMaterial.SetUniform("normalTexture", &butterflyNormalmap, Material::UniformType::_Texture);
		butterflyMaterial.SetUniform("roughnessTexture", &butterflyRoughness, Material::UniformType::_Texture);
		butterflyMaterial.SetUniform("metalnessTexture", &butterflyMetallic, Material::UniformType::_Texture);

		ship.CreateFromOBJ("user/64/wolfen/Wolfen.obj", 0.3, true);
		ship.SetMaterial(&shipMaterial);
		targetShipRotation = ship.GetRotation();

		butterfly[0].CreateFromOBJ("user/64/Butterfly Fighter.obj", 1.0, true);
		butterfly[0].SetMaterial(&butterflyMaterial);
		butterfly[1].CreateFromOBJ("user/64/Butterfly Fighter.obj", 1.0, true);
		butterfly[1].SetMaterial(&colorsMaterial);
		butterfly[2].CreateFromOBJ("user/64/Butterfly Fighter.obj", 1.0, true);
		butterfly[2].SetMaterial(&noiseMaterial);

		butterflies = new ModelReference[COUNT];
		for (int i = 0; i < COUNT; i++)
		{
			butterflies[i].CreateFomModel(butterfly[rand() % 3]);
			butterflies[i].SetScale(Math::Random() * (COUNT - i)/200.0 + 1.0);

			butterflies[i].SetPosition(glm::vec3((Math::Random() - 0.5) * SPAWN_RANGE, (Math::Random() - 0.5) * SPAWN_RANGE, (float)(i * -2)));
			butterflies[i].SetRotation(glm::fquat(glm::vec3(0.0, glm::radians(180.0), glm::radians(Math::Random() * 360.0))));
		}
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		ship.SetPosition(ship.GetPosition() + ship.Forward() * shipSpeed * deltaTime);

		ship.SetRotation(targetShipRotation);
		theCamera.SetPosition(glm::mix(theCamera.GetPosition(), ship.GetPosition() - (ship.Forward() * 4.0) + (ship.Up()), (float)(deltaTime * 2.0)));
		theCamera.LookAt(ship.GetPosition(), ship.Up());
		lookBackCamera.SetPosition(ship.GetPosition() + ship.Forward() * 4.0);
		lookBackCamera.LookAt(ship.GetPosition(), ship.Up());

		for (int i = 0; i < COUNT; i++)
		{
			butterflies[i].SetRotation(butterflies[i].GetRotation() * glm::fquat(glm::vec3(0, 0, sqrt(time) * 0.00001 * i)));
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