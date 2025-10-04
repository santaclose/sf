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
#include "../Viewer.hpp"


#define MODEL_ROTATION_SENSITIVITY 10.0
#define MODEL_OFFSET 50.0

namespace sf
{
	Scene scene;
	std::vector<Entity> galleryObjects;

	glm::quat modelRotation;
	float modelRotationY;

	bool rotationEnabled;

	int selectedEnvironment;
	std::vector<std::string> environments;

	MeshData* meshes;
	BufferLayout meshVertexLayout = BufferLayout({
		BufferComponent::VertexPosition,
		BufferComponent::VertexNormal,
		BufferComponent::VertexTangent,
		BufferComponent::VertexUV
	});

	int selectedModel;

	Material damagedHelmetMaterial;
	Bitmap damagedHelmetAlbedo, damagedHelmetNormalmap, damagedHelmetEmissive, damagedHelmetMetal, damagedHelmetRoughness, damagedHelmetAO;

	Material sciFiHelmetMaterial;

	Game::InitData Game::GetInitData()
	{
		return InitData();
	}

	void Game::Initialize(int argc, char** argv)
	{
		modelRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		modelRotationY = 0.0f;
		rotationEnabled = false;
		selectedEnvironment = 0;
		environments = { "examples/pbr/brown_photostudio_02_4k.hdr", "examples/pbr/aft_lounge_4k.hdr", };
		selectedModel = 0;

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

		ExampleViewer::Initialize(scene);
		Renderer::SetEnvironment(environments[selectedEnvironment]);

		int gltfid;

		meshes = new MeshData[2];
		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& e_t = galleryObjects.back().AddComponent<Transform>();
			e_t.rotation = glm::fquat(glm::vec3(glm::radians(90.0f), 0.0f, 0.0f));

			gltfid = GltfImporter::Load("examples/pbr/DamagedHelmet.glb");

			Bitmap tempMetalRoughness;
			GltfImporter::GenerateBitmap(gltfid, 0, damagedHelmetAlbedo);
			GltfImporter::GenerateBitmap(gltfid, 1, tempMetalRoughness);
			GltfImporter::GenerateBitmap(gltfid, 4, damagedHelmetNormalmap);
			GltfImporter::GenerateBitmap(gltfid, 2, damagedHelmetEmissive);
			GltfImporter::GenerateBitmap(gltfid, 3, damagedHelmetAO);
			damagedHelmetMetal.CreateSolid(tempMetalRoughness.dataType, 1, tempMetalRoughness.width, tempMetalRoughness.height);
			damagedHelmetRoughness.CreateSolid(tempMetalRoughness.dataType, 1, tempMetalRoughness.width, tempMetalRoughness.height);
			damagedHelmetMetal.CopyChannel(tempMetalRoughness, 2, 0);
			damagedHelmetRoughness.CopyChannel(tempMetalRoughness, 1, 0);

			damagedHelmetMaterial.CreateFromFile("examples/pbr/DamagedHelmet.mat");
			damagedHelmetMaterial.uniforms["albedoTexture"] = { DataType::bitmap, &damagedHelmetAlbedo };
			damagedHelmetMaterial.uniforms["normalTexture"] = { DataType::bitmap, &damagedHelmetNormalmap };
			damagedHelmetMaterial.uniforms["metalnessTexture"] = { DataType::bitmap, &damagedHelmetMetal };
			damagedHelmetMaterial.uniforms["roughnessTexture"] = { DataType::bitmap, &damagedHelmetRoughness };
			damagedHelmetMaterial.uniforms["aoTexture"] = { DataType::bitmap, &damagedHelmetAO };
			damagedHelmetMaterial.uniforms["emissiveTexture"] = { DataType::bitmap, &damagedHelmetEmissive };

			meshes[0] = MeshData(&meshVertexLayout);
			GltfImporter::GenerateMeshData(gltfid, meshes[0]);
			MeshProcessor::ComputeTangentSpace(meshes[0]);
			Mesh& objectMesh = galleryObjects.back().AddComponent<Mesh>(&meshes[0], &damagedHelmetMaterial);
		}
		{
			galleryObjects.push_back(scene.CreateEntity());
			galleryObjects.back().AddComponent<Transform>();

			gltfid = GltfImporter::Load("examples/pbr/SciFiHelmet.gltf");
			meshes[1] = MeshData(&meshVertexLayout);
			GltfImporter::GenerateMeshData(gltfid, meshes[1]);
			MeshProcessor::ComputeTangentSpace(meshes[1]);

			sciFiHelmetMaterial.CreateFromFile("examples/pbr/SciFiHelmet.mat");
			Mesh& objectMesh = galleryObjects.back().AddComponent<Mesh>(&meshes[1], &sciFiHelmetMaterial);
		}

		for (int i = 0; i < galleryObjects.size(); i++)
			galleryObjects[i].SetEnabled(i == selectedModel);
	}

	void Game::Terminate()
	{
		delete[] meshes;
		for (Entity e : galleryObjects)
			scene.DestroyEntity(e);
		galleryObjects.clear();
		ExampleViewer::Terminate(scene);
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
		ExampleViewer::UpdateCamera(deltaTime);

		if (Input::KeyDown(Input::KeyCode::Space))
			rotationEnabled = !rotationEnabled;
		else if (Input::KeyDown(Input::KeyCode::Right))
			GalleryChange(true);
		else if (Input::KeyDown(Input::KeyCode::Left))
			GalleryChange(false);
		else if (Input::KeyDown(Input::KeyCode::H))
			ChangeEnvironment();

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
		ExampleViewer::ImGuiCall();
	}
}