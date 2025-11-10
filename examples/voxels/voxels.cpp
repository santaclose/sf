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

#include <SparseVoxelOctree.h>
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
		VoxelVolumeData monkevvd;
		VoxelVolumeData monkevvd2;
		SparseVoxelOctree monkesvo;

		int selectedModel;

		Material meshMaterial;
		Material voxelVolumeMaterial;
		Material marchingCubesMaterial;
		uint32_t marchingCubesThreadsNeeded;

		void GalleryChange(bool next)
		{
			int prevModel = selectedModel;
			selectedModel += next ? 1 : -1;
			selectedModel = Math::Mod(selectedModel, (int)galleryObjects.size());
			galleryObjects[prevModel].SetEnabled(false);
			galleryObjects[selectedModel].SetEnabled(true);

			voxelVolumeMaterial.uniforms["bufferSelect"].data.u32 = selectedModel;
			voxelVolumeMaterial.uniforms["voxelSize"].data.f32 = selectedModel == 0 ? monkevvd.voxelSize : monkevvd2.voxelSize;
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

		monkevvd.BuildFromMesh(Defaults::MeshDataMonkey(), 0.007f, &voxelLayout);
		monkevvd2.BuildFromMesh(Defaults::MeshDataMonkey(), 0.04f, &voxelLayout);
		monkesvo.CreateFromVoxelVolumeData(monkevvd2);

		meshMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		meshMaterial.fragShaderFilePath = "assets/shaders/default.frag";
		voxelVolumeMaterial.vertShaderFilePath = "assets/shaders/voxelVolume.vert";
		voxelVolumeMaterial.fragShaderFilePath = "assets/shaders/uv.frag";
		voxelVolumeMaterial.buffers.resize(2);
		voxelVolumeMaterial.buffers[0] = { "VOXELS_SMALL", monkevvd.voxelBuffer.data(), &voxelLayout, (uint32_t) monkevvd.voxelBuffer.size(), DataType::f32 };
		voxelVolumeMaterial.buffers[1] = { "VOXELS_BIG", monkevvd2.voxelBuffer.data(), &voxelLayout, (uint32_t) monkevvd2.voxelBuffer.size(), DataType::f32 };
		voxelVolumeMaterial.uniforms["voxelSize"].dataType = DataType::f32;
		voxelVolumeMaterial.uniforms["voxelSize"].data.f32 = monkevvd.voxelSize;
		voxelVolumeMaterial.uniforms["bufferSelect"].dataType = DataType::u32;
		voxelVolumeMaterial.uniforms["bufferSelect"].data.u32 = selectedModel;

		/* remove faces on most positive side */
		marchingCubesThreadsNeeded = monkevvd2.voxelCountPerAxis.x - 2 + (monkevvd2.voxelCountPerAxis.y - 2) * monkevvd2.voxelCountPerAxis.x + (monkevvd2.voxelCountPerAxis.z - 2) * monkevvd2.voxelCountPerAxis.x * monkevvd2.voxelCountPerAxis.y;
		marchingCubesMaterial.drawMode = MaterialDrawMode::Lines;
		marchingCubesMaterial.meshShaderFilePath = "examples/voxels/mc.mesh";
		marchingCubesMaterial.fragShaderFilePath = "assets/shaders/solidColor.frag";
		marchingCubesMaterial.meshWorkGroupCount = marchingCubesThreadsNeeded / 16 + 1;
		marchingCubesMaterial.buffers.resize(1);
		marchingCubesMaterial.buffers[0] = { "SVO_BUFFER", monkesvo.data.data(), nullptr, (uint32_t) (monkesvo.data.size() * sizeof(monkesvo.data[0])), DataType::u32 };
		marchingCubesMaterial.uniforms["threadsNeeded"].dataType = DataType::u32;
		marchingCubesMaterial.uniforms["threadsNeeded"].data.u32 = marchingCubesThreadsNeeded;
		marchingCubesMaterial.uniforms["svoDepth"].dataType = DataType::u32;
		marchingCubesMaterial.uniforms["svoDepth"].data.u32 = monkesvo.depth;
		marchingCubesMaterial.uniforms["voxelCountPerAxis"].dataType = DataType::vec3u32;
		marchingCubesMaterial.uniforms["voxelCountPerAxis"].data.p = &monkevvd2.voxelCountPerAxis;
		marchingCubesMaterial.uniforms["voxelSize"].dataType = DataType::f32;
		marchingCubesMaterial.uniforms["voxelSize"].data.f32 = monkevvd2.voxelSize;
		marchingCubesMaterial.uniforms["offset"].dataType = DataType::vec3f32;
		marchingCubesMaterial.uniforms["offset"].data.p = &monkevvd.offset;

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			ParticleSystem& objectParticles = galleryObjects.back().AddComponent<ParticleSystem>();
			objectParticles.dynamic = false;
			objectParticles.meshData = &Defaults::MeshDataCube();
			objectParticles.material = &voxelVolumeMaterial;
			objectParticles.particleCount = monkevvd.GetVoxelCount();
		}

		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& objectTransform = galleryObjects.back().AddComponent<Transform>();
			ParticleSystem& objectParticles = galleryObjects.back().AddComponent<ParticleSystem>();
			objectParticles.dynamic = false;
			objectParticles.meshData = &Defaults::MeshDataCube();
			objectParticles.material = &voxelVolumeMaterial;
			objectParticles.particleCount = monkevvd2.GetVoxelCount();
		}

		{
			galleryObjects.push_back(scene.CreateEntity());
			galleryObjects.back().AddComponent<Mesh>(nullptr, &marchingCubesMaterial);
			galleryObjects.back().AddComponent<Transform>();
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