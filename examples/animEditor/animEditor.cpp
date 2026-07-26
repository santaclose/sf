#include <imgui.h>
#include <imnodes.h>
#include <iostream>
#include <unordered_map>

#include <Game.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Random.h>
#include <Input.h>
#include <Defaults.h>

#include <ImGuiController.h>
#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>

#include "../Viewer.hpp"
#include "retargetNodes.h"

namespace sf
{
	namespace Game
	{
		uint32_t inputModelDisplayPanelId, outputModelDisplayPanelId;
		Scene inputModelScene, outputModelScene;
		ExampleViewer inputModelViewer, outputModelViewer;

		Entity inputModelEntity, outputModelEntity;
		SkeletonData* inputModelSkeleton, *outputModelSkeleton;
		MeshData* inputModelMesh, *outputModelMesh;
		std::vector<const char*> inputAnimNames, outputAnimNames;

		Material modelMaterial;
		BufferLayout modelVertexLayout = BufferLayout({
			BufferComponent::Position,
			BufferComponent::Normal,
			BufferComponent::BoneIndices,
			BufferComponent::BoneWeights
		});

		Transform inputModelBaseTransform, outputModelBaseTransform;
		glm::vec3 inputModelBaseRotHelper = glm::vec3(0.0f);
		glm::vec3 outputModelBaseRotHelper = glm::vec3(0.0f);

		struct Link
		{
			int id, a, b;
		};
		std::vector<RetargetNode*> editorNodes;
		std::vector<Link> editorLinks;
		int FindNewNodeId()
		{
			for (int i = 0; i < editorNodes.size(); i++)
				if (editorNodes[i] == nullptr)
					return i;
			editorNodes.push_back(nullptr);
			return (int) (editorNodes.size() - 1);

		}
		int FindNewLinkId()
		{
			for (int i = 0; i < editorLinks.size(); i++)
				if (editorLinks[i].id == -1)
					return i;
			editorLinks.emplace_back();
			return (int) (editorLinks.size() - 1);
		}

		void OpenFile(const char* filePath, Entity& entity, SkeletonData*& sd, MeshData*& md, std::vector<const char*>& animNames, bool isInputModel)
		{
			Scene& targetScene = isInputModel ? inputModelScene : outputModelScene;

			if (entity)
				targetScene.DestroyEntity(entity);
			entity = targetScene.CreateEntity();
			entity.AddComponent<Transform>();
			sd = new SkeletonData();
			md = new MeshData(&modelVertexLayout);
			uint32_t gltfid = GltfImporter::Load(filePath);
			GltfImporter::GenerateSkeleton(gltfid, *sd);
			GltfImporter::GenerateMeshData(gltfid, *md);
			MeshProcessor::ComputeNormals(*md);
			MeshProcessor::RemoveUnusedBones(*md, *sd);
			entity.AddComponent<SkinnedMesh>(md, sd, &modelMaterial);
			if (isInputModel)
			{
				animNames.resize(sd->m_animations.size());
				for (int i = 0; i < sd->m_animations.size(); i++)
					animNames[i] = sd->m_animations[i].name;
				editorNodes.push_back(new RetargetNode());
				RetargetNodeInitialize(RetargetNodeType::InputBones, *editorNodes.back());
				editorNodes.back()->inputBones.localTransforms = &sd->m_boneLocalTransforms;
				editorNodes.back()->inputBones.boneNames = (const char**)calloc(sd->m_boneData.size(), sizeof(char*));
				for (int i = 0; i < sd->m_boneData.size(); i++)
					editorNodes.back()->inputBones.boneNames[i] = sd->m_boneData[i].name;
			}
			else // is output model file
			{
				animNames.resize(sd->m_animations.size());
				for (int i = 0; i < sd->m_animations.size(); i++)
					animNames[i] = sd->m_animations[i].name;
				editorNodes.push_back(new RetargetNode());
				RetargetNodeInitialize(RetargetNodeType::OutputBones, *editorNodes.back());
				editorNodes.back()->outputBones.localTransforms = &sd->m_boneLocalTransforms;
				editorNodes.back()->outputBones.boneNames = (const char**)calloc(sd->m_boneData.size(), sizeof(char*));
				editorNodes.back()->outputBones.inputPos = (const glm::vec3**)calloc(sd->m_boneLocalTransforms.size(), sizeof(glm::vec3*));
				editorNodes.back()->outputBones.inputRot = (const glm::quat**)calloc(sd->m_boneLocalTransforms.size(), sizeof(glm::quat*));
				editorNodes.back()->outputBones.inputScale = (const float**)calloc(sd->m_boneLocalTransforms.size(), sizeof(float*));
				for (int i = 0; i < sd->m_boneData.size(); i++)
					editorNodes.back()->outputBones.boneNames[i] = sd->m_boneData[i].name;
				return;
			}
		}

		void ExecuteNodeAndDependants(int nodeId)
		{
			static std::vector<int> activationQueue;
			activationQueue.clear();
			int currentQueueIndex = 0;
			int currentNode;
			activationQueue.push_back(nodeId);
			while (currentQueueIndex < activationQueue.size())
			{
				currentNode = activationQueue[currentQueueIndex];
				currentQueueIndex++;
				RetargetNodeRun(editorNodes[currentNode][0]);
				for (int i = 0; editorNodes[currentNode][0].inputQuat.nodesToTheRight[i] != -1; i++)
					activationQueue.push_back(editorNodes[currentNode][0].inputQuat.nodesToTheRight[i]);
			}
		}
	}

	Game::InitData Game::GetInitData()
	{
		return InitData();
	}

	void Game::Initialize(int argc, char** argv)
	{
		inputModelDisplayPanelId = ImGuiController::CreateDisplayPanel("Input Model Viewer", 8);
		outputModelDisplayPanelId = ImGuiController::CreateDisplayPanel("Output Model Viewer", 8);
		std::vector<Renderer::RenderTarget>& renderTargets = Renderer::GetRenderTargets();
		renderTargets.back().framebuffer = ImGuiController::GetDisplayPanelFramebufferToDraw(inputModelDisplayPanelId);
		renderTargets.push_back(renderTargets.back());
		renderTargets.back().framebuffer = ImGuiController::GetDisplayPanelFramebufferToDraw(outputModelDisplayPanelId);

		modelMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		modelMaterial.fragShaderFilePath = "assets/shaders/default.frag";

		inputModelViewer.Initialize(inputModelScene, 0);
		outputModelViewer.Initialize(outputModelScene, 1);

		OpenFile("assets/examples/Fox.glb", inputModelEntity, inputModelSkeleton, inputModelMesh, inputAnimNames, true);
		OpenFile("/home/san/catGameMaybe/catTextured.glb", outputModelEntity, outputModelSkeleton, outputModelMesh, outputAnimNames, false);

		inputModelBaseTransform.scale = 0.02f;
		outputModelBaseTransform.rotation = glm::quat(glm::radians(glm::vec3(-90.0f, 0.0f, 0.0f)));
		inputModelEntity.GetComponent<Transform>() = inputModelBaseTransform;
		outputModelEntity.GetComponent<Transform>() = outputModelBaseTransform;
	}

	void Game::Terminate()
	{
		outputModelScene.DestroyEntity(outputModelEntity);
		inputModelScene.DestroyEntity(inputModelEntity);
		outputModelViewer.Terminate(outputModelScene);
		inputModelViewer.Terminate(inputModelScene);
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (ImGuiController::GetActiveDisplayPanel() == inputModelDisplayPanelId)
		{
			inputModelViewer.UpdateCameraInput(deltaTime);
			if (Input::MouseButtonDown(2))
				ImGuiController::LockActiveDisplayPanel(inputModelDisplayPanelId);
			else if (Input::MouseButtonUp(2))
				ImGuiController::UnlockActiveDisplayPanel();
		}
		inputModelViewer.UpdateCamera(deltaTime, false);
		if (ImGuiController::GetActiveDisplayPanel() == outputModelDisplayPanelId)
		{
			outputModelViewer.UpdateCameraInput(deltaTime);
			if (Input::MouseButtonDown(2))
				ImGuiController::LockActiveDisplayPanel(outputModelDisplayPanelId);
			else if (Input::MouseButtonUp(2))
				ImGuiController::UnlockActiveDisplayPanel();
		}
		outputModelViewer.UpdateCamera(deltaTime, false);

		if (inputModelEntity)
			inputModelSkeleton->UpdateAnimation(deltaTime);
		if (inputModelEntity && outputModelEntity)
		{
			outputModelSkeleton->UpdateAnimation(deltaTime);
			ExecuteNodeAndDependants(0); // zero is input bones node
			for (int i = 0; i < editorNodes.size(); i++)
				if (editorNodes[i] != nullptr && editorNodes[i]->inputQuat.type == RetargetNodeType::InputQuat)
					ExecuteNodeAndDependants(i); // nodes with no dependencies

			// update output skeleton
			outputModelSkeleton->PropagateLocalTransforms();
		}
	}

	void Game::ImGuiCall()
	{
		ImGui::Begin("node editor");

		ImNodes::BeginNodeEditor();
		for (int i = 0; i < editorNodes.size(); i++)
		{
			if (editorNodes[i] != nullptr)
				RetargetNodeImGui(i, *editorNodes[i]);
		}
		for (const Link& link : editorLinks)
		{
			if (link.id != -1)
				ImNodes::Link(link.id, link.a, link.b);
		}
		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
		ImNodes::EndNodeEditor();

		int start_attr, end_attr;
		if (ImNodes::IsLinkCreated(&start_attr, &end_attr))
		{
			int nodeA = start_attr / 1000;
			int pinA = start_attr % 1000;
			int nodeB = end_attr / 1000;
			int pinB = end_attr % 1000;

			// check type match before creating
			if (RetargetGetPinType(*editorNodes[nodeA], pinA) == RetargetGetPinType(*editorNodes[nodeB], pinB))
			{
				int newLinkId = FindNewLinkId();
				editorLinks[newLinkId] = { newLinkId, start_attr, end_attr };

				const void* pointerFromNodeA = RetargetNodeGetOutput(editorNodes[nodeA][0], pinA);
				RetargetNodeSetInput(editorNodes[nodeB][0], pinB, pointerFromNodeA);
				RetargetNodeAddToRight(editorNodes[nodeA][0], nodeB);
			}
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace))
		{
			int count = ImNodes::NumSelectedLinks();
			int* selected_links = (int*) alloca(sizeof(int) * count);
			ImNodes::GetSelectedLinks(selected_links);
			for (int i = 0; i < count; i++)
			{
				if (editorLinks[selected_links[i]].id == -1)
					continue;

				int nodeA = editorLinks[selected_links[i]].a / 1000;
				int pinA = editorLinks[selected_links[i]].a % 1000;
				int nodeB = editorLinks[selected_links[i]].b / 1000;
				int pinB = editorLinks[selected_links[i]].b % 1000;

				RetargetNodeSetInput(editorNodes[nodeB][0], pinB, nullptr);
				RetargetNodeRemoveFromRight(editorNodes[nodeA][0], nodeB);
				editorLinks[selected_links[i]].id = -1;
			}
			count = ImNodes::NumSelectedNodes();
			int* selected_nodes = (int*) alloca(sizeof(int) * count);
			ImNodes::GetSelectedNodes(selected_nodes);
			for (int i = 0; i < count; i++)
			{
				int selectedNode = selected_nodes[i];
				if (editorNodes[selectedNode] == nullptr)
					continue;
				if (editorNodes[selectedNode]->inputQuat.type == RetargetNodeType::InputBones)
					continue;
				if (editorNodes[selectedNode]->inputQuat.type == RetargetNodeType::OutputBones)
					continue;
				for (int j = 0; j < editorLinks.size(); j++)
				{
					if (editorLinks[j].id == -1)
						continue;

					int nodeA = editorLinks[j].a / 1000;
					int pinA = editorLinks[j].a % 1000;
					int nodeB = editorLinks[j].b / 1000;
					int pinB = editorLinks[j].b % 1000;

					if (selectedNode == nodeA)
					{
						RetargetNodeSetInput(editorNodes[nodeB][0], pinB, nullptr);
						editorLinks[j].id = -1;
					}
					if (selectedNode == nodeB)
					{
						RetargetNodeRemoveFromRight(editorNodes[nodeA][0], nodeB);
						editorLinks[j].id = -1;
					}
				}
				delete editorNodes[selectedNode];
				editorNodes[selectedNode] = nullptr;
			}
		}
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("node_create_menu");
		}
		if (ImGui::BeginPopup("node_create_menu"))
		{
			RetargetNodeType newNodeType = RetargetNodeType::INVALID;
			if (ImGui::MenuItem("Quaternion from euler"))
				newNodeType = RetargetNodeType::InputQuat;
			else if (ImGui::MenuItem("Multiply quaternions"))
				newNodeType = RetargetNodeType::QuatMultiply;
			else if (ImGui::MenuItem("Slerp"))
				newNodeType = RetargetNodeType::Slerp;
			else if (ImGui::MenuItem("Scale vector"))
				newNodeType = RetargetNodeType::ScaleVector;
			else if (ImGui::MenuItem("Rotation sandwich"))
				newNodeType = RetargetNodeType::RotSandwich;

			if (newNodeType != RetargetNodeType::INVALID)
			{
				int newNodeId = FindNewNodeId();
				editorNodes[newNodeId] = new RetargetNode();
				RetargetNodeInitialize(newNodeType, editorNodes[newNodeId][0]);

				ImNodes::SetNodeScreenSpacePos(newNodeId, ImGui::GetMousePos());
			}

			ImGui::EndPopup();
		}

		ImGui::End();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::PushID("inputModelSection");
				ImGui::TextUnformatted("Input model");
				static char inputModelTextFieldBuffer[256];
				ImGui::InputText("Path", inputModelTextFieldBuffer, 256);
				if (ImGui::Button("Load"))
					OpenFile(inputModelTextFieldBuffer, inputModelEntity, inputModelSkeleton, inputModelMesh, inputAnimNames, true);
				ImGui::PopID();

				ImGui::Separator();

				ImGui::PushID("outputModelSection");
				ImGui::TextUnformatted("Output model");
				static char outputModelTextFieldBuffer[256];
				ImGui::InputText("Path", outputModelTextFieldBuffer, 256);
				if (ImGui::Button("Load"))
					OpenFile(outputModelTextFieldBuffer, outputModelEntity, outputModelSkeleton, outputModelMesh, outputAnimNames, false);
				ImGui::PopID();

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (inputModelEntity)
		{
			ImGui::Begin("Input model");
			bool setPosition, setRotation, setScale;
			setPosition = ImGui::DragFloat3("Base position", &inputModelBaseTransform.position.x);
			setRotation = ImGui::DragFloat3("Base rotation", &inputModelBaseRotHelper.x);
			setScale = ImGui::DragFloat("Base scale", &inputModelBaseTransform.scale);
			ImGui::Checkbox("Animate", &inputModelSkeleton->m_animate);
			if (setRotation || setScale || setPosition)
			{
				inputModelBaseTransform.rotation = glm::quat(glm::radians(inputModelBaseRotHelper));
				inputModelEntity.GetComponent<Transform>() = inputModelBaseTransform;
			}

			if (ImGui::CollapsingHeader("Add single animation node"))
			{
				static int animSelection;
				ImGui::PushID("singleSection");
				ImGui::Combo("Animation", &animSelection, inputAnimNames.data(), inputAnimNames.size());
				if (ImGui::Button("Add single animation node"))
					inputModelSkeleton->AddNodeSingle((uint32_t)animSelection, 1.0f);
				ImGui::PopID();
			}

			ImGui::Text("Node count: %u", inputModelSkeleton->m_nodes.size());
			if (ImGui::CollapsingHeader("Existing nodes"))
			{
				for (int i = 0; i < inputModelSkeleton->m_nodes.size(); i++)
				{
					ImGui::PushID(i);
					switch (inputModelSkeleton->m_nodes[i].single.type)
					{
					case Animation::NodeType::Single:
						if (ImGui::CollapsingHeader(inputModelSkeleton->m_nodes[i].single.animation->name))
						{
							ImGui::Text("%s:", inputModelSkeleton->m_nodes[i].single.animation->name);
							ImGui::DragFloat("Weight", &inputModelSkeleton->m_nodes[i].single.weight, 0.01f);
							ImGui::DragFloat("Speed", &inputModelSkeleton->m_nodes[i].single.speed, 0.01f);
						}
						break;
					}
					ImGui::PopID();
				}
			}
			ImGui::End();
		}
		if (outputModelEntity)
		{
			ImGui::Begin("Output model");
			bool setPosition, setRotation, setScale;
			setPosition = ImGui::DragFloat3("Base position", &outputModelBaseTransform.position.x);
			setRotation = ImGui::DragFloat3("Base rotation", &outputModelBaseRotHelper.x);
			setScale = ImGui::DragFloat("Base scale", &outputModelBaseTransform.scale);
			ImGui::Checkbox("Animate", &outputModelSkeleton->m_animate);
			if (setRotation || setScale || setPosition)
			{
				outputModelBaseTransform.rotation = glm::quat(glm::radians(outputModelBaseRotHelper));
				outputModelEntity.GetComponent<Transform>() = outputModelBaseTransform;
			}

			if (ImGui::CollapsingHeader("Add single animation node"))
			{
				static int animSelection;
				ImGui::PushID("singleSection");
				ImGui::Combo("Animation", &animSelection, outputAnimNames.data(), outputAnimNames.size());
				if (ImGui::Button("Add single animation node"))
					outputModelSkeleton->AddNodeSingle((uint32_t)animSelection, 1.0f);
				ImGui::PopID();
			}

			ImGui::Text("Node count: %u", outputModelSkeleton->m_nodes.size());
			if (ImGui::CollapsingHeader("Existing nodes"))
			{
				for (int i = 0; i < outputModelSkeleton->m_nodes.size(); i++)
				{
					ImGui::PushID(i);
					switch (outputModelSkeleton->m_nodes[i].single.type)
					{
					case Animation::NodeType::Single:
						if (ImGui::CollapsingHeader(outputModelSkeleton->m_nodes[i].single.animation->name))
						{
							ImGui::Text("%s:", outputModelSkeleton->m_nodes[i].single.animation->name);
							ImGui::DragFloat("Weight", &outputModelSkeleton->m_nodes[i].single.weight, 0.01f);
							ImGui::DragFloat("Speed", &outputModelSkeleton->m_nodes[i].single.speed, 0.01f);
						}
						break;
					}
					ImGui::PopID();
				}
			}
			ImGui::End();
		}

		inputModelViewer.ImGuiCall("Input viewer");
		outputModelViewer.ImGuiCall("Output viewer");
	}
}