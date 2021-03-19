#include "../../user/Game.h"
#include <GLFW/glfw3.h>
#include "../../src/Texture.h"
#include "../../src/Cubemap.h"
#include "../../src/Material.h"
#include "../../src/Model.h"
#include "../../src/ModelReference.h"
#include "../../src/Math.h"
#include "../../src/Skybox.h"
#include "../../src/Input.h"
#include <iostream>

#define MOVE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.06
#define SPEED 15.0f
#define MODEL_OFFSET 50.0

namespace User
{
	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);
	Camera* camera;
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

	std::vector<Model*> models;
	int selectedModel = 0;

	void Game::Initialize()
	{
		CameraSpecs cs;
		cs.aspectRatio = 16.0f / 9.0f;
		cs.farClippingPlane = 100.0f;
		cs.nearClippingPlane = 0.01f;
		cs.fieldOfView = glm::radians(90.0f);
		camera = new Camera(cs);

		pbrShader.CreateFromFiles("assets/shaders/pbrV.shader", "assets/shaders/gpbrF.shader");

		sciFiHelmetMaterial.CreateFromShader(&pbrShader);
		sciFiHelmetAlbedo.CreateFromFile("examples/pbr/gltf/SciFiHelmet/albedo.png", Texture::Type::Albedo);
		sciFiHelmetNormalmap.CreateFromFile("examples/pbr/gltf/SciFiHelmet/normals.png", Texture::Type::Normals);
		sciFiHelmetRoughness.CreateFromFile("examples/pbr/gltf/SciFiHelmet/roughness.png", Texture::Type::Roughness);
		sciFiHelmetMetallic.CreateFromFile("examples/pbr/gltf/SciFiHelmet/metallic.png", Texture::Type::Metallic);
		sciFiHelmetMaterial.SetUniform("albedoTexture", &sciFiHelmetAlbedo, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("normalTexture", &sciFiHelmetNormalmap, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("roughnessTexture", &sciFiHelmetRoughness, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("metalnessTexture", &sciFiHelmetMetallic, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("emissiveTexture", nullptr, Material::UniformType::_Texture);

		damagedHelmetMaterial.CreateFromShader(&pbrShader);
		damagedHelmetAlbedo.CreateFromFile("examples/pbr/gltf/DamagedHelmet/albedo.jpg", Texture::Type::Albedo);
		damagedHelmetNormalmap.CreateFromFile("examples/pbr/gltf/DamagedHelmet/normals.jpg", Texture::Type::Normals);
		damagedHelmetRoughness.CreateFromFile("examples/pbr/gltf/DamagedHelmet/roughness.png", Texture::Type::Roughness);
		damagedHelmetMetallic.CreateFromFile("examples/pbr/gltf/DamagedHelmet/metallic.png", Texture::Type::Metallic);
		damagedHelmetEmissive.CreateFromFile("examples/pbr/gltf/DamagedHelmet/Default_emissive.jpg", Texture::Type::Albedo);
		damagedHelmetMaterial.SetUniform("albedoTexture", &damagedHelmetAlbedo, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("normalTexture", &damagedHelmetNormalmap, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("roughnessTexture", &damagedHelmetRoughness, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("metalnessTexture", &damagedHelmetMetallic, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("emissiveTexture", &damagedHelmetEmissive, Material::UniformType::_Texture);

		envCubemap.CreateFromFiles("examples/pbr/cubemap/y", ".jpg");
		Skybox::Generate(&envCubemap);

		models.emplace_back();
		models.back() = new Model();
		models.back()->CreateFromFile("examples/pbr/gltf/SciFiHelmet/SciFiHelmet.gltf");
		models.back()->SetMaterial(&sciFiHelmetMaterial);
		models.emplace_back();
		models.back() = new Model();
		models.back()->CreateFromFile("examples/pbr/gltf/DamagedHelmet/DamagedHelmet.gltf");
		models.back()->SetMaterial(&damagedHelmetMaterial);

		for (int i = 0; i < models.size(); i++)
		{
			std::cout << "setting model position " << i << std::endl;
			models[i]->SetPosition(glm::vec3(i * MODEL_OFFSET, 0.0, 0.0));
			if (i > 0)
				models[i]->SetScale(0.0);
		}

		std::cout << "setting gimbal position" << std::endl;
		gimbal.SetPosition(glm::vec3(0.0, 0.0, 0.0));
		gimbal.LookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		std::cout << "setting camera position" << std::endl;
		camera->SetPosition(glm::vec3(0.0, 0.0, cameraDistance));
		camera->LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	}

	void Game::OnUpdate(float deltaTime, float time)
	{

		if (Input::KeyDown(Input::KeyCode::M))
			glPolygonMode(GL_FRONT, GL_POINT);
		else if (Input::KeyDown(Input::KeyCode::N))
			glPolygonMode(GL_FRONT, GL_LINE);
		else if (Input::KeyDown(Input::KeyCode::B))
			glPolygonMode(GL_FRONT, GL_FILL);
		else if (Input::KeyDown(Input::KeyCode::Right))
		{
			models[selectedModel]->SetScale(0.0);
			selectedModel++;
			selectedModel %= models.size();
			models[selectedModel]->SetScale(1.0);
			gimbal.SetPosition(glm::vec3(selectedModel * MODEL_OFFSET, 0.0, 0.0));
		}
		else if (Input::KeyDown(Input::KeyCode::Left))
		{
			models[selectedModel]->SetScale(0.0);
			selectedModel--;
			selectedModel %= models.size();
			models[selectedModel]->SetScale(1.0);
			gimbal.SetPosition(glm::vec3(selectedModel * MODEL_OFFSET, 0.0, 0.0));
		}
		cameraDistance -= SCROLL_SENSITIVITY * Input::MouseScrollUp() ? 1.0f : 0.0f;
		cameraDistance += SCROLL_SENSITIVITY * Input::MouseScrollDown() ? 1.0f : 0.0f;

		targetGimbalRotation.y -= Input::MousePosDeltaX() * MOVE_SENSITIVITY;
		targetGimbalRotation.x += Input::MousePosDeltaY() * MOVE_SENSITIVITY;

		gimbal.SetRotation(glm::slerp(gimbal.GetRotation(), glm::fquat(targetGimbalRotation), deltaTime * SPEED));
		camera->SetPosition(gimbal.GetPosition() + gimbal.Forward() * cameraDistance);
		camera->LookAt(gimbal.GetPosition(), glm::vec3(0.0, 1.0, 0.0));

		models[selectedModel]->SetRotation(glm::vec3(0.0, time / 20.0, 0.0));
	}
	void Game::Terminate()
	{
		for (Model* m : models)
			delete m;
	}
}