#include "pbrTest.h"
#include <GLFW/glfw3.h>
#include "../Texture.h"
#include "../Cubemap.h"
#include "../Material.h"
#include "../Model.h"
#include "../ModelReference.h"
#include "../Math.h"
#include "../Skybox.h"
#include <iostream>


namespace User
{
	glm::vec3 targetGimbalRotation = glm::vec3(0.0, 0.0, 0.0);
	Camera camera = Camera(1.7777777777777, 90.0, 0.1, 10000.0);
	Entity gimbal;
	float cameraDistance = 3.0;

	Shader pbrShader;

	Material sciFiHelmetMaterial;
	Texture sciFiHelmetAlbedo;
	Texture sciFiHelmetNormalmap;
	Texture sciFiHelmetRoughness;
	Texture sciFiHelmetMetallic;

	Material damagedHelmetMaterial;
	Texture damagedHelmetAlbedo;
	Texture damagedHelmetNormalmap;
	Texture damagedHelmetRoughness;
	Texture damagedHelmetMetallic;
	Texture damagedHelmetEmissive;

	HdrTexture envTexture;
	Cubemap envCubemap;
	Cubemap irradianceCubemap;

#define MODEL_OFFSET 50.0
	std::vector<Model*> models;
	int selectedModel = 0;

	void Game::Initialize()
	{
		pbrShader.CreateFromFiles("res/shaders/pbrV.shader", "res/shaders/gpbrF.shader");

		sciFiHelmetMaterial.CreateFromShader(&pbrShader);
		sciFiHelmetAlbedo.CreateFromFile("user/gltf/SciFiHelmet/albedo.png", Texture::Type::Albedo);
		sciFiHelmetNormalmap.CreateFromFile("user/gltf/SciFiHelmet/normals.png", Texture::Type::Normals);
		sciFiHelmetRoughness.CreateFromFile("user/gltf/SciFiHelmet/roughness.png", Texture::Type::Roughness);
		sciFiHelmetMetallic.CreateFromFile("user/gltf/SciFiHelmet/metallic.png", Texture::Type::Metallic);
		sciFiHelmetMaterial.SetUniform("albedoTexture", &sciFiHelmetAlbedo, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("normalTexture", &sciFiHelmetNormalmap, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("roughnessTexture", &sciFiHelmetRoughness, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("metalnessTexture", &sciFiHelmetMetallic, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("emissiveTexture", nullptr, Material::UniformType::_Texture);

		damagedHelmetMaterial.CreateFromShader(&pbrShader);
		damagedHelmetAlbedo.CreateFromFile("user/gltf/DamagedHelmet/albedo.jpg", Texture::Type::Albedo);
		damagedHelmetNormalmap.CreateFromFile("user/gltf/DamagedHelmet/normals.jpg", Texture::Type::Normals);
		damagedHelmetRoughness.CreateFromFile("user/gltf/DamagedHelmet/roughness.png", Texture::Type::Roughness);
		damagedHelmetMetallic.CreateFromFile("user/gltf/DamagedHelmet/metallic.png", Texture::Type::Metallic);
		damagedHelmetEmissive.CreateFromFile("user/gltf/DamagedHelmet/Default_emissive.jpg", Texture::Type::Albedo);
		damagedHelmetMaterial.SetUniform("albedoTexture", &damagedHelmetAlbedo, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("normalTexture", &damagedHelmetNormalmap, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("roughnessTexture", &damagedHelmetRoughness, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("metalnessTexture", &damagedHelmetMetallic, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("emissiveTexture", &damagedHelmetEmissive, Material::UniformType::_Texture);

		//envCubemap.CreateFomHDR(envTexture);
		//irradianceCubemap.CreateIrradiance(envCubemap);

		envCubemap.CreateFromFiles("user/cubemap/y", ".jpg");
		//envTexture.CreateFromFile("user/hdr/newport_loft.hdr");
		//envCubemap.CreateFomHDR(envTexture);
		Skybox::Generate(&envCubemap);

		models.emplace_back();
		models.back() = new Model();
		models.back()->CreateFromGLTF("user/gltf/SciFiHelmet/SciFiHelmet.gltf");
		models.back()->SetMaterial(&sciFiHelmetMaterial);
		models.emplace_back();
		models.back() = new Model();
		//models.back()->CreateFromGLTF("user/icosphere.gltf");
		models.back()->CreateFromGLTF("user/gltf/DamagedHelmet/DamagedHelmet.gltf");
		models.back()->SetMaterial(&damagedHelmetMaterial);
		models.emplace_back();

		for (int i = 0; i < models.size(); i++)
		{
			models[i]->SetPosition(glm::vec3(i * MODEL_OFFSET, 0.0, 0.0));
			if (i > 0)
				models[i]->SetScale(0.0);
		}

		gimbal.SetPosition(glm::vec3(0.0, 0.0, 0.0));
		gimbal.LookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		camera.SetPosition(glm::vec3(0.0, 0.0, cameraDistance));
		camera.LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	}

#define SPEED 15.0f
	void Game::OnUpdate(float deltaTime, float time)
	{
		gimbal.SetRotation(glm::slerp(gimbal.GetRotation(), glm::fquat(targetGimbalRotation), deltaTime * SPEED));
		camera.SetPosition(gimbal.GetPosition() + gimbal.Forward() * cameraDistance);
		camera.LookAt(gimbal.GetPosition(), glm::vec3(0.0, 1.0, 0.0));

		models[selectedModel]->SetRotation(glm::vec3(0.0, time / 20.0, 0.0));
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
				models[selectedModel]->SetScale(0.0);
				selectedModel++;
				selectedModel %= models.size();
				models[selectedModel]->SetScale(1.0);
				gimbal.SetPosition(glm::vec3(selectedModel * MODEL_OFFSET, 0.0, 0.0));
				break;
			case GLFW_KEY_LEFT:
				models[selectedModel]->SetScale(0.0);
				selectedModel--;
				selectedModel %= models.size();
				models[selectedModel]->SetScale(1.0);
				gimbal.SetPosition(glm::vec3(selectedModel * MODEL_OFFSET, 0.0, 0.0));
				break;
			}
		}
	}

#define MOVE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.06
	void Game::OnMouseScroll(double xoffset, double yoffset)
	{
		cameraDistance -= SCROLL_SENSITIVITY * yoffset;
	}

	void Game::OnMouseMove(double xpos, double ypos, const glm::vec2& mousePosDelta)
	{
		targetGimbalRotation.y += mousePosDelta.x * MOVE_SENSITIVITY;
		targetGimbalRotation.x += mousePosDelta.y * MOVE_SENSITIVITY;
	}
	void Game::Terminate()
	{
		for (Model* m : models)
			delete m;
	}
}