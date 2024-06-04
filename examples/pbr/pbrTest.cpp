#include <imgui.h>
#include <iostream>
#include <fstream>

#include <Game.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Input.h>
#include <FileUtils.h>

#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>

#define MOUSE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.12
#define MODEL_ROTATION_SENSITIVITY 10.0
#define PAN_SENSITIVITY 0.1f
#define GIMBAL_MOVEMENT_SPEED 7.5

#define GIMBAL_ROTATION_SPEED 15.0f
#define MODEL_OFFSET 50.0
#define MIN_CAMERA_DISTANCE 0.5f

namespace sf
{
	std::string Game::ConfigFilePath = "examples/pbr/config.json";

	Scene scene;
	Entity gimbal, cameraObject;
	std::vector<Entity> galleryObjects;

	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);

	glm::quat modelRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	float modelRotationY = 0.0f;

	float cameraDistance = 3.0;
	bool rotationEnabled = false;

	int selectedEnvironment = 0;
	std::vector<std::string> environments = { "examples/pbr/brown_photostudio_02_4k.hdr", "examples/pbr/aft_lounge_4k.hdr", };

	MeshData* meshes;

	int selectedModel = 0;

	void Game::Initialize(int argc, char** argv)
	{
		FileUtils::DownloadFiles({
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

		gimbal.AddComponent<Transform>();

		cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();

		Renderer::SetEnvironment(environments[selectedEnvironment]);

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
		}

		for (int i = 0; i < galleryObjects.size(); i++)
			galleryObjects[i].SetEnabled(i == selectedModel);

		gimbal.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, 0.0);
		gimbal.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		cameraObject.GetComponent<Transform>().position = glm::vec3(0.0, 0.0, cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	}

	void UpdateCamera(float deltaTime)
	{
		if (Input::Key(Input::KeyCode::F))
			gimbal.GetComponent<Transform>().position = glm::vec3(0.0f);
		if (Input::Key(Input::KeyCode::E))
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Up() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::Q))
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Up() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::W))
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Forward() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::S))
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Forward() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::D))
			gimbal.GetComponent<Transform>().position += cameraObject.GetComponent<Transform>().Right() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;
		if (Input::Key(Input::KeyCode::A))
			gimbal.GetComponent<Transform>().position -= cameraObject.GetComponent<Transform>().Right() * GIMBAL_MOVEMENT_SPEED * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * deltaTime;

		cameraDistance -= glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollUp() ? SCROLL_SENSITIVITY : 0.0f);
		cameraDistance += glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollDown() ? SCROLL_SENSITIVITY : 0.0f);
		cameraDistance = glm::max(MIN_CAMERA_DISTANCE, cameraDistance);

		if (Input::MouseButton(2))
		{
			if (Input::Key(Input::KeyCode::LeftShift))
				gimbal.GetComponent<Transform>().position +=
				-cameraObject.GetComponent<Transform>().Right() * GIMBAL_MOVEMENT_SPEED * MOUSE_SENSITIVITY * PAN_SENSITIVITY * Input::MousePosDeltaX() * glm::sqrt(cameraDistance) +
				cameraObject.GetComponent<Transform>().Up() * GIMBAL_MOVEMENT_SPEED * MOUSE_SENSITIVITY * PAN_SENSITIVITY * Input::MousePosDeltaY() * glm::sqrt(cameraDistance);
			else
			{
				targetGimbalRotation.y -= Input::MousePosDeltaX() * MOUSE_SENSITIVITY;
				targetGimbalRotation.x += Input::MousePosDeltaY() * MOUSE_SENSITIVITY;
			}
		}

		targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

		gimbal.GetComponent<Transform>().rotation = glm::slerp(gimbal.GetComponent<Transform>().rotation, glm::quat(targetGimbalRotation), deltaTime * GIMBAL_ROTATION_SPEED);
		cameraObject.GetComponent<Transform>().position = glm::vec3(gimbal.GetComponent<Transform>().position + gimbal.GetComponent<Transform>().Forward() * cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(gimbal.GetComponent<Transform>().position, glm::vec3(0.0, 1.0, 0.0));
	}

	void ChangeEnvironment()
	{
		selectedEnvironment++;
		selectedEnvironment = Math::Mod(selectedEnvironment, (int)environments.size());
		Renderer::SetEnvironment(environments[selectedEnvironment]);
	}

	void GalleryChange(bool next)
	{
		int prevModel = selectedModel;
		selectedModel += next ? 1 : -1;
		selectedModel = Math::Mod(selectedModel, (int)galleryObjects.size());
		galleryObjects[prevModel].SetEnabled(false);
		galleryObjects[selectedModel].SetEnabled(true);
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (Input::KeyDown(Input::KeyCode::Space))
			rotationEnabled = !rotationEnabled;
		else if (Input::KeyDown(Input::KeyCode::Right))
			GalleryChange(true);
		else if (Input::KeyDown(Input::KeyCode::Left))
			GalleryChange(false);
		else if (Input::KeyDown(Input::KeyCode::H))
			ChangeEnvironment();

		UpdateCamera(deltaTime);

		modelRotationY += Input::MousePosDeltaX() * MOUSE_SENSITIVITY * MODEL_ROTATION_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
		modelRotationY = glm::mix(modelRotationY, 0.0f, deltaTime * 7.0f);
		modelRotation = glm::quat(glm::vec3(0.0f, modelRotationY * deltaTime, 0.0f));
		if (rotationEnabled)
			modelRotation *= glm::quat(glm::vec3(0.0f, 1.5f * deltaTime, 0.0f));

		Transform& objectTransform = galleryObjects[selectedModel].GetComponent<Transform>();
		objectTransform.rotation = modelRotation * objectTransform.rotation;
	}
	void Game::ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Gallery"))
			{
				if (ImGui::MenuItem("Previous", "Left arrow")) { GalleryChange(false); }
				if (ImGui::MenuItem("Next", "Right arrow")) { GalleryChange(true); }
				if (ImGui::MenuItem("Toggle rotation", "Space")) { rotationEnabled = !rotationEnabled; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Camera"))
			{
				if (ImGui::MenuItem("Center", "F"))
					gimbal.GetComponent<Transform>().position = glm::vec3(0.0f);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Environment"))
			{
				if (ImGui::MenuItem("Change", "H")) { ChangeEnvironment(); }
				ImGui::Separator();
				ImGui::Text("From file");
				static char environmentTextFieldBuffer[256];
				ImGui::InputText("Path", environmentTextFieldBuffer, 256);
				if (ImGui::Button("Load"))
					Renderer::SetEnvironment(std::string(environmentTextFieldBuffer));
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
	void Game::Terminate()
	{
		delete[] meshes;
	}
}