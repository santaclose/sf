#include <imgui.h>
#include <iostream>

#include <Game.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Random.h>
#include <Input.h>
#include <Defaults.h>

#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <VoxelVolumeData.h>
#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>

#include "../Viewer.hpp"

#define MODEL_ROTATION_SENSITIVITY 10.0
#define MODEL_OFFSET 50.0

namespace sf
{
	namespace Game
	{
		Scene scene;
		std::vector<Entity> galleryObjects;

		glm::quat modelRotation;
		float modelRotationY;

		bool rotationEnabled;

		BufferLayout voxelLayout = BufferLayout({BufferComponent::Position});
		VoxelVolumeData monkevbd;
		VoxelVolumeData monkevbd2;

		int selectedModel;

		Material meshMaterial;
		Material voxelVolumeMaterial;

		void GalleryChange(bool next)
		{
			int prevModel = selectedModel;
			selectedModel += next ? 1 : -1;
			selectedModel = Math::Mod(selectedModel, (int)galleryObjects.size());
			galleryObjects[prevModel].SetEnabled(false);
			galleryObjects[selectedModel].SetEnabled(true);

			voxelVolumeMaterial.uniforms["bufferSelect"].data.u32 = selectedModel;
			voxelVolumeMaterial.uniforms["voxelSize"].data.f32 = selectedModel == 0 ? monkevbd.voxelSize : monkevbd2.voxelSize;
		}
	}

	Game::InitData Game::GetInitData()
	{
		return InitData();
	}

	void Game::Initialize(int argc, char** argv)
	{
		modelRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		modelRotationY = 0.0f;
		rotationEnabled = false;
		selectedModel = 0;

		ExampleViewer::Initialize(scene);

		monkevbd.BuildFromMesh(Defaults::MeshDataMonkey(), 0.007f, &voxelLayout);
		monkevbd2.BuildFromMesh(Defaults::MeshDataMonkey(), 0.02f, &voxelLayout);

		meshMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		meshMaterial.fragShaderFilePath = "assets/shaders/default.frag";
		voxelVolumeMaterial.vertShaderFilePath = "assets/shaders/voxelVolume.vert";
		voxelVolumeMaterial.fragShaderFilePath = "assets/shaders/uv.frag";
		voxelVolumeMaterial.buffers.resize(2);
		voxelVolumeMaterial.buffers[0] = { "VOXELS_SMALL", monkevbd.voxelBuffer.data(), &voxelLayout, (uint32_t) monkevbd.voxelBuffer.size(), DataType::f32 };
		voxelVolumeMaterial.buffers[1] = { "VOXELS_BIG", monkevbd2.voxelBuffer.data(), &voxelLayout, (uint32_t) monkevbd2.voxelBuffer.size(), DataType::f32 };
		voxelVolumeMaterial.uniforms["voxelSize"].dataType = DataType::f32;
		voxelVolumeMaterial.uniforms["voxelSize"].data.f32 = monkevbd.voxelSize;
		voxelVolumeMaterial.uniforms["bufferSelect"].dataType = DataType::u32;
		voxelVolumeMaterial.uniforms["bufferSelect"].data.u32 = selectedModel;

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			ParticleSystem& objectParticles = galleryObjects.back().AddComponent<ParticleSystem>();
			objectParticles.dynamic = false;
			objectParticles.meshData = &Defaults::MeshDataCube();
			objectParticles.material = &voxelVolumeMaterial;
			objectParticles.particleCount = monkevbd.GetVoxelCount();
		}

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			ParticleSystem& objectParticles = galleryObjects.back().AddComponent<ParticleSystem>();
			objectParticles.dynamic = false;
			objectParticles.meshData = &Defaults::MeshDataCube();
			objectParticles.material = &voxelVolumeMaterial;
			objectParticles.particleCount = monkevbd2.GetVoxelCount();
		}

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			Mesh& objectMesh = galleryObjects.back().AddComponent<Mesh>(&Defaults::MeshDataMonkey(), &meshMaterial);
		}

		for (int i = 0; i < galleryObjects.size(); i++)
			galleryObjects[i].SetEnabled(i == selectedModel);
	}

	void Game::Terminate()
	{
		for (Entity e : galleryObjects)
			scene.DestroyEntity(e);
		galleryObjects.clear();
		ExampleViewer::Terminate(scene);
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
			ImGui::EndMainMenuBar();
		}
		ExampleViewer::ImGuiCall();
	}
}