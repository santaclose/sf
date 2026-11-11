#include <imgui.h>
#include <imnodes.h>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include <Game.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Random.h>
#include <Input.h>
#include <Defaults.h>
#include <OS.h>
#include <FileUtils.h>

#include <ImGuiController.h>
#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>
#include <Components/ScreenCoordinates.h>
#include <Components/Text.h>

#include <LOD.h>

#include "../Viewer.hpp"

#define MODEL_ROTATION_SENSITIVITY 10.0

namespace sf
{
	namespace Game
	{
		std::string openFilePath;

		uint32_t modelViewerDisplayPanelId;
		Scene scene;
		ExampleViewer modelViewer;
		Entity modelEntity;
		SkeletonData* modelSkeleton = nullptr;
		MeshData* modelMesh = nullptr;
		std::vector<const char*> animNames;
		Entity textEntity;
		char textString[128];
		float modelDistance;

		Material modelMaterial;
		Buffer::Layout skinnedModelVertexLayout = Buffer::Layout({
			Buffer::Component::Position,
			Buffer::Component::Normal,
			Buffer::Component::BoneIndices,
			Buffer::Component::BoneWeights
		});
		Buffer::Layout modelVertexLayout = Buffer::Layout({
			Buffer::Component::Position,
			Buffer::Component::Normal
		});

		glm::quat modelRotation;
		float modelRotationY;
		bool rotationEnabled;

		LODSpec lodSpec;
		LODGroup lodGroup;

		void CloseFile()
		{
			if (modelEntity)
			{
				scene.DestroyEntity(modelEntity);
				modelEntity = {};
			}

			lodSpec.Clear();
			animNames.clear();

			if (modelMesh)
			{
				Renderer::InvalidateLODGroup(lodGroup);
				Renderer::InvalidateMeshData(*modelMesh);
				lodGroup.Destroy();
				delete modelMesh;
				modelMesh = nullptr;
			}
			if (modelSkeleton)
			{
				Renderer::InvalidateSkeletonData(*modelSkeleton);
				delete modelSkeleton;
				modelSkeleton = nullptr;
			}
		}

		void OpenFile(const char* filePath)
		{
			CloseFile();

			modelEntity = scene.CreateEntity();
			modelEntity.AddComponent<Transform>();

			if (FileUtils::ExtensionIs(filePath, "obj"))
			{
				int objid = ObjImporter::Load(filePath);
				if (objid == -1)
				{
					printf("Could not load file: %s\n", filePath);
					scene.DestroyEntity(modelEntity);
					modelEntity = {};
					return;
				}
				modelMesh = new MeshData();
				ObjImporter::GenerateMeshData(objid, modelVertexLayout, *modelMesh);
			}
			else // gltf
			{
				int gltfid = GltfImporter::Load(filePath);
				if (gltfid == -1)
				{
					printf("Could not load file: %s\n", filePath);
					scene.DestroyEntity(modelEntity);
					modelEntity = {};
					return;
				}
				modelSkeleton = new SkeletonData();
				if (!GltfImporter::GenerateSkeleton(gltfid, *modelSkeleton))
				{
					delete modelSkeleton;
					modelSkeleton = nullptr;
				}
				modelMesh = new MeshData();
				GltfImporter::GenerateMeshData(gltfid, modelSkeleton == nullptr ? modelVertexLayout : skinnedModelVertexLayout, *modelMesh);
			}

			MeshProcessor::ComputeNormals(*modelMesh);

			lodSpec.Push({ 0, 1.00f, 1e-2f, 3.0f });
			lodSpec.Push({ 0, 0.40f, 1e-2f, 10.0f });
			lodSpec.Push({ 0, 0.01f, 1e-2f, 56.0f });

			lodGroup.Create(*modelMesh, lodSpec);

			if (modelSkeleton != nullptr)
			{
				MeshProcessor::RemoveUnusedBones(*modelMesh, *modelSkeleton);
				modelEntity.AddComponent<SkinnedMesh>(&lodGroup, modelSkeleton, &modelMaterial);
				animNames.resize(modelSkeleton->m_animations.size());
				for (int i = 0; i < modelSkeleton->m_animations.size(); i++)
					animNames[i] = modelSkeleton->m_animations[i].name;
			}
			else
			{
				modelEntity.AddComponent<Mesh>(&lodGroup, &modelMaterial);
			}
			openFilePath = std::string(filePath);
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

		modelViewerDisplayPanelId = ImGuiController::CreateDisplayPanel("Model Viewer", 8);
		std::vector<Renderer::RenderTarget>& renderTargets = Renderer::GetRenderTargets();
		renderTargets.back().framebuffer = ImGuiController::GetDisplayPanelFramebufferToDraw(modelViewerDisplayPanelId);

		modelMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		modelMaterial.fragShaderFilePath = "assets/shaders/default.frag";

		modelViewer.Initialize(scene, 0);

		sprintf(textString, "No LOD");
		textEntity = scene.CreateEntity();
		ScreenCoordinates& e_sc = textEntity.AddComponent<ScreenCoordinates>();
		e_sc.origin = { 1.0f, 1.0f };
		e_sc.offset = { -22.0f, -22.0f };
		Text& e_txt = textEntity.AddComponent<Text>();
		e_txt.color = { 0.0f, 0.0f, 0.0f, 1.0f };
		e_txt.fontPath = "assets/fonts/FiraCode/FiraCode-Regular.ttf";
		e_txt.string = textString;
		e_txt.size = 26.0f;
		e_txt.alignmentH = ALIGNMENT_RIGHT;
		e_txt.alignmentV = ALIGNMENT_BOTTOM;

		// OpenFile("assets/examples/BrainStem.glb");
		// OpenFile("assets/examples/rb3d/Toilet_7291921_f.glb");
	}

	void Game::Terminate()
	{
		CloseFile();
		scene.DestroyEntity(textEntity);
		modelViewer.Terminate(scene);
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (!modelEntity)
		{
			sprintf(textString, "No LOD");
			return;
		}

		Transform& modelTransform = modelEntity.GetComponent<Transform>();
		Transform& cameraTransform = modelViewer.cameraObject.GetComponent<Transform>();

		SphereCollider worldSpaceBoundingSphere;// = lodGroup.sourceMesh->boundingSphere.ApplyTransform(modelTransform);
		if (modelSkeleton != nullptr)
			worldSpaceBoundingSphere = modelEntity.GetComponent<SkinnedMesh>().GetBoundingSphere().ApplyTransform(modelEntity.GetComponent<Transform>());
		else
			worldSpaceBoundingSphere = modelEntity.GetComponent<Mesh>().GetBoundingSphere().ApplyTransform(modelEntity.GetComponent<Transform>());
		modelDistance = glm::distance(worldSpaceBoundingSphere.center, cameraTransform.position);

		uint32_t currentLod = modelSkeleton == nullptr ?
			modelEntity.GetComponent<Mesh>().GetCurrentLOD(glm::distance2(worldSpaceBoundingSphere.center, cameraTransform.position)) :
			modelEntity.GetComponent<SkinnedMesh>().GetCurrentLOD(glm::distance2(worldSpaceBoundingSphere.center, cameraTransform.position));
		if (currentLod >= lodSpec.Count())
			sprintf(textString, "Culled\nDistance: %f", modelDistance);
		else
			sprintf(textString, "LOD: %u\nDistance: %f", currentLod, modelDistance);

		if (ImGuiController::GetActiveDisplayPanel() == modelViewerDisplayPanelId)
		{
			modelViewer.UpdateCameraInput(deltaTime);

			modelRotationY += Input::MousePosDeltaX() * MOUSE_SENSITIVITY * MODEL_ROTATION_SENSITIVITY * (Input::MouseButton(0) ? 1.0f : 0.0f);
			modelRotationY = glm::mix(modelRotationY, 0.0f, deltaTime * 7.0f);
			modelRotation = glm::quat(glm::vec3(0.0f, modelRotationY * deltaTime, 0.0f));
			if (rotationEnabled)
				modelRotation *= glm::quat(glm::vec3(0.0f, 1.5f * deltaTime, 0.0f));
			modelTransform.rotation = modelRotation * modelTransform.rotation;

			if (Input::MouseButtonDown(2) || Input::MouseButtonDown(0))
				ImGuiController::LockActiveDisplayPanel(modelViewerDisplayPanelId);
			else if (Input::MouseButtonUp(2) || Input::MouseButtonDown(0))
				ImGuiController::UnlockActiveDisplayPanel();
		}
		modelViewer.UpdateCamera(deltaTime, false);

		if (modelSkeleton != nullptr)
			modelSkeleton->UpdateAnimation(deltaTime);
	}

	void Game::ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
					std::vector<std::string> selection = OS::OpenFileDialog("*.glb *.gltf *.obj");
					if (selection.size() > 0)
					{
						OpenFile(selection[0].c_str());
					}
				}
				if (ImGui::MenuItem("open toilet"))
				{
					// OpenFile("assets");
					OpenFile("assets/examples/rb3d/Toilet_7291921_f.glb");
				}
				if (ImGui::MenuItem("open robot"))
				{
					// OpenFile("assets");
					// OpenFile("assets/examples/BrainStem.glb");
					OpenFile("assets/examples/rb3d/WashingMachine_9800066_f.glb");
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (modelEntity)
		{
			ImGui::Begin("Model");

			bool needToRegenerate = false;
			for (uint32_t i = 0; i < lodSpec.Count(); i++)
			{
				ImGui::PushID(i);
				ImGui::Text("LOD %u\n", i);
				needToRegenerate |= ImGui::SliderInt("Remesh res", (int*) &lodSpec.items[i].remeshRes, 0, 256);
				needToRegenerate |= ImGui::SliderFloat("Ratio", &lodSpec.items[i].ratio, 0.001f, 1.0f);
				needToRegenerate |= ImGui::SliderFloat("Error", &lodSpec.items[i].error, 1e-5f, 0.5f);
				needToRegenerate |= ImGui::DragFloat("Max distance", &lodSpec.items[i].maxDistance, 0.01f);
				uint32_t triangles = lodGroup.GetMeshDataForLOD(i)->indexCount / 3;
				ImGui::Text("Triangle count: %u\n", triangles);
				ImGui::Separator();
				ImGui::PopID();
			}
			if (ImGui::Button("Add"))
			{
				needToRegenerate = true;
				lodSpec.Push({ 0, lodSpec.items.back().ratio, 1e-2f, lodSpec.items.back().maxDistance * 2.0f });
			}
			if (ImGui::Button("Save with model"))
			{
				std::string targetFilePath = FileUtils::RemoveExtension(openFilePath);
				targetFilePath += std::string(".lod");

				if (lodSpec.SaveToFile(targetFilePath.c_str()))
					OS::ShowNotification("File saved");
				else
					OS::ShowNotification("Couldn't save file");
			}
			if (needToRegenerate)
			{
				if (modelSkeleton == nullptr)
					modelEntity.RemoveComponent<Mesh>();
				else
					modelEntity.RemoveComponent<SkinnedMesh>();

				Renderer::InvalidateLODGroup(lodGroup);
				Renderer::InvalidateMeshData(*modelMesh);
				lodGroup.Destroy();
				lodGroup.Create(*modelMesh, lodSpec);

				if (modelSkeleton == nullptr)
					modelEntity.AddComponent<Mesh>(&lodGroup, &modelMaterial);
				else
					modelEntity.AddComponent<SkinnedMesh>(&lodGroup, modelSkeleton, &modelMaterial);
			}

			if (modelSkeleton != nullptr)
			{
				ImGui::Checkbox("Animate", &modelSkeleton->m_animate);

				if (ImGui::CollapsingHeader("Add single animation node"))
				{
					static int animSelection;
					ImGui::PushID("singleSection");
					ImGui::Combo("Animation", &animSelection, animNames.data(), animNames.size());
					if (ImGui::Button("Add single animation node"))
						modelSkeleton->AddNodeSingle(animNames[animSelection], 1.0f);
					ImGui::PopID();
				}

				ImGui::Text("Node count: %u", modelSkeleton->m_nodes.size());
				if (ImGui::CollapsingHeader("Existing nodes"))
				{
					for (int i = 0; i < modelSkeleton->m_nodes.size(); i++)
					{
						ImGui::PushID(i);
						switch (modelSkeleton->m_nodes[i].single.type)
						{
						case Animation::NodeType::Single:
							if (ImGui::CollapsingHeader(modelSkeleton->m_nodes[i].single.animation->name))
							{
								ImGui::Text("%s:", modelSkeleton->m_nodes[i].single.animation->name);
								ImGui::DragFloat("Weight", &modelSkeleton->m_nodes[i].single.weight, 0.01f);
								ImGui::DragFloat("Speed", &modelSkeleton->m_nodes[i].single.speed, 0.01f);
							}
							break;
						}
						ImGui::PopID();
					}
				}
			}
			ImGui::End();
		}

		modelViewer.ImGuiCall("Model viewer");
	}
}