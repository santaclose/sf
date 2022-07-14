#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>

#include <Game.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Input.h>

#include <Renderer/Renderer.h>

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

	int selectedEnvironment = 0;
	std::vector<std::string> environments = { "examples/pbr/brown_photostudio_02_4k.hdr", "examples/pbr/aft_lounge_4k.hdr", };

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
			"https://dl.polyhaven.org/file/ph-assets/HDRIs/hdr/4k/brown_photostudio_02_4k.hdr",
			"https://dl.polyhaven.org/file/ph-assets/HDRIs/hdr/4k/aft_lounge_4k.hdr"
			}, "examples/pbr/");

		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();
		Renderer::activeCameraEntity = cameraObject;

		gimbal.AddComponent<Transform>();

		cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();

		Renderer::SetEnvironment(environments[selectedEnvironment]);
		Renderer::drawSkybox = true;

		int gltfid;

		meshes = new MeshData[2];
		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& e_t = galleryObjects.back().AddComponent<Transform>();
			e_t.rotation = glm::fquat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));

			gltfid = GltfImporter::Load("examples/pbr/DamagedHelmet.glb");

			Bitmap albedo, metalRoughness, normalmap, emissive, ao;
			GltfImporter::GenerateBitmap(gltfid, 0, albedo);
			GltfImporter::GenerateBitmap(gltfid, 1, metalRoughness);
			GltfImporter::GenerateBitmap(gltfid, 4, normalmap);
			GltfImporter::GenerateBitmap(gltfid, 2, emissive);
			GltfImporter::GenerateBitmap(gltfid, 3, ao);
			Bitmap metal(metalRoughness.dataType, 1, metalRoughness.width, metalRoughness.height);
			Bitmap roughness(metalRoughness.dataType, 1, metalRoughness.width, metalRoughness.height);
			roughness.CopyChannel(metalRoughness, 1, 0);
			metal.CopyChannel(metalRoughness, 2, 0);

			Material damagedHelmetMaterial("examples/pbr/DamagedHelmet.mat", false);
			damagedHelmetMaterial.uniforms["albedoTexture"] = { (uint32_t)ShaderDataType::bitmap, &albedo };
			damagedHelmetMaterial.uniforms["normalTexture"] = { (uint32_t)ShaderDataType::bitmap, &normalmap };
			damagedHelmetMaterial.uniforms["metalnessTexture"] = { (uint32_t)ShaderDataType::bitmap, &metal };
			damagedHelmetMaterial.uniforms["roughnessTexture"] = { (uint32_t)ShaderDataType::bitmap, &roughness };
			damagedHelmetMaterial.uniforms["aoTexture"] = { (uint32_t)ShaderDataType::bitmap, &ao };
			damagedHelmetMaterial.uniforms["emissiveTexture"] = { (uint32_t)ShaderDataType::bitmap, &emissive };

			GltfImporter::GenerateMeshData(gltfid, meshes[0]);
			MeshProcessor::ComputeTangentSpace(meshes[0]);
			Mesh& objectMesh = galleryObjects.back().AddComponent<Mesh>(&(meshes[0]));
			uint32_t materialId = Renderer::CreateMaterial(damagedHelmetMaterial);
			Renderer::SetMeshMaterial(objectMesh, materialId);
			galleryObjects.back().SetEnabled(true);
		}
		{
			galleryObjects.push_back(scene.CreateEntity());
			galleryObjects.back().AddComponent<Transform>();

			gltfid = GltfImporter::Load("examples/pbr/SciFiHelmet.gltf");
			GltfImporter::GenerateMeshData(gltfid, meshes[1]);

			MeshProcessor::ComputeTangentSpace(meshes[1]);
			Mesh& objectMesh = galleryObjects.back().AddComponent<Mesh>(&(meshes[1]));
			uint32_t materialId = Renderer::CreateMaterial(Material("examples/pbr/SciFiHelmet.mat", false));
			Renderer::SetMeshMaterial(objectMesh, materialId);
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
		else if (Input::KeyDown(Input::KeyCode::H))
		{
			selectedEnvironment++;
			selectedEnvironment = Math::Mod(selectedEnvironment, (int)environments.size());
			Renderer::SetEnvironment(environments[selectedEnvironment]);
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