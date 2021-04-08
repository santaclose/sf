#include <GLFW/glfw3.h>
#include <iostream>

#include <Game.h>
#include <Texture.h>
#include <Cubemap.h>
#include <Material.h>
#include <Model.h>
#include <ModelReference.h>
#include <ModelProcessor.h>
#include <Math.hpp>
#include <Skybox.h>
#include <Input.h>
#include <IblHelper.h>
#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#define MOVE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.06
#define SPEED 15.0f
#define MODEL_OFFSET 50.0

namespace sf
{
	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);
	Camera* camera;
	Entity gimbal;
	float cameraDistance = 3.0;
	bool rotationEnabled = false;

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

	Material shoeMaterial;
	Texture shoeAo;
	Texture shoeAlbedo;
	Texture shoeNormalmap;
	Texture shoeRoughness;
	Texture shoeMetallic;

	Material defaultMaterial;

	std::vector<glm::vec3> dirLightsDir = { {0.0f, -0.5f, -1.0f},  {0.0f, -0.5f, 1.0f} };
	std::vector<glm::vec3> dirLightsRad = { {10.0f, 10.0f, 10.0f}, {2.0f, 2.0f, 2.0f} };

	std::vector<glm::vec3> pLightsPos = { {1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} };
	std::vector<glm::vec3> pLightsRad = { {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f} };
	std::vector<float> pLightsRa = { 1.0f, 1.0f };

	Texture envTexture;
	Cubemap envCubemap;
	Cubemap irradianceCubemap;
	Cubemap prefilterCubemap;
	Texture lookupTexture;

	std::vector<Model*> models;
	int selectedModel = 0;

	void Game::Initialize(int argc, char** argv)
	{
		CameraSpecs cs;
		cs.farClippingPlane = 100.0f;
		cs.nearClippingPlane = 0.01f;
		cs.fieldOfView = glm::radians(75.0f);
		camera = new Camera(cs);

		pbrShader.CreateFromFiles("assets/shaders/pbrV.shader", "assets/shaders/pbrF.shader");

		//pbrShader.Bind();
		//pbrShader.SetUniform3fv("dLightDir", &(dirLightsDir[0].x), dirLightsDir.size());
		//pbrShader.SetUniform3fv("dLightRad", &(dirLightsRad[0].x), dirLightsRad.size());
		//pbrShader.SetUniform3fv("pLightPos", &(pLightsPos[0].x), pLightsPos.size());
		//pbrShader.SetUniform3fv("pLightRad", &(pLightsRad[0].x), pLightsRad.size());
		//pbrShader.SetUniform1fv("pLightRa", &(pLightsRa[0]), pLightsRa.size());

		defaultMaterial.CreateFromShader(&pbrShader);
		defaultMaterial.SetUniform("useAlbedoTexture", (void*)false, Material::UniformType::_Boolean);
		defaultMaterial.SetUniform("useNormalTexture", (void*)false, Material::UniformType::_Boolean);
		defaultMaterial.SetUniform("useRoughnessTexture", (void*)false, Material::UniformType::_Boolean);
		defaultMaterial.SetUniform("useMetalnessTexture", (void*)false, Material::UniformType::_Boolean);
		defaultMaterial.SetUniform("useEmissiveTexture", (void*)false, Material::UniformType::_Boolean);
		defaultMaterial.SetUniform("useAoTexture", (void*)false, Material::UniformType::_Boolean);

		sciFiHelmetMaterial.CreateFromShader(&pbrShader);
		sciFiHelmetMaterial.SetUniform("useAlbedoTexture", (void*)true, Material::UniformType::_Boolean);
		sciFiHelmetMaterial.SetUniform("useNormalTexture", (void*)true, Material::UniformType::_Boolean);
		sciFiHelmetMaterial.SetUniform("useRoughnessTexture", (void*)true, Material::UniformType::_Boolean);
		sciFiHelmetMaterial.SetUniform("useMetalnessTexture", (void*)true, Material::UniformType::_Boolean);
		sciFiHelmetMaterial.SetUniform("useEmissiveTexture", (void*)false, Material::UniformType::_Boolean);
		sciFiHelmetMaterial.SetUniform("useAoTexture", (void*)false, Material::UniformType::_Boolean);
		sciFiHelmetAlbedo.CreateFromFile("examples/pbr/gltf/SciFiHelmet/albedo.png", 3, Texture::Color, Texture::UnsignedByte);
		sciFiHelmetNormalmap.CreateFromFile("examples/pbr/gltf/SciFiHelmet/normals.png", 3, Texture::NonColor, Texture::UnsignedByte);
		sciFiHelmetRoughness.CreateFromFile("examples/pbr/gltf/SciFiHelmet/roughness.png", 1, Texture::NonColor, Texture::UnsignedByte);
		sciFiHelmetMetallic.CreateFromFile("examples/pbr/gltf/SciFiHelmet/metallic.png", 1, Texture::NonColor, Texture::UnsignedByte);
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
		damagedHelmetAlbedo.CreateFromFile("examples/pbr/gltf/DamagedHelmet/albedo.jpg", 3, Texture::Color, Texture::UnsignedByte);
		damagedHelmetNormalmap.CreateFromFile("examples/pbr/gltf/DamagedHelmet/normals.jpg", 3, Texture::NonColor, Texture::UnsignedByte);
		damagedHelmetRoughness.CreateFromFile("examples/pbr/gltf/DamagedHelmet/roughness.png", 1, Texture::NonColor, Texture::UnsignedByte);
		damagedHelmetMetallic.CreateFromFile("examples/pbr/gltf/DamagedHelmet/metallic.png", 1, Texture::NonColor, Texture::UnsignedByte);
		damagedHelmetEmissive.CreateFromFile("examples/pbr/gltf/DamagedHelmet/Default_emissive.jpg", 3, Texture::Color, Texture::UnsignedByte);
		damagedHelmetMaterial.SetUniform("albedoTexture", &damagedHelmetAlbedo, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("normalTexture", &damagedHelmetNormalmap, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("roughnessTexture", &damagedHelmetRoughness, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("metalnessTexture", &damagedHelmetMetallic, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("emissiveTexture", &damagedHelmetEmissive, Material::UniformType::_Texture);

		shoeMaterial.CreateFromShader(&pbrShader);
		shoeMaterial.SetUniform("useAlbedoTexture", (void*)true, Material::UniformType::_Boolean);
		shoeMaterial.SetUniform("useNormalTexture", (void*)true, Material::UniformType::_Boolean);
		shoeMaterial.SetUniform("useRoughnessTexture", (void*)true, Material::UniformType::_Boolean);
		shoeMaterial.SetUniform("useMetalnessTexture", (void*)true, Material::UniformType::_Boolean);
		shoeMaterial.SetUniform("useEmissiveTexture", (void*)false, Material::UniformType::_Boolean);
		shoeMaterial.SetUniform("useAoTexture", (void*)true, Material::UniformType::_Boolean);
		shoeAlbedo.CreateFromFile("examples/pbr/gltf/MaterialsVariantsShoe/diffuseMidnight.jpg", 3, Texture::Color, Texture::UnsignedByte);
		shoeNormalmap.CreateFromFile("examples/pbr/gltf/MaterialsVariantsShoe/normal.jpg", 3, Texture::NonColor, Texture::UnsignedByte);
		shoeRoughness.CreateFromFile("examples/pbr/gltf/MaterialsVariantsShoe/roughness.png", 1, Texture::NonColor, Texture::UnsignedByte);
		shoeMetallic.CreateFromFile("examples/pbr/gltf/MaterialsVariantsShoe/metalness.png", 1, Texture::NonColor, Texture::UnsignedByte);
		shoeAo.CreateFromFile("examples/pbr/gltf/MaterialsVariantsShoe/ao.png", 1, Texture::NonColor, Texture::UnsignedByte);
		shoeMaterial.SetUniform("albedoTexture", &shoeAlbedo, Material::UniformType::_Texture);
		shoeMaterial.SetUniform("normalTexture", &shoeNormalmap, Material::UniformType::_Texture);
		shoeMaterial.SetUniform("roughnessTexture", &shoeRoughness, Material::UniformType::_Texture);
		shoeMaterial.SetUniform("metalnessTexture", &shoeMetallic, Material::UniformType::_Texture);
		shoeMaterial.SetUniform("aoTexture", &shoeAo, Material::UniformType::_Texture);
				
		envTexture.CreateFromFile("examples/pbr/newport_loft.hdr", 3, Texture::Color, Texture::Float16, Texture::ClampToEdge);
		IblHelper::HdrToCubemaps(envTexture, envCubemap, irradianceCubemap, prefilterCubemap, lookupTexture);

		Skybox::SetCubemap(&envCubemap);

		defaultMaterial.SetUniform("irradianceMap", &irradianceCubemap, Material::UniformType::_Cubemap);
		defaultMaterial.SetUniform("prefilterMap", &prefilterCubemap, Material::UniformType::_Cubemap);
		defaultMaterial.SetUniform("brdfLUT", &lookupTexture, Material::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("irradianceMap", &irradianceCubemap, Material::UniformType::_Cubemap);
		sciFiHelmetMaterial.SetUniform("prefilterMap", &prefilterCubemap, Material::UniformType::_Cubemap);
		sciFiHelmetMaterial.SetUniform("brdfLUT", &lookupTexture, Material::UniformType::_Texture);
		damagedHelmetMaterial.SetUniform("irradianceMap", &irradianceCubemap, Material::UniformType::_Cubemap);
		damagedHelmetMaterial.SetUniform("prefilterMap", &prefilterCubemap, Material::UniformType::_Cubemap);
		damagedHelmetMaterial.SetUniform("brdfLUT", &lookupTexture, Material::UniformType::_Texture);
		shoeMaterial.SetUniform("irradianceMap", &irradianceCubemap, Material::UniformType::_Cubemap);
		shoeMaterial.SetUniform("prefilterMap", &prefilterCubemap, Material::UniformType::_Cubemap);
		shoeMaterial.SetUniform("brdfLUT", &lookupTexture, Material::UniformType::_Texture);

		int gltfid, objid;

		models.emplace_back();
		models.back() = new Model();
		gltfid = GltfImporter::Load("examples/pbr/gltf/SciFiHelmet/SciFiHelmet.gltf");
		models.back()->CreateFromGltf(gltfid, 0);
		ModelProcessor::ComputeTangentSpace(*models.back());
		models.back()->ReloadVertexData();
		models.back()->SetMaterial(&sciFiHelmetMaterial);

		models.emplace_back();
		models.back() = new Model();
		gltfid = GltfImporter::Load("examples/pbr/gltf/DamagedHelmet/DamagedHelmet.gltf");
		models.back()->CreateFromGltf(gltfid, 0);
		ModelProcessor::ComputeTangentSpace(*models.back());
		models.back()->ReloadVertexData();
		models.back()->SetMaterial(&damagedHelmetMaterial);

		models.emplace_back();
		models.back() = new Model();
		gltfid = GltfImporter::Load("examples/pbr/gltf/MaterialsVariantsShoe/MaterialsVariantsShoe.gltf");
		models.back()->CreateFromGltf(gltfid, 0);
		ModelProcessor::ComputeTangentSpace(*models.back());
		models.back()->ReloadVertexData();
		models.back()->SetMaterial(&shoeMaterial);

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

		targetGimbalRotation.y -= Input::MousePosDeltaX() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x += Input::MousePosDeltaY() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

		gimbal.SetRotation(glm::slerp(gimbal.GetRotation(), glm::fquat(targetGimbalRotation), deltaTime * SPEED));
		camera->SetPosition(gimbal.GetPosition() + gimbal.Forward() * cameraDistance);
		camera->LookAt(gimbal.GetPosition(), glm::vec3(0.0, 1.0, 0.0));

		if (rotationEnabled)
			models[selectedModel]->SetRotation(models[selectedModel]->GetRotation() * glm::fquat(glm::vec3(0.0f, 0.07f * deltaTime, 0.0f)));
	}
	void Game::Terminate()
	{
		for (Model* m : models)
			delete m;
	}
}