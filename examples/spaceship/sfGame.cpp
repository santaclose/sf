#include "../../user/Game.h"
#include <GLFW/glfw3.h>
#include "../../src/Texture.h"
#include "../../src/Material.h"
#include "../../src/Model.h"
#include "../../src/ModelReference.h"
#include "../../src/Math.h"
#include "../../src/Camera.h"

#define SENSITIVITY 0.007

#define COUNT 4000
#define SPAWN_RANGE 700.0

namespace User
{
	float shipSpeed = 5.0;
	glm::fquat targetShipRotation = glm::fquat(1.0, 0.0, 0.0, 0.0);
	Camera theCamera = Camera(1.7777777777777, 90.0, 0.1, 10000.0);
	Camera lookBackCamera = Camera(1.7777777777777, 120.0, 0.1, 1000.0);

	Shader shadelessShader;
	Shader uvShader;
	Shader colorShader;
	Shader noiseShader;
	/*Shader pbrShader;

	Texture shipTexture;
	Texture shipNormalmap;
	Texture shipRoughness;
	Texture shipMetallic;

	Texture butterflyTexture;
	Texture butterflyNormalmap;
	Texture butterflyRoughness;
	Texture butterflyMetallic;

	Material shipMaterial;
	Material butterflyMaterial;*/

	Material colorsMaterial;
	Material noiseMaterial;
	Material uvMaterial;

	Model ship;
	Model butterfly[3];
	ModelReference* butterflies;

	void Game::Initialize()
	{
		shadelessShader.CreateFromFiles("assets/shaders/pbrV.shader", "assets/shaders/shadelessF.shader");
		colorShader.CreateFromFiles("examples/spaceship/randomColorsV.shader", "examples/spaceship/randomColorsF.shader");
		uvShader.CreateFromFiles("assets/shaders/pbrV.shader", "assets/shaders/uvF.shader");
		noiseShader.CreateFromFiles("examples/spaceship/noiseV.shader", "examples/spaceship/noiseF.shader");
		//pbrShader.CreateFromFiles("assets/shaders/pbrV.shader", "assets/shaders/gpbrF.shader");

		uvMaterial.CreateFromShader(&uvShader);
		colorsMaterial.CreateFromShader(&colorShader);
		noiseMaterial.CreateFromShader(&noiseShader);

		/*shipTexture.CreateFromFile("examples/spaceship/wolfen/Wolfen_plate_BaseColor.png", Texture::Type::Albedo);
		shipNormalmap.CreateFromFile("examples/spaceship/wolfen/Wolfen_plate_Normal.png", Texture::Type::Normals);
		shipRoughness.CreateFromFile("examples/spaceship/wolfen/Wolfen_plate_Roughness.png", Texture::Type::Roughness);
		shipMetallic.CreateFromFile("examples/spaceship/wolfen/Wolfen_plate_Metallic.png", Texture::Type::Metallic);

		butterflyTexture.CreateFromFile("examples/spaceship/butterfly/Butterfly Fighter_pink_BaseColor.png", Texture::Type::Albedo);
		butterflyNormalmap.CreateFromFile("examples/spaceship/butterfly/Butterfly Fighter_pink_Normal.png", Texture::Type::Normals);
		butterflyRoughness.CreateFromFile("examples/spaceship/butterfly/Butterfly Fighter_pink_Roughness.png", Texture::Type::Roughness);
		butterflyMetallic.CreateFromFile("examples/spaceship/butterfly/Butterfly Fighter_pink_Metallic.png", Texture::Type::Metallic);

		shipMaterial.CreateFromShader(&pbrShader);
		shipMaterial.SetUniform("albedoTexture", &shipTexture, Material::UniformType::_Texture);
		shipMaterial.SetUniform("normalTexture", &shipNormalmap, Material::UniformType::_Texture);
		shipMaterial.SetUniform("roughnessTexture", &shipRoughness, Material::UniformType::_Texture);
		shipMaterial.SetUniform("metalnessTexture", &shipMetallic, Material::UniformType::_Texture);

		butterflyMaterial.CreateFromShader(&pbrShader);
		butterflyMaterial.SetUniform("albedoTexture", &butterflyTexture, Material::UniformType::_Texture);
		butterflyMaterial.SetUniform("normalTexture", &butterflyNormalmap, Material::UniformType::_Texture);
		butterflyMaterial.SetUniform("roughnessTexture", &butterflyRoughness, Material::UniformType::_Texture);
		butterflyMaterial.SetUniform("metalnessTexture", &butterflyMetallic, Material::UniformType::_Texture);*/

		ship.CreateFromOBJ("examples/spaceship/ship.obj", 0.3, true);
		ship.SetMaterial(&uvMaterial);
		targetShipRotation = ship.GetRotation();

		butterfly[0].CreateFromOBJ("examples/spaceship/thing.obj", 1.0, true);
		butterfly[0].SetMaterial(&uvMaterial);
		butterfly[1].CreateFromOBJ("examples/spaceship/thing.obj", 1.0, true);
		butterfly[1].SetMaterial(&colorsMaterial);
		butterfly[2].CreateFromOBJ("examples/spaceship/thing.obj", 1.0, true);
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

	void Game::Terminate()
	{

	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		ship.SetPosition(ship.GetPosition() + ship.Forward() * shipSpeed * deltaTime);

		ship.SetRotation(targetShipRotation);
		theCamera.SetPosition(glm::mix(theCamera.GetPosition(), ship.GetPosition() - (ship.Forward() * 4.0) + (ship.Up()), (float)(deltaTime * 2.0)));
		theCamera.LookAt(ship.GetPosition(), ship.Up());
		lookBackCamera.SetPosition(ship.GetPosition() + ship.Forward() * 4.0);
		lookBackCamera.LookAt(ship.GetPosition(), ship.Up());

		/*for (int i = 0; i < COUNT; i++)
		{
			butterflies[i].SetRotation(butterflies[i].GetRotation() * glm::fquat(glm::vec3(0, 0, sqrt(time) * 0.00001 * i)));
		}*/
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

	void Game::OnMouseScroll(double xoffset, double yoffset)
	{
		shipSpeed += yoffset;
	}

	void Game::OnMouseMove(double xpos, double ypos, const glm::vec2& mousePosDelta)
	{
		targetShipRotation *= glm::fquat(glm::vec3(mousePosDelta[1] * SENSITIVITY, 0.0, -mousePosDelta[0] * SENSITIVITY));
	}
}