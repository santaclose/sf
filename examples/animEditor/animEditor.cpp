#include <imgui.h>
#include <iostream>

#include <Game.h>
#include <MeshProcessor.h>
#include <Math.hpp>
#include <Random.h>
#include <Input.h>
#include <Defaults.h>
#include <GameInitializationData.h>

#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>

#include "../Viewer.hpp"


namespace sf
{
	std::string Game::ConfigFilePath = "examples/animEditor/config.json";

	Scene scene;

	Entity modelEntity;
	SkeletonData* modelSkeleton;
	MeshData* modelMesh;
	uint32_t modelMaterial;
	BufferLayout modelVertexLayout = BufferLayout({
		BufferComponent::VertexPosition,
		BufferComponent::VertexNormal,
		BufferComponent::VertexBoneIndices,
		BufferComponent::VertexBoneWeights
	});
	glm::vec3 modelBaseRotation;

	float animSpeedSelection = 1.0f;
	int animSelection = 0;
	std::vector<const char*> animNames;
	std::vector<BlendSpacePoint1DCreateInfo> bs1dpoints;
	std::vector<BlendSpacePoint2DCreateInfo> bs2dpoints;
	std::vector<std::pair<float, float>> bs1dPosMinMax;
	std::vector<std::pair<glm::vec2, glm::vec2>> bs2dPosMinMax;

	void ComputeBs1dMinMaxPos(int node)
	{
		bs1dPosMinMax[node].first = bs1dPosMinMax[node].second = modelSkeleton->m_nodes[node].bs1d.points[0].pos;
		for (int k = 1; k < modelSkeleton->m_nodes[node].bs1d.pointCount; k++)
		{
			if (modelSkeleton->m_nodes[node].bs1d.points[k].pos < bs1dPosMinMax[node].first)
				bs1dPosMinMax[node].first = modelSkeleton->m_nodes[node].bs1d.points[k].pos;
			if (modelSkeleton->m_nodes[node].bs1d.points[k].pos > bs1dPosMinMax[node].second)
				bs1dPosMinMax[node].second = modelSkeleton->m_nodes[node].bs1d.points[k].pos;
		}
	}

	void ComputeBs2dMinMaxPos(int node)
	{
		bs2dPosMinMax[node].first = bs2dPosMinMax[node].second = modelSkeleton->m_nodes[node].bs2d.points[0].pos;
		for (int k = 1; k < modelSkeleton->m_nodes[node].bs2d.pointCount; k++)
		{
			if (modelSkeleton->m_nodes[node].bs2d.points[k].pos.x < bs2dPosMinMax[node].first.x)
				bs2dPosMinMax[node].first.x = modelSkeleton->m_nodes[node].bs2d.points[k].pos.x;
			if (modelSkeleton->m_nodes[node].bs2d.points[k].pos.y < bs2dPosMinMax[node].first.y)
				bs2dPosMinMax[node].first.y = modelSkeleton->m_nodes[node].bs2d.points[k].pos.y;
			if (modelSkeleton->m_nodes[node].bs2d.points[k].pos.x > bs2dPosMinMax[node].second.x)
				bs2dPosMinMax[node].second.x = modelSkeleton->m_nodes[node].bs2d.points[k].pos.x;
			if (modelSkeleton->m_nodes[node].bs2d.points[k].pos.y > bs2dPosMinMax[node].second.y)
				bs2dPosMinMax[node].second.y = modelSkeleton->m_nodes[node].bs2d.points[k].pos.y;
		}
	}

	void OpenFile(const char* filePath)
	{
		if (modelEntity)
			scene.DestroyEntity(modelEntity);
		modelEntity = scene.CreateEntity();
		modelEntity.AddComponent<Transform>();

		modelMaterial = Renderer::CreateMaterial(Material("assets/shaders/default.vert", "assets/shaders/default.frag"), modelVertexLayout);
		modelSkeleton = new SkeletonData();
		modelMesh = new MeshData(modelVertexLayout);
		uint32_t gltfid = GltfImporter::Load(filePath);
		GltfImporter::GenerateSkeleton(gltfid, *modelSkeleton);
		GltfImporter::GenerateMeshData(gltfid, *modelMesh);
		modelEntity.AddComponent<SkinnedMesh>(modelMesh, modelMaterial, modelSkeleton);
		animNames.resize(modelSkeleton->m_animations.size());
		for (int i = 0; i < modelSkeleton->m_animations.size(); i++)
			animNames[i] = modelSkeleton->m_animations[i].name;

	}

	void Game::Initialize(int argc, char** argv)
	{
		modelBaseRotation = glm::vec3(-90.0f, 0.0f, 0.0f);
		ExampleViewer::Initialize(scene);
		OpenFile("examples/thirdperson/Fox.glb");
	}

	void Game::Terminate()
	{
		scene.DestroyEntity(modelEntity);
		ExampleViewer::Terminate(scene);
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		ExampleViewer::UpdateCamera(deltaTime);
		if (modelEntity)
		{
			modelEntity.GetComponent<Transform>().rotation = glm::quat(modelBaseRotation * Math::DTOR);
			modelSkeleton->UpdateAnimation(deltaTime);
		}
	}

	void Game::ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "Ctrl+O")) { OpenFile("examples/thirdperson/shanyung_blendspace2d.glb"); }
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (modelEntity)
		{
			ImGui::Begin("Animation");
			ImGui::InputFloat3("Base rotation", &modelBaseRotation.x);
			ImGui::Checkbox("Animate", &modelSkeleton->m_animate);

			if (ImGui::CollapsingHeader("Add single animation node"))
			{
				ImGui::PushID("singleSection");
				ImGui::Combo("Animation", &animSelection, animNames.data(), animNames.size());
				if (ImGui::Button("Add single animation node"))
					modelSkeleton->AddNodeSingle((uint32_t)animSelection, 1.0f);
				ImGui::PopID();
			}
			if (ImGui::CollapsingHeader("Add blendspace 1d node"))
			{
				ImGui::PushID("blendspace1dSection");
				ImGui::Combo("Animation", &animSelection, animNames.data(), animNames.size());
				if (ImGui::Button("Add blendspace 1d point"))
					bs1dpoints.push_back({ (uint32_t)animSelection, 1.0f, (float)bs1dpoints.size() });
				{
					for (int i = 0; i < bs1dpoints.size(); i++)
						ImGui::Text(animNames[bs1dpoints[i].animationIndex]);
				}
				if (ImGui::Button("Add blendspace 1d node"))
				{
					modelSkeleton->AddNodeBlendSpace1D(bs1dpoints, 0.0f);
					bs1dPosMinMax.resize(modelSkeleton->m_nodes.size());
					ComputeBs1dMinMaxPos(modelSkeleton->m_nodes.size() - 1);
					bs1dpoints.clear();
				}
				ImGui::PopID();
			}
			if (ImGui::CollapsingHeader("Add blendspace 2d node"))
			{
				ImGui::PushID("blendspace2dSection");
				ImGui::Combo("Animation", &animSelection, animNames.data(), animNames.size());
				if (ImGui::Button("Add blendspace 2d point"))
					bs2dpoints.push_back({ (uint32_t)animSelection, 1.0f, { glm::cos((float)bs2dpoints.size()), glm::sin((float)bs2dpoints.size()) } });
				{
					for (int i = 0; i < bs2dpoints.size(); i++)
						ImGui::Text(animNames[bs2dpoints[i].animationIndex]);
				}
				if (ImGui::Button("Add blendspace 2d node"))
				{
					modelSkeleton->AddNodeBlendSpace2D(bs2dpoints, { 0.0f, 0.0f });
					bs2dPosMinMax.resize(modelSkeleton->m_nodes.size());
					ComputeBs2dMinMaxPos(modelSkeleton->m_nodes.size() - 1);
					bs2dpoints.clear();
				}
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
					case Animation::NodeType::BlendSpace1D:
						if (ImGui::CollapsingHeader("BlendSpace1D"))
						{
							ImGui::DragFloat("Weight", &modelSkeleton->m_nodes[i].bs1d.weight, 0.01f);
							ImGui::DragFloat("Position", &modelSkeleton->m_nodes[i].bs1d.pos, 0.01f);
							ImGui::Text("Point count: %u", modelSkeleton->m_nodes[i].bs1d.pointCount);
							for (int j = 0; j < modelSkeleton->m_nodes[i].bs1d.pointCount; j++)
							{
								ImGui::PushID(j);
								ImGui::Text("%s:", modelSkeleton->m_nodes[i].bs1d.points[j].animation->name);
								ImGui::DragFloat("Speed", &modelSkeleton->m_nodes[i].bs1d.points[j].speed, 0.01f);
								if (ImGui::DragFloat("Position", &modelSkeleton->m_nodes[i].bs1d.points[j].pos, 0.01f))
								{
									ComputeBs1dMinMaxPos(i);
								}
								ImGui::PopID();
							}
						}
						break;
					case Animation::NodeType::BlendSpace2D:
						if (ImGui::CollapsingHeader("BlendSpace2D"))
						{
							ImGui::DragFloat("Weight", &modelSkeleton->m_nodes[i].bs2d.weight, 0.01f);
							ImGui::DragFloat2("Position", &modelSkeleton->m_nodes[i].bs2d.pos.x, 0.01f);
							ImGui::Text("Point count: %u", modelSkeleton->m_nodes[i].bs2d.pointCount);
							for (int j = 0; j < modelSkeleton->m_nodes[i].bs2d.pointCount; j++)
							{
								ImGui::PushID(j);
								ImGui::Text("%s:", modelSkeleton->m_nodes[i].bs2d.points[j].animation->name);
								ImGui::DragFloat("Speed", &modelSkeleton->m_nodes[i].bs2d.points[j].speed, 0.01f);
								if (ImGui::DragFloat2("Position", &modelSkeleton->m_nodes[i].bs2d.points[j].pos.x, 0.01f))
								{
									modelSkeleton->m_nodes[i].bs2d.needsToComputeBlendMatrix = true;
									ComputeBs2dMinMaxPos(i);
								}
								ImGui::PopID();
							}
						}
						break;
					}
					ImGui::PopID();
				}
			}
			ImGui::End();
			/* Clamp node positions */
			for (int i = 0; i < modelSkeleton->m_nodes.size(); i++)
			{
				switch (modelSkeleton->m_nodes[i].single.type)
				{
				case Animation::NodeType::BlendSpace1D:
					if (modelSkeleton->m_nodes[i].bs1d.pos < bs1dPosMinMax[i].first)
						modelSkeleton->m_nodes[i].bs1d.pos = bs1dPosMinMax[i].first;
					if (modelSkeleton->m_nodes[i].bs1d.pos > bs1dPosMinMax[i].second)
						modelSkeleton->m_nodes[i].bs1d.pos = bs1dPosMinMax[i].second;
					break;
				case Animation::NodeType::BlendSpace2D:
					if (modelSkeleton->m_nodes[i].bs2d.pos.x < bs2dPosMinMax[i].first.x)
						modelSkeleton->m_nodes[i].bs2d.pos.x = bs2dPosMinMax[i].first.x;
					if (modelSkeleton->m_nodes[i].bs2d.pos.x > bs2dPosMinMax[i].second.x)
						modelSkeleton->m_nodes[i].bs2d.pos.x = bs2dPosMinMax[i].second.x;
					if (modelSkeleton->m_nodes[i].bs2d.pos.y < bs2dPosMinMax[i].first.y)
						modelSkeleton->m_nodes[i].bs2d.pos.y = bs2dPosMinMax[i].first.y;
					if (modelSkeleton->m_nodes[i].bs2d.pos.y > bs2dPosMinMax[i].second.y)
						modelSkeleton->m_nodes[i].bs2d.pos.y = bs2dPosMinMax[i].second.y;
					break;
				default:
					break;
				}
			}
		}

		ExampleViewer::ImGuiCall();
	}
}