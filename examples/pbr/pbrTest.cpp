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
#include "../../src/GltfController.h"
#include "../../src/IblTools.h"
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
	bool rotationEnabled = false;

	Shader pbrShader;
	Shader pbrIblShader;

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

	Material asdfMaterial;
	Texture asdfTexture;

	std::vector<glm::vec3> dirLightsDir = { {0.0f, -0.5f, -1.0f},  {0.0f, -0.5f, 1.0f} };
	std::vector<glm::vec3> dirLightsRad = { {10.0f, 10.0f, 10.0f}, {2.0f, 2.0f, 2.0f} };

	//std::vector<glm::vec3> pLightsPos = { {1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} };
	//std::vector<glm::vec3> pLightsRad = { {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f} };
	//std::vector<float> pLightsRa = { 1.0f, 1.0f };

	Texture envTexture;
	Cubemap envCubemap;
	Cubemap irradianceCubemap;
	Cubemap prefilterCubemap;
	Texture lookupTexture;

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

		pbrShader.CreateFromFiles("assets/shaders/pbrV.shader", "assets/shaders/jpbrF.shader");
		pbrIblShader.CreateFromFiles("assets/shaders/pbrV.shader", "assets/shaders/pbrIblF.shader");
		pbrShader.Bind();
		pbrShader.SetUniform3fv("dLightDir", &(dirLightsDir[0].x), dirLightsDir.size());
		pbrShader.SetUniform3fv("dLightRad", &(dirLightsRad[0].x), dirLightsRad.size());
		//pbrShader.SetUniform3fv("pLightPos", &(pLightsPos[0].x), pLightsPos.size());
		//pbrShader.SetUniform3fv("pLightRad", &(pLightsRad[0].x), pLightsRad.size());
		//pbrShader.SetUniform1fv("pLightRa", &(pLightsRa[0]), pLightsRa.size());

		sciFiHelmetMaterial.CreateFromShader(&pbrIblShader);
		sciFiHelmetMaterial.SetUniform("useAlbedoTexture", (void*)true, Material::UniformType::_Boolean);
		sciFiHelmetMaterial.SetUniform("useNormalTexture", (void*)true, Material::UniformType::_Boolean);
		sciFiHelmetMaterial.SetUniform("useRoughnessTexture", (void*)true, Material::UniformType::_Boolean);
		sciFiHelmetMaterial.SetUniform("useMetalnessTexture", (void*)true, Material::UniformType::_Boolean);
		sciFiHelmetMaterial.SetUniform("useEmissiveTexture", (void*)false, Material::UniformType::_Boolean);
		sciFiHelmetMaterial.SetUniform("useAoTexture", (void*)false, Material::UniformType::_Boolean);
		sciFiHelmetAlbedo.CreateFromFile("examples/pbr/gltf/SciFiHelmet/albedo.png", Texture::Type::Albedo);
		sciFiHelmetNormalmap.CreateFromFile("examples/pbr/gltf/SciFiHelmet/normals.png", Texture::Type::Normals);
		sciFiHelmetRoughness.CreateFromFile("examples/pbr/gltf/SciFiHelmet/roughness.png", Texture::Type::Roughness);
		sciFiHelmetMetallic.CreateFromFile("examples/pbr/gltf/SciFiHelmet/metallic.png", Texture::Type::Metallic);
		sciFiHelmetMaterial.SetUniform("albedoTexture", &sciFiHelmetAlbedo, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("normalTexture", &sciFiHelmetNormalmap, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("roughnessTexture", &sciFiHelmetRoughness, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("metalnessTexture", &sciFiHelmetMetallic, Material::UniformType::_Texture);

		damagedHelmetMaterial.CreateFromShader(&pbrShader);
		damagedHelmetMaterial.SetUniform("useAlbedoTexture", (void*)true, Material::UniformType::_Boolean);
		damagedHelmetMaterial.SetUniform("useNormalTexture", (void*)true, Material::UniformType::_Boolean);
		damagedHelmetMaterial.SetUniform("useRoughnessTexture", (void*)true, Material::UniformType::_Boolean);
		damagedHelmetMaterial.SetUniform("useMetalnessTexture", (void*)true, Material::UniformType::_Boolean);
		damagedHelmetMaterial.SetUniform("useEmissiveTexture", (void*)true, Material::UniformType::_Boolean);
		damagedHelmetMaterial.SetUniform("useAoTexture", (void*)false, Material::UniformType::_Boolean);
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

		asdfMaterial.CreateFromShader(&pbrShader);
		asdfMaterial.SetUniform("useAlbedoTexture", (void*)true, Material::UniformType::_Boolean);
		asdfMaterial.SetUniform("useNormalTexture", (void*)false, Material::UniformType::_Boolean);
		asdfMaterial.SetUniform("useRoughnessTexture", (void*)false, Material::UniformType::_Boolean);
		asdfMaterial.SetUniform("useMetalnessTexture", (void*)false, Material::UniformType::_Boolean);
		asdfMaterial.SetUniform("useEmissiveTexture", (void*)false, Material::UniformType::_Boolean);
		asdfMaterial.SetUniform("useAoTexture", (void*)false, Material::UniformType::_Boolean);

		//envCubemap.CreateFromFiles("examples/pbr/cubemap/y", ".jpg");
		//Skybox::SetCubemap(&envCubemap);

		//envCubemap.CreateFromFiles("examples/pbr/cubemap/a", ".hdr", true);
		//irradianceCubemap = IblTools::DiffuseIrradiance(envCubemap);
		//Skybox::SetCubemap(&irradianceCubemap);
		
		envTexture.CreateFromFile("examples/pbr/lilienstein_4k.hdr", Texture::Type::HDR, true);
		//std::cout << envTexture.GetWidth() << std::endl;
		//std::cout << envTexture.GetHeight() << std::endl;
		//std::cout << envTexture.GlId() << std::endl;

		envCubemap.Create(512, true, true);
		irradianceCubemap.Create(32, false, true);
		prefilterCubemap.Create(128, true, true);
		prefilterCubemap.ComputeMipmap();
		lookupTexture.Create(512, 512, false, true, 2);
		//lookupTexture.CreateFromFile("ibl_brdf_lut.png", Texture::Normals, false);

		IblTools::HdrToCubemaps(envTexture, envCubemap, irradianceCubemap, prefilterCubemap, lookupTexture);

		Skybox::SetCubemap(&envCubemap);

		sciFiHelmetMaterial.SetUniform("irradianceMap", &irradianceCubemap, Material::UniformType::_Cubemap);
		sciFiHelmetMaterial.SetUniform("prefilterMap", &prefilterCubemap, Material::UniformType::_Cubemap);
		sciFiHelmetMaterial.SetUniform("brdfLUT", &lookupTexture, Material::UniformType::_Texture);

		int gltfid;

		models.emplace_back();
		models.back() = new Model();
		gltfid = GltfController::Load("examples/pbr/gltf/SciFiHelmet/SciFiHelmet.gltf");
		models.back()->CreateFromGltf(gltfid, 0);
		models.back()->SetMaterial(&sciFiHelmetMaterial);

		models.emplace_back();
		models.back() = new Model();
		gltfid = GltfController::Load("examples/pbr/gltf/DamagedHelmet/DamagedHelmet.gltf");
		models.back()->CreateFromGltf(gltfid, 0);
		models.back()->SetMaterial(&damagedHelmetMaterial);

		gltfid = GltfController::Load("examples/pbr/gltf/asdf/asdf.glb");
		//asdfTexture.CreateFromGltf(gltfid, 0);
		asdfMaterial.SetUniform("albedoTexture", &lookupTexture, Material::UniformType::_Texture);

		models.emplace_back();
		models.back() = new Model();
		models.back()->CreateFromGltf(gltfid, 0);
		models.back()->SetMaterial(&asdfMaterial);

		models.emplace_back();
		models.back() = new Model();
		models.back()->CreateFromGltf(gltfid, 1);
		models.back()->SetMaterial(&asdfMaterial);

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
		else if (Input::KeyDown(Input::KeyCode::Space))
			rotationEnabled = !rotationEnabled;
		else if (Input::KeyDown(Input::KeyCode::Right))
		{
			glm::fquat theRot = models[selectedModel]->GetRotation();
			models[selectedModel]->SetScale(0.0);
			selectedModel++;
			selectedModel %= models.size();
			models[selectedModel]->SetScale(1.0);
			models[selectedModel]->SetRotation(theRot);
			gimbal.SetPosition(glm::vec3(selectedModel * MODEL_OFFSET, 0.0, 0.0));
		}
		else if (Input::KeyDown(Input::KeyCode::Left))
		{
			glm::fquat theRot = models[selectedModel]->GetRotation();
			models[selectedModel]->SetScale(0.0);
			selectedModel--;
			selectedModel = (selectedModel + models.size()) % models.size();
			models[selectedModel]->SetScale(1.0);
			models[selectedModel]->SetRotation(theRot);
			gimbal.SetPosition(glm::vec3(selectedModel * MODEL_OFFSET, 0.0, 0.0));
		}
		cameraDistance -= SCROLL_SENSITIVITY * (Input::MouseScrollUp() ? 1.0f : 0.0f);
		cameraDistance += SCROLL_SENSITIVITY * (Input::MouseScrollDown() ? 1.0f : 0.0f);

		targetGimbalRotation.y -= Input::MousePosDeltaX() * MOVE_SENSITIVITY;
		targetGimbalRotation.x += Input::MousePosDeltaY() * MOVE_SENSITIVITY;

		gimbal.SetRotation(glm::slerp(gimbal.GetRotation(), glm::fquat(targetGimbalRotation), deltaTime * SPEED));
		camera->SetPosition(gimbal.GetPosition() + gimbal.Forward() * cameraDistance);
		camera->LookAt(gimbal.GetPosition(), glm::vec3(0.0, 1.0, 0.0));

		if (rotationEnabled)
			models[selectedModel]->SetRotation(models[selectedModel]->GetRotation() * glm::fquat(glm::vec3(0.0f, 0.07f * deltaTime, 0.0f)));

		//Skybox::SetExposure((glm::sin(time * 0.1f) + 1.0f) * 10.0f);
	}
	void Game::Terminate()
	{
		for (Model* m : models)
			delete m;
	}
}