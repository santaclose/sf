#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>

#include <Game.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Skybox.h>
#include <Input.h>

#include <Renderer/Renderer.h>
#include <Renderer/GlTexture.h>
#include <Renderer/GlCubemap.h>
#include <Renderer/GlMaterial.h>
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
	std::vector<Entity> galleryObjects;

	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);

	float cameraDistance = 3.0;
	bool rotationEnabled = false;

	GlShader pbrShader;

	GlMaterial sciFiHelmetMaterial;
	GlTexture sciFiHelmetAlbedo;
	GlTexture sciFiHelmetNormalmap;
	GlTexture sciFiHelmetMetalRoughness;
	GlTexture sciFiHelmetRoughness;
	GlTexture sciFiHelmetMetallic;
	GlTexture sciFiHelmetAO;

	GlMaterial damagedHelmetMaterial;
	GlTexture damagedHelmetAlbedo;
	GlTexture damagedHelmetNormalmap;
	GlTexture damagedHelmetMetalRoughness;
	GlTexture damagedHelmetRoughness;
	GlTexture damagedHelmetMetallic;
	GlTexture damagedHelmetEmissive;
	GlTexture damagedHelmetAO;

	GlTexture envTexture;
	GlCubemap envCubemap;
	GlCubemap irradianceCubemap;
	GlCubemap prefilterCubemap;
	GlTexture lookupTexture;

	MeshData* meshes;

	int selectedModel = 0;

	void DownloadAssetDependencies(const std::vector<std::string>& urls, const std::string& targetPath)
	{
		for (const std::string& url : urls)
		{
			const std::string fileName = url.substr(url.find_last_of('/') + 1);
			std::ifstream f(targetPath + fileName);
			if (!f.good()) // download if file doesn't exist
			{
				std::string commandString = "curl " + url + " --output " + targetPath + fileName;
				system(commandString.c_str());
			}

		}
	}

	void Game::Initialize(int argc, char** argv)
	{
		DownloadAssetDependencies({
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/DamagedHelmet/glTF-Binary/DamagedHelmet.glb",
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/SciFiHelmet/glTF/SciFiHelmet.gltf",
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/SciFiHelmet/glTF/SciFiHelmet.bin",
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/SciFiHelmet/glTF/SciFiHelmet_AmbientOcclusion.png",
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/SciFiHelmet/glTF/SciFiHelmet_BaseColor.png",
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/SciFiHelmet/glTF/SciFiHelmet_MetallicRoughness.png",
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/SciFiHelmet/glTF/SciFiHelmet_Normal.png",
			"https://dl.polyhaven.org/file/ph-assets/HDRIs/hdr/4k/brown_photostudio_02_4k.hdr"
			}, "examples/pbr/");

		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();
		Renderer::activeCameraEntity = cameraObject;

		gimbal.AddComponent<Transform>();

		cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();

		pbrShader.CreateFromFiles("assets/shaders/pbrV.shader", "assets/shaders/pbrF.shader");

		damagedHelmetMaterial.CreateFromShader(&pbrShader);
		sciFiHelmetMaterial.CreateFromShader(&pbrShader);

		envTexture.CreateFromFile("examples/pbr/brown_photostudio_02_4k.hdr", 3, GlTexture::Float16, GlTexture::ClampToEdge);
		IblHelper::HdrToCubemaps(envTexture, envCubemap, irradianceCubemap, prefilterCubemap, lookupTexture);

		damagedHelmetMaterial.SetUniform("irradianceMap", &irradianceCubemap, GlMaterial::UniformType::_Cubemap);
		damagedHelmetMaterial.SetUniform("prefilterMap", &prefilterCubemap, GlMaterial::UniformType::_Cubemap);
		damagedHelmetMaterial.SetUniform("brdfLUT", &lookupTexture, GlMaterial::UniformType::_Texture);
		sciFiHelmetMaterial.SetUniform("irradianceMap", &irradianceCubemap, GlMaterial::UniformType::_Cubemap);
		sciFiHelmetMaterial.SetUniform("prefilterMap", &prefilterCubemap, GlMaterial::UniformType::_Cubemap);
		sciFiHelmetMaterial.SetUniform("brdfLUT", &lookupTexture, GlMaterial::UniformType::_Texture);

		Skybox::SetCubemap(&envCubemap);
		Renderer::drawSkybox = true;

		int gltfid;

		meshes = new MeshData[2];
		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& e_t = galleryObjects.back().AddComponent<Transform>();
			e_t.rotation = glm::fquat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));

			gltfid = GltfImporter::Load("examples/pbr/DamagedHelmet.glb");
			GltfImporter::GenerateMeshData(gltfid, meshes[0]);
			GltfImporter::GenerateTexture(gltfid, 0, damagedHelmetAlbedo);
			GltfImporter::GenerateTexture(gltfid, 1, damagedHelmetMetalRoughness);
			GltfImporter::GenerateTexture(gltfid, 4, damagedHelmetNormalmap);
			GltfImporter::GenerateTexture(gltfid, 2, damagedHelmetEmissive);
			GltfImporter::GenerateTexture(gltfid, 3, damagedHelmetAO);
			damagedHelmetRoughness.CreateFromChannel(damagedHelmetMetalRoughness, 1);
			damagedHelmetMetallic.CreateFromChannel(damagedHelmetMetalRoughness, 2);

			damagedHelmetMaterial.SetUniform("useAlbedoTexture", (void*)true, GlMaterial::UniformType::_Boolean);
			damagedHelmetMaterial.SetUniform("albedoTexture", &damagedHelmetAlbedo, GlMaterial::UniformType::_Texture);
			damagedHelmetMaterial.SetUniform("useRoughnessTexture", (void*)true, GlMaterial::UniformType::_Boolean);
			damagedHelmetMaterial.SetUniform("roughnessTexture", &damagedHelmetRoughness, GlMaterial::UniformType::_Texture);
			damagedHelmetMaterial.SetUniform("useMetalnessTexture", (void*)true, GlMaterial::UniformType::_Boolean);
			damagedHelmetMaterial.SetUniform("metalnessTexture", &damagedHelmetMetallic, GlMaterial::UniformType::_Texture);
			damagedHelmetMaterial.SetUniform("useNormalTexture", (void*)true, GlMaterial::UniformType::_Boolean);
			damagedHelmetMaterial.SetUniform("normalTexture", &damagedHelmetNormalmap, GlMaterial::UniformType::_Texture);
			damagedHelmetMaterial.SetUniform("useEmissiveTexture", (void*)true, GlMaterial::UniformType::_Boolean);
			damagedHelmetMaterial.SetUniform("emissiveTexture", &damagedHelmetEmissive, GlMaterial::UniformType::_Texture);
			damagedHelmetMaterial.SetUniform("useAoTexture", (void*)true, GlMaterial::UniformType::_Boolean);
			damagedHelmetMaterial.SetUniform("aoTexture", &damagedHelmetAO, GlMaterial::UniformType::_Texture);

			MeshProcessor::ComputeTangentSpace(meshes[0]);
			Mesh& objectMesh = galleryObjects.back().AddComponent<Mesh>(&(meshes[0]));
			Renderer::SetMeshMaterial(objectMesh, &damagedHelmetMaterial);
			galleryObjects.back().SetEnabled(true);
		}
		{
			galleryObjects.push_back(scene.CreateEntity());
			galleryObjects.back().AddComponent<Transform>();

			gltfid = GltfImporter::Load("examples/pbr/SciFiHelmet.gltf");
			GltfImporter::GenerateMeshData(gltfid, meshes[1]);

			sciFiHelmetAlbedo.CreateFromFile("examples/pbr/SciFiHelmet_BaseColor.png", 3, GlTexture::UnsignedByte);
			sciFiHelmetNormalmap.CreateFromFile("examples/pbr/SciFiHelmet_Normal.png", 3, GlTexture::UnsignedByte);
			sciFiHelmetMetalRoughness.CreateFromFile("examples/pbr/SciFiHelmet_MetallicRoughness.png", 3, GlTexture::UnsignedByte);
			sciFiHelmetRoughness.CreateFromChannel(sciFiHelmetMetalRoughness, 1);
			sciFiHelmetMetallic.CreateFromChannel(sciFiHelmetMetalRoughness, 2);
			sciFiHelmetAO.CreateFromFile("examples/pbr/SciFiHelmet_AmbientOcclusion.png", 3, GlTexture::UnsignedByte);

			sciFiHelmetMaterial.SetUniform("useAlbedoTexture", (void*)true, GlMaterial::UniformType::_Boolean);
			sciFiHelmetMaterial.SetUniform("albedoTexture", &sciFiHelmetAlbedo, GlMaterial::UniformType::_Texture);
			sciFiHelmetMaterial.SetUniform("useRoughnessTexture", (void*)true, GlMaterial::UniformType::_Boolean);
			sciFiHelmetMaterial.SetUniform("roughnessTexture", &sciFiHelmetRoughness, GlMaterial::UniformType::_Texture);
			sciFiHelmetMaterial.SetUniform("useMetalnessTexture", (void*)true, GlMaterial::UniformType::_Boolean);
			sciFiHelmetMaterial.SetUniform("metalnessTexture", &sciFiHelmetMetallic, GlMaterial::UniformType::_Texture);
			sciFiHelmetMaterial.SetUniform("useNormalTexture", (void*)true, GlMaterial::UniformType::_Boolean);
			sciFiHelmetMaterial.SetUniform("normalTexture", &sciFiHelmetNormalmap, GlMaterial::UniformType::_Texture);
			sciFiHelmetMaterial.SetUniform("useAoTexture", (void*)true, GlMaterial::UniformType::_Boolean);
			sciFiHelmetMaterial.SetUniform("aoTexture", &sciFiHelmetAO, GlMaterial::UniformType::_Texture);
			sciFiHelmetMaterial.SetUniform("useEmissiveTexture", (void*)false, GlMaterial::UniformType::_Boolean);

			MeshProcessor::ComputeTangentSpace(meshes[1]);
			Mesh& objectMesh = galleryObjects.back().AddComponent<Mesh>(&(meshes[1]));
			Renderer::SetMeshMaterial(objectMesh, &sciFiHelmetMaterial);
			galleryObjects.back().SetEnabled(false);
		}

		gimbal.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 0.0);
		gimbal.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		cameraObject.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (Input::KeyDown(Input::KeyCode::Space))
			rotationEnabled = !rotationEnabled;
		else if (Input::KeyDown(Input::KeyCode::Right))
		{
			int prevModel = selectedModel;
			selectedModel++;
			selectedModel = Math::Mod(selectedModel, (int)galleryObjects.size());
			galleryObjects[prevModel].SetEnabled(false);
			galleryObjects[selectedModel].SetEnabled(true);
		}
		else if (Input::KeyDown(Input::KeyCode::Left))
		{
			int prevModel = selectedModel;
			selectedModel--;
			selectedModel = Math::Mod(selectedModel, (int)galleryObjects.size());
			galleryObjects[prevModel].SetEnabled(false);
			galleryObjects[selectedModel].SetEnabled(true);
		}
		cameraDistance -= SCROLL_SENSITIVITY * (Input::MouseScrollUp() ? 1.0f : 0.0f);
		cameraDistance += SCROLL_SENSITIVITY * (Input::MouseScrollDown() ? 1.0f : 0.0f);

		targetGimbalRotation.y -= Input::MousePosDeltaX() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x += Input::MousePosDeltaY() * MOVE_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

		gimbal.GetComponent<Transform>().rotation = glm::slerp(gimbal.GetComponent<Transform>().rotation, glm::fquat(targetGimbalRotation), deltaTime * SPEED);
		cameraObject.GetComponent<Transform>().position = glm::vec3(gimbal.GetComponent<Transform>().position + gimbal.GetComponent<Transform>().Forward() * cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(gimbal.GetComponent<Transform>().position, glm::vec3(0.0, 1.0, 0.0));

		if (rotationEnabled)
		{
			Transform& objectTransform = galleryObjects[selectedModel].GetComponent<Transform>();
			objectTransform.rotation = glm::fquat(glm::vec3(0.0f, 0.07f * deltaTime, 0.0f)) * objectTransform.rotation;
		}
	}
	void Game::Terminate()
	{
		delete[] meshes;
	}
}