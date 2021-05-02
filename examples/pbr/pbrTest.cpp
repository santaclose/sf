#include <GLFW/glfw3.h>
#include <iostream>

#include <Game.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Skybox.h>
#include <Input.h>

#include <Renderer/Renderer.h>
#include <Renderer/Texture.h>
#include <Renderer/Cubemap.h>
#include <Renderer/Material.h>
#include <Renderer/IblHelper.h>

#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>

#define MOVE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.06
#define SPEED 15.0f
#define MODEL_OFFSET 50.0

namespace sf
{
	Scene scene;
	Entity gimbal, cameraObject;
	std::vector<Entity> meshObjects;

	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);

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

	int selectedModel = 0;

	void Game::Initialize(int argc, char** argv)
	{
		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();
		Renderer::activeCameraEntity = cameraObject;

		gimbal.AddComponent<Transform>();

		cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();

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
		
		Skybox::SetCubemap(&envCubemap);

		int gltfid, objid;

		{
			meshObjects.push_back(scene.CreateEntity());
			meshObjects.back().AddComponent<Transform>();
			Mesh& objectMesh = meshObjects.back().AddComponent<Mesh>();
			gltfid = GltfImporter::Load("examples/pbr/gltf/SciFiHelmet/SciFiHelmet.gltf");
			GltfImporter::GetMesh(gltfid, objectMesh);
			MeshProcessor::ComputeTangentSpace(objectMesh);
			objectMesh.vertexReloadPending = true;
			objectMesh.SetMaterial(&sciFiHelmetMaterial);
			meshObjects.back().SetEnabled(true);
		}

		{
			meshObjects.push_back(scene.CreateEntity());
			meshObjects.back().AddComponent<Transform>();
			Mesh& objectMesh = meshObjects.back().AddComponent<Mesh>();
			gltfid = GltfImporter::Load("examples/pbr/gltf/DamagedHelmet/DamagedHelmet.gltf");
			GltfImporter::GetMesh(gltfid, objectMesh);
			MeshProcessor::ComputeTangentSpace(objectMesh);
			objectMesh.vertexReloadPending = true;
			objectMesh.SetMaterial(&damagedHelmetMaterial);
			meshObjects.back().SetEnabled(false);
		}

		{
			meshObjects.push_back(scene.CreateEntity());
			meshObjects.back().AddComponent<Transform>();
			Mesh& objectMesh = meshObjects.back().AddComponent<Mesh>();
			gltfid = GltfImporter::Load("examples/pbr/gltf/MaterialsVariantsShoe/MaterialsVariantsShoe.gltf");
			GltfImporter::GetMesh(gltfid, objectMesh);
			MeshProcessor::ComputeTangentSpace(objectMesh);
			objectMesh.vertexReloadPending = true;
			objectMesh.SetMaterial(&shoeMaterial);
			meshObjects.back().SetEnabled(false);
		}

		gimbal.GetComponent<Transform>().SetPosition(glm::vec3(0.0, 0.0, 0.0));
		gimbal.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		cameraObject.GetComponent<Transform>().SetPosition(glm::vec3(0.0, 0.0, cameraDistance));
		cameraObject.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
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
			int prevModel = selectedModel;
			selectedModel++;
			selectedModel = Math::Mod(selectedModel, (int)meshObjects.size());
			meshObjects[prevModel].SetEnabled(false);
			meshObjects[selectedModel].SetEnabled(true);
		}
		else if (Input::KeyDown(Input::KeyCode::Left))
		{
			int prevModel = selectedModel;
			selectedModel--;
			selectedModel = Math::Mod(selectedModel, (int)meshObjects.size());
			meshObjects[prevModel].SetEnabled(false);
			meshObjects[selectedModel].SetEnabled(true);
		}
		cameraDistance -= SCROLL_SENSITIVITY * (Input::MouseScrollUp() ? 1.0f : 0.0f);
		cameraDistance += SCROLL_SENSITIVITY * (Input::MouseScrollDown() ? 1.0f : 0.0f);

		targetGimbalRotation.y -= Input::MousePosDeltaX() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x += Input::MousePosDeltaY() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

		gimbal.GetComponent<Transform>().SetRotation(glm::slerp(gimbal.GetComponent<Transform>().GetRotation(), glm::fquat(targetGimbalRotation), deltaTime* SPEED));
		cameraObject.GetComponent<Transform>().SetPosition(gimbal.GetComponent<Transform>().GetPosition() + gimbal.GetComponent<Transform>().Forward() * cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(gimbal.GetComponent<Transform>().GetPosition(), glm::vec3(0.0, 1.0, 0.0));

		if (rotationEnabled)
		{
			Transform& objectTransform = meshObjects[selectedModel].GetComponent<Transform>();
			objectTransform.SetRotation(objectTransform.GetRotation() * glm::fquat(glm::vec3(0.0f, 0.07f * deltaTime, 0.0f)));
		}
	}
	void Game::Terminate()
	{
	}
}