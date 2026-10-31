#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>
#include <fstream>

#include <glm/gtx/rotate_vector.hpp>

#include <Defaults.h>
#include <Game.h>
#include <Math.hpp>
#include <Input.h>
#include <FileUtils.h>
#include <Random.h>
#include <EntityIntersect.h>

#include <Renderer/Renderer.h>

#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>


#include <Components/Mesh.h>
#include <Components/Camera.h>
#include <Components/Transform.h>
#include <Components/MeshCollider.h>

#include "../Terrain.hpp"

#define MOUSE_SENSITIVITY 0.003
#define SCROLL_SENSITIVITY 0.12

#define GIMBAL_ROTATION_SPEED 15.0f
#define MODEL_OFFSET 50.0
#define MIN_CAMERA_DISTANCE 0.5f

#define GIMBAL_OFFSET_SHANYUNG 1.0f
#define GIMBAL_OFFSET_FOX 0.5f

#define TREE_COUNT 500

#include "houseObjectManager.h"

namespace sf
{
	namespace Game
	{
		Scene scene;
		Entity gimbal, cameraObject;
		Entity shanyung, fox, foxCol;

		std::vector<Entity> houseObjects;

		struct BitmapAoData
		{
			glm::vec3 origin;
			glm::vec3 size;
		};
		MeshData* buildingMeshes[3];
		MeshData* buildingColMeshes[3];
		Bitmap buildingAoBitmaps[3];
		BitmapAoData buildingAoData[3];
		const std::vector<const char*> buildingMeshNames = {"ba2f", "ba3f", "house"};
		std::vector<const std::vector<ObjImporter::ObjMarkerExt>*> perBuildingMarkers;
		std::vector<LODGroup*> buildingLodGroups;
		uint32_t houseCountPerGroup[] = { 4, 6, 8 };
		struct BuildingData
		{
			Entity entity;
			uint32_t type;
		};
		std::vector<BuildingData> houses;

		std::vector<Entity> trees;
		glm::vec3 targetGimbalRotation;

		float cameraDistance;

		Buffer::Layout positionOnlyVertexLayout = Buffer::Layout({
			Buffer::Component::Position
		});

		Buffer::Layout treeObjectVertexLayout = Buffer::Layout({
			Buffer::Component::Position,
			Buffer::Component::Normal,
			Buffer::Component::UV,
			Buffer::Component::AO
		});

		Buffer::Layout houseObjectVertexLayout = Buffer::Layout({
			Buffer::Component::Position,
			Buffer::Component::Normal,
			Buffer::Component::UV
		});

		Buffer::Layout characterVertexLayout = Buffer::Layout({
			Buffer::Component::BoneIndices,
			Buffer::Component::BoneWeights,
			Buffer::Component::Position,
			Buffer::Component::UV,
			Buffer::Component::AO
		});
		SkeletonData* shanyungSkeleton;
		MeshData* shanyungMesh;
		uint32_t shanyungBlendSpace;
		glm::vec2 shanyungBlendSpaceCurrentPos;
		std::vector<float> shanyungWeights;
		std::vector<glm::vec2> shanyungSpeedPerAnimation;
		float shanyungVerticalSpeed = 0.0f;
		float shanyungCapsuleSeparation;
		LODGroup shanyungLodGroup;

		SkeletonData* foxSkeleton;
		MeshData* foxMesh;
		uint32_t foxBlendSpace;
		float foxBlendSpaceCurrentX;
		std::vector<float> foxWeights;
		std::vector<float> foxSpeedPerAnimation;
		float foxVerticalSpeed = 0.0f;
		float foxSphereSeparation;

		Material buildingMaterial;
		Material buildingObjectMaterial;
		Material buildingFlatMaterial;
		Material characterMaterial;
		Material genericMaterial;
		Terrain terrain;

		const glm::vec3 treeLeavesColor = { 0.0f, 0.44f, 0.0f };
		const glm::vec3 treeBranchColor = { 128.0f / 255.0f, 96.0f / 255.0f, 89.0f / 255.0f };
		const glm::vec3 skinColor = { 0.82f, 0.66f, 0.52f };
		const glm::vec3 eyeColor = { 1.0f, 1.0f, 1.0f };
		const glm::vec3 foxColor = { 213.0f / 255.0f, 119.0f / 255.0f, 5.0f / 255.0f };

		LODSpec houseLodSpec;
		LODSpec shanyungLodSpec;

		Bitmap eyeColorBitmap;
		Bitmap euroAshBranchBitmap;
		Bitmap euroAshLeafBitmap;

		void UpdateCamera(float deltaTime, Entity targetCharacter, float gimbalOffsetY)
		{
			Transform& gimbalT = gimbal.GetComponent<Transform>();
			Transform& camT = cameraObject.GetComponent<Transform>();

			cameraDistance -= glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollUp() ? SCROLL_SENSITIVITY : 0.0f);
			cameraDistance += glm::sqrt(cameraDistance) * (Input::Key(Input::KeyCode::LeftShift) ? 0.5f : 1.0f) * (Input::MouseScrollDown() ? SCROLL_SENSITIVITY : 0.0f);
			cameraDistance = glm::max(MIN_CAMERA_DISTANCE, cameraDistance);

			gimbalT.position = targetCharacter.GetComponent<Transform>().position + glm::vec3(0.0f, gimbalOffsetY, 0.0f);
			targetGimbalRotation.y -= Input::MousePosDeltaX() * MOUSE_SENSITIVITY;
			targetGimbalRotation.x += Input::MousePosDeltaY() * MOUSE_SENSITIVITY;
			targetGimbalRotation.x = glm::clamp(targetGimbalRotation.x, -Math::Pi * 0.499f, Math::Pi * 0.499f);

			gimbalT.rotation = glm::slerp(gimbalT.rotation, glm::quat(targetGimbalRotation), deltaTime * GIMBAL_ROTATION_SPEED);

			camT.position = gimbalT.position + gimbalT.Forward() * cameraDistance;

			Geometry::RayHit rh;
			for (uint32_t i = 0; i < houses.size(); i++)
			{
				if (EntityIntersect::MovingSphereMesh(gimbal, camT.position - gimbalT.position, houses[i].entity, &rh))
					camT.position = gimbalT.position + gimbalT.Forward() * rh.distance;
			}

			camT.LookAt(gimbalT.position, glm::vec3(0.0, 1.0, 0.0));
			float terrainY;
			terrain.Sample(camT.position, terrainY);
			camT.position.y = glm::max(camT.position.y, terrainY);
		}

		void SwitchCharacter()
		{
			bool f = fox.IsEnabled();
			fox.SetEnabled(!f);
			foxCol.SetEnabled(!f);
			shanyung.SetEnabled(f);
		}

		float CharacterStepCastShanyung(Entity charEntity)
		{
			CapsuleCollider& cc = charEntity.GetComponent<CapsuleCollider>();

			bool hitMesh = false;
			int hitMeshIndex;
			Geometry::RayHit rh;
			for (int i = 0; i < houses.size(); i++)
			{
				if (EntityIntersect::MovingCapsuleMesh(charEntity, glm::vec3(0.0f, -30.0f, 0.0f), houses[i].entity, &rh))
				{
					hitMesh = true;
					hitMeshIndex = i;
					break;
				}
			}
			CapsuleCollider cc_world = EntityIntersect::WorldSpace<CapsuleCollider>(charEntity);
			glm::vec3 capsuleBottom = cc_world.centerA - glm::vec3(0.0f, cc_world.radius, 0.0f);
			float terrainY;
			terrain.Sample(capsuleBottom, terrainY);
			float terrainDis = capsuleBottom.y - terrainY;
			if (hitMesh)
			{
				Transform& buildingTransform = houses[hitMeshIndex].entity.GetComponent<Transform>();
				SkinnedMesh& sm = shanyung.GetComponent<SkinnedMesh>();
				sm.uniformOverrides[0]["useAoSampler"].u32 = sm.uniformOverrides[1]["useAoSampler"].u32 = 1;
				sm.uniformOverrides[0]["aoSampler"].p = sm.uniformOverrides[1]["aoSampler"].p = &buildingAoBitmaps[houses[hitMeshIndex].type];
				sm.uniformOverrides[0]["aoSamplerOrigin"].p = sm.uniformOverrides[1]["aoSamplerOrigin"].p = &buildingAoData[houses[hitMeshIndex].type].origin;
				sm.uniformOverrides[0]["aoSamplerSize"].p = sm.uniformOverrides[1]["aoSamplerSize"].p = &buildingAoData[houses[hitMeshIndex].type].size;
				sm.uniformOverrides[0]["aoSamplerModelPosition"].p = sm.uniformOverrides[1]["aoSamplerModelPosition"].p = &buildingTransform.position;
				sm.uniformOverrides[0]["aoSamplerModelRotation"].p = sm.uniformOverrides[1]["aoSamplerModelRotation"].p = &buildingTransform.rotation;
				return glm::min(terrainDis, rh.distance);
			}
			else
			{
				SkinnedMesh& sm = shanyung.GetComponent<SkinnedMesh>();
				sm.uniformOverrides[0]["useAoSampler"].u32 = sm.uniformOverrides[1]["useAoSampler"].u32 = 0;
				return terrainDis;
			}
		}

		float CharacterStepCastFox(Entity charEntity)
		{
			bool hitMesh = false;
			int hitMeshIndex;
			Geometry::RayHit rh;
			for (int i = 0; i < houses.size(); i++)
			{
				if (EntityIntersect::MovingSphereMesh(charEntity, glm::vec3(0.0f, -30.0f, 0.0f), houses[i].entity, &rh))
				{
					hitMesh = true;
					hitMeshIndex = i;
					break;
				}
			}
			SphereCollider sc_world = EntityIntersect::WorldSpace<SphereCollider>(charEntity);
			glm::vec3 sphereBottom = sc_world.center - glm::vec3(0.0f, sc_world.radius, 0.0f);
			float terrainY;
			terrain.Sample(sphereBottom, terrainY);
			float terrainDis = sphereBottom.y - terrainY;
			if (hitMesh)
			{
				Transform& buildingTransform = houses[hitMeshIndex].entity.GetComponent<Transform>();
				SkinnedMesh& sm = fox.GetComponent<SkinnedMesh>();
				sm.uniformOverrides[0]["useAoSampler"].u32 = 1;
				sm.uniformOverrides[0]["aoSampler"].p = &buildingAoBitmaps[houses[hitMeshIndex].type];
				sm.uniformOverrides[0]["aoSamplerOrigin"].p = &buildingAoData[houses[hitMeshIndex].type].origin;
				sm.uniformOverrides[0]["aoSamplerSize"].p = &buildingAoData[houses[hitMeshIndex].type].size;
				sm.uniformOverrides[0]["aoSamplerModelPosition"].p = &buildingTransform.position;
				sm.uniformOverrides[0]["aoSamplerModelRotation"].p = &buildingTransform.rotation;
				return glm::min(terrainDis, rh.distance);
			}
			else
			{
				SkinnedMesh& sm = fox.GetComponent<SkinnedMesh>();
				sm.uniformOverrides[0]["useAoSampler"].u32 = 0;
				return terrainDis;
			}
		}
	}

	Game::InitData Game::GetInitData()
	{
		InitData id;
		id.cursorRequired = false;
		id.toolBarEnabled = false;
		return id;
	}

	void Game::Initialize(int argc, char** argv)
	{
		FileUtils::CreateFolder("assets/examples");
		FileUtils::DownloadFiles({
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/Fox/glTF-Binary/Fox.glb",
			"https://us.v-cdn.net/5021068/uploads/editor/ha/7frj09nru4zu.png",
			"https://github.com/santaclose/sample_models/raw/master/shanyung_blendspace2d.glb",
			"https://casual-effects.com/g3d/data10/research/model/horseChestnut/chestnut.zip"
			}, "assets/examples/");

		eyeColorBitmap.CreateFromFile("assets/examples/eye_c.png");

		characterMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		characterMaterial.fragShaderFilePath = "examples/thirdperson/buildingObject.frag";
		characterMaterial.uniforms["useAoSampler"] = { DataType::b, 0 };
		characterMaterial.uniforms["aoSampler"] = { DataType::bitmap, nullptr };
		characterMaterial.uniforms["aoSamplerOrigin"] = { DataType::vec3f32, nullptr };
		characterMaterial.uniforms["aoSamplerSize"] = { DataType::vec3f32, nullptr };
		characterMaterial.uniforms["aoSamplerModelPosition"] = { DataType::vec3f32, nullptr };
		characterMaterial.uniforms["aoSamplerModelRotation"] = { DataType::vec4f32, nullptr };

		buildingFlatMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		buildingFlatMaterial.fragShaderFilePath = "assets/shaders/defaultFlat.frag";

		genericMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		genericMaterial.fragShaderFilePath = "examples/thirdperson/generic.frag";
		genericMaterial.uniforms["color"] = { DataType::vec3f32, nullptr };
		genericMaterial.uniforms["useColorSampler"] = { DataType::b, 0 };
		genericMaterial.uniforms["colorSampler"] = { DataType::bitmap, nullptr };
		genericMaterial.uniforms["useAoSampler"] = { DataType::b, 0 };
		genericMaterial.uniforms["aoSampler"] = { DataType::bitmap, nullptr };
		genericMaterial.uniforms["aoSamplerOrigin"] = { DataType::vec3f32, nullptr };
		genericMaterial.uniforms["aoSamplerSize"] = { DataType::vec3f32, nullptr };
		genericMaterial.uniforms["aoSamplerModelPosition"] = { DataType::vec3f32, nullptr };
		genericMaterial.uniforms["aoSamplerModelRotation"] = { DataType::vec4f32, nullptr };

		buildingMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		buildingMaterial.fragShaderFilePath = "examples/thirdperson/building.frag";
		buildingMaterial.uniforms["aoSampler"] = { DataType::bitmap, nullptr };
		buildingMaterial.uniforms["aoSamplerOrigin"] = { DataType::vec3f32, nullptr };
		buildingMaterial.uniforms["aoSamplerSize"] = { DataType::vec3f32, nullptr };
		buildingMaterial.uniforms["aoSamplerModelPosition"] = { DataType::vec3f32, nullptr };
		buildingMaterial.uniforms["aoSamplerModelRotation"] = { DataType::vec4f32, nullptr };

		buildingObjectMaterial.vertShaderFilePath = "assets/shaders/default.vert";
		buildingObjectMaterial.fragShaderFilePath = "examples/thirdperson/buildingObject.frag";
		buildingObjectMaterial.uniforms["aoSampler"] = { DataType::bitmap, nullptr };
		buildingObjectMaterial.uniforms["aoSamplerOrigin"] = { DataType::vec3f32, nullptr };
		buildingObjectMaterial.uniforms["aoSamplerSize"] = { DataType::vec3f32, nullptr };
		buildingObjectMaterial.uniforms["aoSamplerModelPosition"] = { DataType::vec3f32, nullptr };
		buildingObjectMaterial.uniforms["aoSamplerModelRotation"] = { DataType::vec4f32, nullptr };

		targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);
		cameraDistance = 3.0;
		shanyungBlendSpaceCurrentPos = { 0.0f, 0.0f };
		foxBlendSpaceCurrentX = 0.0f;

		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();

		gimbal.AddComponent<Transform>();
		SphereCollider& c_sc = gimbal.AddComponent<SphereCollider>();
		c_sc.radius = 0.15;

		Camera& cam = cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();

		terrain.Create(scene, "../Downloads/Telegram Desktop/test.r16", 0.5566f, 152.0f, 41,
			glm::vec3(-(float)(1025 - 1) * 0.5f * 0.5566f, 0.0f, (float)(1025 - 1) * 0.5f * 0.5566f));

		hom::Initialize(treeObjectVertexLayout);

		// Houses
		{
			bool loadSucceeded = houseLodSpec.LoadFromFile("assets/examples/building.lod");
			assert(loadSucceeded);
			for (int i = 0; i < buildingMeshNames.size(); i++)
			{
				const char* name = buildingMeshNames[i];
				std::string filePath = std::string("assets/examples/") + std::string(name) + std::string(".obj");
				std::string colFilePath = std::string("assets/examples/") + std::string(name) + std::string("Col.obj");
				buildingMeshes[i] = new MeshData();
				int objid = ObjImporter::Load(filePath.c_str());
				ObjImporter::GenerateMeshData(objid, treeObjectVertexLayout, *buildingMeshes[i]);

				sf::VoxelVolumeData vv2;
				vv2.BuildFromMesh(*buildingMeshes[i], 0.12, 0u, 2);
				std::string aoBitmapFileName = std::string("assets/examples/") + std::string(name) + std::string(".bitmap");
				if (!buildingAoBitmaps[i].LoadFromFile(aoBitmapFileName.c_str()))
				{
					sf::MeshProcessor::AmbientOcclusionBitmapFromVoxelVolume(vv2, buildingAoBitmaps[i]);
					buildingAoBitmaps[i].SaveToFile(aoBitmapFileName.c_str());
				}

				buildingAoData[i].origin = vv2.offset;
				buildingAoData[i].size = glm::vec3((float) vv2.voxelCountPerAxis.x * vv2.voxelSize, (float) vv2.voxelCountPerAxis.y * vv2.voxelSize, (float) vv2.voxelCountPerAxis.z * vv2.voxelSize);

				buildingLodGroups.emplace_back();
				buildingLodGroups.back() = new LODGroup();
				buildingLodGroups.back()->Create(*buildingMeshes[i], houseLodSpec);

				perBuildingMarkers.emplace_back();
				perBuildingMarkers.back() = ObjImporter::GetMarkers(objid);
				printf("BUILDING WITH NAME %s has %u markers\n", name, perBuildingMarkers.back()->size());

				buildingColMeshes[i] = new MeshData();
				objid = ObjImporter::Load(colFilePath.c_str());
				ObjImporter::GenerateMeshData(objid, positionOnlyVertexLayout, *buildingColMeshes[i]);
			}

			uint32_t attempts = 0;
			while (attempts < 5000)
			{
				int buildingToUseIndex = Random::Int(3);
				LODGroup* buildingLODGroup = buildingLodGroups[buildingToUseIndex];
				MeshData* buildingColMesh = buildingColMeshes[buildingToUseIndex];

				int numberOfBuildings = houseCountPerGroup[Random::Int(sizeof(houseCountPerGroup) / sizeof(houseCountPerGroup[0]))];
				houses.emplace_back();
				houses.back().type = buildingToUseIndex;
				houses.back().entity = scene.CreateEntity();
				float houseBoundingSphereRadius = houses.back().entity.AddComponent<MeshCollider>(buildingColMesh).boundingSphere.radius;

				glm::vec2 area;
				uint32_t numStepsX;
				switch (numberOfBuildings)
				{
				case 4:
					area.x = houseBoundingSphereRadius * 2.0f * 2.0f;
					area.y = houseBoundingSphereRadius * 2.0f * 2.0f;
					numStepsX = 2;
					break;
				case 6:
					area.x = houseBoundingSphereRadius * 2.0f * 3.0f;
					area.y = houseBoundingSphereRadius * 2.0f * 2.0f;
					numStepsX = 3;
					break;
				case 8:
					area.x = houseBoundingSphereRadius * 2.0f * 4.0f;
					area.y = houseBoundingSphereRadius * 2.0f * 2.0f;
					numStepsX = 4;
					break;
				default:
					assert(false);
					break;
				}

				bool areaIsAvailable = true;
				glm::vec2 areaCenter;
				areaCenter.x = (Random::Float() - 0.5f) * 1025.0f * 0.5f;
				areaCenter.y = (Random::Float() - 0.5f) * 1025.0f * 0.5f;
				const float maxDeltaY = 0.02f;
				float areaRotation = Random::Float() * 2 * glm::pi<float>();
				uint32_t areaSampleCount = 1000;
				for ( ; areaSampleCount > 0; areaSampleCount--)
				{
					glm::vec2 right = glm::rotate(glm::vec2(area.x / 2.0f, 0.0f), areaRotation) * ((Random::Float() - 0.5f) * 2.0f);
					glm::vec2 forward = glm::rotate(glm::vec2(0.0f, area.y / 2.0f), areaRotation) * ((Random::Float() - 0.5f) * 2.0f);
					glm::vec3 sample = glm::vec3(areaCenter.x, 0.0f, areaCenter.y) + glm::vec3(right.x, 0.0f, right.y) + glm::vec3(forward.x, 0.0f, forward.y);
					for (int i = 0; i < houses.size() - 1; i++) // -1 because we added one to check the bounding sphere
					{
						SphereCollider houseSphere = houses[i].entity.GetComponent<MeshCollider>().boundingSphere.ApplyTransform(houses[i].entity.GetComponent<Transform>());
						if (glm::distance2(glm::vec2(sample.x, sample.z), glm::vec2(houseSphere.center.x, houseSphere.center.z)) < houseSphere.radius * houseSphere.radius)
						{
							areaIsAvailable = false;
							break;
						}
					}
					if (!areaIsAvailable)
						break;
					terrain.Sample(sample, sample.y);
					glm::vec3 sampleX = sample + glm::vec3(0.1f, 0.0f, 0.0f);
					terrain.Sample(sampleX, sampleX.y);
					glm::vec3 sampleZ = sample + glm::vec3(0.0f, 0.0f, 0.1f);
					terrain.Sample(sampleZ, sampleZ.y);
					if (glm::abs(sampleX.y - sample.y) > maxDeltaY || glm::abs(sampleZ.y - sample.y) > maxDeltaY)
					{
						areaIsAvailable = false;
						break;
					}
				}
				if (!areaIsAvailable)
				{
					attempts++;
					scene.DestroyEntity(houses.back().entity);
					houses.resize(houses.size() - 1);
				}
				else
				{
					attempts = 0;
					houses.resize(houses.size() + numberOfBuildings - 1);
					for (int i = 0; i < numberOfBuildings -1; i++)
					{
						houses[houses.size() - 1 - i].entity = scene.CreateEntity();
						houses[houses.size() - 1 - i].entity.AddComponent<MeshCollider>(buildingColMesh);
						houses[houses.size() - 1 - i].type = buildingToUseIndex;
					}
					glm::vec2 right = glm::rotate(glm::vec2(1.0f, 0.0f), areaRotation);
					glm::vec2 forward = glm::rotate(glm::vec2(0.0f, 1.0f), areaRotation);
					glm::vec2 origin = areaCenter - right * area.x * 0.5f - forward * area.y * 0.5f;

					for (int i = 0; i < numberOfBuildings; i++)
					{
						glm::vec2 cursor = origin + right * (houseBoundingSphereRadius * (1 + 2 * (i % numStepsX))) + forward * (houseBoundingSphereRadius * (1 + 2 * (i / numStepsX)));
						Mesh& m = houses[houses.size() - 1 - i].entity.AddComponent<Mesh>(buildingLODGroup, &buildingMaterial);
						Transform& t = houses[houses.size() - 1 - i].entity.AddComponent<Transform>();
						t.position = glm::vec3(cursor.x, 0.0f, cursor.y);
						t.rotation = glm::quat(glm::vec3(0.0f, -areaRotation, 0.0f));
						terrain.Sample(t.position, t.position.y);
						m.uniformOverrides[0]["aoSampler"].p = &buildingAoBitmaps[buildingToUseIndex];
						m.uniformOverrides[0]["aoSamplerOrigin"].p = &buildingAoData[buildingToUseIndex].origin;
						m.uniformOverrides[0]["aoSamplerSize"].p = &buildingAoData[buildingToUseIndex].size;
						m.uniformOverrides[0]["aoSamplerModelPosition"].p = &t.position;
						m.uniformOverrides[0]["aoSamplerModelRotation"].p = &t.rotation;

						const std::vector<ObjImporter::ObjMarkerExt>* markers = perBuildingMarkers[buildingToUseIndex];
						for (const ObjImporter::ObjMarkerExt& marker : *markers)
						{
							houseObjects.emplace_back();
							houseObjects.back() = scene.CreateEntity();

							Transform& ho_t = houseObjects.back().AddComponent<Transform>();
							ho_t = t;
							Transform toApply;
							toApply.position = marker.position;
							toApply.rotation = marker.rotation;
							ho_t.Apply(toApply);
							Mesh& ho_m = houseObjects.back().AddComponent<Mesh>(hom::GetLodsByType(marker.name), &buildingObjectMaterial);
							ho_m.uniformOverrides[0]["aoSampler"].p = &buildingAoBitmaps[buildingToUseIndex];
							ho_m.uniformOverrides[0]["aoSamplerOrigin"].p = &buildingAoData[buildingToUseIndex].origin;
							ho_m.uniformOverrides[0]["aoSamplerSize"].p = &buildingAoData[buildingToUseIndex].size;
							ho_m.uniformOverrides[0]["aoSamplerModelPosition"].p = &t.position;
							ho_m.uniformOverrides[0]["aoSamplerModelRotation"].p = &t.rotation;
							houseObjects.back().AddComponent<MeshCollider>(hom::GetLodsByType(marker.name)->sourceMesh);
						}
					}
				}
			}

			trees.reserve(TREE_COUNT);
			for (int i = 0; i < TREE_COUNT; i++)
			{
				trees.emplace_back();
				trees.back() = scene.CreateEntity();
				const LODGroup* treeLodGroup = hom::GetLodsByType("tree");
				Mesh& m = trees.back().AddComponent<Mesh>(treeLodGroup, &genericMaterial);
				assert(m.uniformOverrides.size() == 2);
				m.uniformOverrides[0]["color"].p = &treeBranchColor;
				m.uniformOverrides[1]["color"].p = &treeLeavesColor;
				trees.back().AddComponent<MeshCollider>(treeLodGroup->GetMeshDataForLOD(0));
				Transform& t = trees.back().AddComponent<Transform>();

				bool treeIsInAvailablePlace;
				do
				{
					treeIsInAvailablePlace = true;
					glm::vec2 treeCenter;
					treeCenter.x = (Random::Float() - 0.5f) * 1025.0f * 0.5f;
					treeCenter.y = (Random::Float() - 0.5f) * 1025.0f * 0.5f;
					t.position = glm::vec3(treeCenter.x, 0.0f, treeCenter.y);
					terrain.Sample(t.position, t.position.y);
					glm::vec3 sampleX = t.position + glm::vec3(0.1f, 0.0f, 0.0f);
					terrain.Sample(sampleX, sampleX.y);
					glm::vec3 sampleZ = t.position + glm::vec3(0.0f, 0.0f, 0.1f);
					terrain.Sample(sampleZ, sampleZ.y);
					float maxDeltaY = 0.02f;
					if (glm::abs(sampleX.y - t.position.y) > maxDeltaY || glm::abs(sampleZ.y - t.position.y) > maxDeltaY)
					{
						treeIsInAvailablePlace = false;
						continue;
					}
					t.rotation = glm::quat(glm::vec3(0.0f, Math::Pi * 2.0f * Random::Float(), 0.0f));
					t.scale = Random::Float() + 1.0;

					for (const BuildingData& h : houses)
					{
						if (EntityIntersect::MeshBoundingSpheres(h.entity, trees.back()))
							treeIsInAvailablePlace = false;
					}
				}
				while (!treeIsInAvailablePlace);
				trees.back().RemoveComponent<MeshCollider>();
				CapsuleCollider& cc = trees.back().AddComponent<CapsuleCollider>();
				cc.centerA = glm::vec3(0.0f, 0.0f, 0.0f);
				cc.centerB = glm::vec3(0.0f, 2.0f, 0.0f);
				cc.radius = 0.2f * t.scale;
			}
		}

		{
			shanyung = scene.CreateEntity();
			shanyung.AddComponent<Transform>();

			shanyungSkeleton = new SkeletonData();
			shanyungMesh = new MeshData();
			int gltfid = GltfImporter::Load("assets/examples/shanyung_blendspace2d.glb");
			GltfImporter::GenerateSkeleton(gltfid, *shanyungSkeleton);
			GltfImporter::GenerateMeshData(gltfid, characterVertexLayout, *shanyungMesh);
			MeshProcessor::RemoveUnusedBones(*shanyungMesh, *shanyungSkeleton);
			{
				Transform shanyungAoBakeTransform;
				shanyungAoBakeTransform.rotation = glm::quat(glm::vec3(glm::radians(180.0f), 0.0f, 0.0f));
				MeshProcessor::TransformMesh(*shanyungMesh, shanyungAoBakeTransform);
				sf::VoxelVolumeData shyvv;
				shyvv.BuildFromMesh(*shanyungMesh, 0.005);
				sf::MeshProcessor::ComputeVertexAmbientOcclusion(*shanyungMesh, &shyvv);
				MeshProcessor::TransformMesh(*shanyungMesh, shanyungAoBakeTransform.Inverse());

				Transform shanyungImportTransform;
				shanyungImportTransform.rotation = glm::quat(glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f));
				shanyungImportTransform.rotation *= glm::quat(glm::vec3(0.0f, 0.0f, glm::radians(180.0f)));
				MeshProcessor::TransformMesh(*shanyungMesh, shanyungImportTransform);
				MeshProcessor::TransformSkeleton(*shanyungSkeleton, shanyungImportTransform);
			}


			bool loadSucceeded = shanyungLodSpec.LoadFromFile("assets/examples/shanyung.lod");
			assert(loadSucceeded);
			shanyungLodGroup.Create(*shanyungMesh, shanyungLodSpec);
			SkinnedMesh& sm = shanyung.AddComponent<SkinnedMesh>(&shanyungLodGroup, shanyungSkeleton, &genericMaterial);
			assert(sm.uniformOverrides.size() == 2);
			sm.uniformOverrides[0]["useColorSampler"].u32 = 1;
			sm.uniformOverrides[0]["colorSampler"].p = &eyeColorBitmap;
			sm.uniformOverrides[1]["color"].p = &skinColor;

			CapsuleCollider& e_cc = shanyung.AddComponent<CapsuleCollider>();
			e_cc.radius = 0.3;
			e_cc.centerA = glm::vec3(0.0f, 0.7f, 0.0f);
			e_cc.centerB = glm::vec3(0.0f, 1.2f, 0.0f);
			shanyungCapsuleSeparation = e_cc.centerA.y - e_cc.radius;

			shanyungWeights.resize(10);
			shanyungSpeedPerAnimation = { {0.0f, 0.0f}, {0.0f, 6.0f}, {-6.0f, 0.0f}, {6.0f, 0.0f}, {0.0f, 1.68f}, {0.0f, -1.08f}, {-0.763f, -0.763f}, {0.763f, -0.763f}, {-1.68f, 0.0f}, {1.68f, 0.0f} };
			shanyungBlendSpace = shanyungSkeleton->AddNodeBlendSpace2D(
				{
					{"Idle", 1.0f, {0.0f, 0.0f}},
					{"Run", 1.0f,
					{0.0f, 1.0f}},
					{"RunLeft", 1.0f,
					{-1.0f, 0.0f}},
					{"RunRight", 1.0f,
					{1.0f, 0.0f}},
					{"Walk", 1.0f,
					{0.0f, 0.5f}},
					{"WalkBack", 1.0f,
					{0.0f, -0.5f}},
					{"WalkBackL", 1.0f,
					{-0.5f, -0.5f}},
					{"WalkBackR", 1.0f,
					{0.5f, -0.5f}},
					{"WalkLeft", 1.0f,
					{-0.5f, 0.0f}},
					{"WalkRight", 1.0f, {0.5f, 0.0f}} },
				{ 0.0f, 0.0f },
				shanyungWeights.data());
			shanyungSkeleton->SetAnimate(true);
		}
		{
			fox = scene.CreateEntity();
			fox.AddComponent<Transform>();

			foxSkeleton = new SkeletonData();
			foxMesh = new MeshData();
			int gltfid = GltfImporter::Load("assets/examples/Fox.glb");
			GltfImporter::GenerateSkeleton(gltfid, *foxSkeleton);
			GltfImporter::GenerateMeshData(gltfid, characterVertexLayout, *foxMesh);
			MeshProcessor::RemoveUnusedBones(*foxMesh, *foxSkeleton);
			{
				Transform foxImportTransform;
				foxImportTransform.rotation = glm::quat(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
				foxImportTransform.scale = 0.01f;
				MeshProcessor::TransformMesh(*foxMesh, foxImportTransform);
				MeshProcessor::TransformSkeleton(*foxSkeleton, foxImportTransform);
				sf::VoxelVolumeData foxvv;
				foxvv.BuildFromMesh(*foxMesh, 0.005);
				sf::MeshProcessor::ComputeVertexAmbientOcclusion(*foxMesh, &foxvv);
			}
			SkinnedMesh& sm = fox.AddComponent<SkinnedMesh>(foxMesh, foxSkeleton, &genericMaterial);
			assert(sm.uniformOverrides.size() == 1);
			sm.uniformOverrides[0]["color"].p = &foxColor;

			foxCol = scene.CreateEntity();
			foxCol.AddComponent<Transform>();
			SphereCollider& col_sc = foxCol.AddComponent<SphereCollider>();
			col_sc.radius = 0.3;
			col_sc.center = glm::vec3(0.0f, 0.6f, 0.0f);
			foxSphereSeparation = col_sc.center.y - col_sc.radius;

			foxWeights.resize(4);
			foxSpeedPerAnimation = { 0.0f, 1.5f, 6.0f };
			foxBlendSpace = foxSkeleton->AddNodeBlendSpace1D({ {"Survey", 1.0f, 0.0f}, {"Walk", 1.0f, 0.5f}, {"Run", 2.3f, 1.0f} }, 0.0f, foxWeights.data());
			foxSkeleton->SetAnimate(true);
		}

		fox.SetEnabled(false);
		foxCol.SetEnabled(false);

		gimbal.GetComponent<Transform>().position = glm::vec3(0.0, GIMBAL_OFFSET_SHANYUNG, 0.0);
		gimbal.GetComponent<Transform>().LookAt(glm::vec3(0.0, 1.0, 1.0), glm::vec3(0.0, 1.0, 0.0));

		cameraObject.GetComponent<Transform>().position = glm::vec3(0.0, GIMBAL_OFFSET_SHANYUNG, cameraDistance);
		cameraObject.GetComponent<Transform>().LookAt(glm::vec3(0.0, GIMBAL_OFFSET_SHANYUNG, 0.0), glm::vec3(0.0, 1.0, 0.0));
	}

	void Game::Terminate()
	{
		scene.DestroyEntity(gimbal);
		scene.DestroyEntity(cameraObject);
		scene.DestroyEntity(shanyung);
		scene.DestroyEntity(fox);
		terrain.Destroy(scene);

		delete shanyungMesh;
		delete shanyungSkeleton;
		delete foxMesh;
		delete foxSkeleton;
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (Input::KeyDown(Input::KeyCode::Left) || Input::KeyDown(Input::KeyCode::Right))
			SwitchCharacter();

		if (shanyung.IsEnabled())
		{
			glm::vec2 targetBlendSpacePos = { 0.0f, 0.0f };
			if (Input::Key(Input::KeyCode::D))
				targetBlendSpacePos += glm::vec2(0.5f, 0.0f);
			if (Input::Key(Input::KeyCode::A))
				targetBlendSpacePos -= glm::vec2(0.5f, 0.0f);
			if (Input::Key(Input::KeyCode::W))
				targetBlendSpacePos += glm::vec2(0.0f, 0.5f);
			if (Input::Key(Input::KeyCode::S))
				targetBlendSpacePos -= glm::vec2(0.0f, 0.5f);
			if (Input::Key(Input::KeyCode::LeftShift) && targetBlendSpacePos.y >= 0.0f)
				targetBlendSpacePos *= 2.0f;

			shanyungBlendSpaceCurrentPos = glm::mix(shanyungBlendSpaceCurrentPos, targetBlendSpacePos, deltaTime * 7.0f);
			shanyungSkeleton->SetBlendSpace2DPosition(shanyungBlendSpace, shanyungBlendSpaceCurrentPos);
			shanyungSkeleton->UpdateAnimation(deltaTime);
			Transform& e_t = shanyung.GetComponent<Transform>();
			glm::vec2 targetSpeed;
			Math::WeightedBlend(shanyungSpeedPerAnimation.data(), shanyungWeights.data(), shanyungWeights.size(), targetSpeed);
			e_t.rotation = glm::slerp(e_t.rotation, glm::quat(glm::vec3(0.0f, glm::radians(180.0f) + targetGimbalRotation.y, 0.0f)), deltaTime * (glm::length(targetSpeed) > 0.01f ? 1.0f : 0.0f) * 4.0f);
			glm::vec3 disp = (e_t.Forward() * targetSpeed.y + e_t.Right() * targetSpeed.x) * deltaTime;

			uint32_t outNCount;
			Geometry::ContactData outCd[4];
			for (int i = 0; i < houses.size(); i++)
			{
				if (EntityIntersect::CapsuleMesh(shanyung, houses[i].entity, 4, &outNCount, outCd))
				{
					for (uint32_t i = 0; i < outNCount; i++)
					{
						glm::vec3 flatCapsuleNormal = glm::normalize(glm::vec3(outCd[i].normalA.x, 0.0f, outCd[i].normalA.z));
						glm::vec3 flatTriNormal = glm::normalize(glm::vec3(outCd[i].normalB.x, 0.0f, outCd[i].normalB.z));

						Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.3f, 0.0f), e_t.position + glm::vec3(0.0f, 0.3f, 0.0f) + flatTriNormal, glm::vec3(0.0f, 0.0f, 1.0f));
						Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.4f, 0.0f), e_t.position + glm::vec3(0.0f, 0.4f, 0.0f) + flatCapsuleNormal, glm::vec3(1.0f, 0.0f, 0.0f));
						if (glm::dot(disp, flatTriNormal) < 0.0f && glm::dot(flatCapsuleNormal, flatTriNormal) < -0.777f)
							disp -= glm::dot(disp, flatTriNormal) * flatTriNormal;
					}
				}
			}
			for (int i = 0; i < houseObjects.size(); i++)
			{
				if (EntityIntersect::CapsuleMesh(shanyung, houseObjects[i], 4, &outNCount, outCd))
				{
					for (uint32_t i = 0; i < outNCount; i++)
					{
						glm::vec3 flatCapsuleNormal = glm::normalize(glm::vec3(outCd[i].normalA.x, 0.0f, outCd[i].normalA.z));
						glm::vec3 flatTriNormal = glm::normalize(glm::vec3(outCd[i].normalB.x, 0.0f, outCd[i].normalB.z));

						Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.3f, 0.0f), e_t.position + glm::vec3(0.0f, 0.3f, 0.0f) + flatTriNormal, glm::vec3(0.0f, 0.0f, 1.0f));
						Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.4f, 0.0f), e_t.position + glm::vec3(0.0f, 0.4f, 0.0f) + flatCapsuleNormal, glm::vec3(1.0f, 0.0f, 0.0f));
						if (glm::dot(disp, flatTriNormal) < 0.0f && glm::dot(flatCapsuleNormal, flatTriNormal) < -0.777f)
							disp -= glm::dot(disp, flatTriNormal) * flatTriNormal;
					}
				}
			}
			for (int i = 0; i < TREE_COUNT; i++)
			{
				if (EntityIntersect::CapsuleCapsule(shanyung, trees[i], outCd))
				{
					glm::vec3 flatNormalA = glm::normalize(glm::vec3(outCd[0].normalA.x, 0.0f, outCd[0].normalA.z));
					glm::vec3 flatNormalB = glm::normalize(glm::vec3(outCd[0].normalB.x, 0.0f, outCd[0].normalB.z));

					Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.3f, 0.0f), e_t.position + glm::vec3(0.0f, 0.3f, 0.0f) + flatNormalA, glm::vec3(0.0f, 0.0f, 1.0f));
					Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.4f, 0.0f), e_t.position + glm::vec3(0.0f, 0.4f, 0.0f) + flatNormalB, glm::vec3(1.0f, 0.0f, 0.0f));
					if (glm::dot(disp, flatNormalB) < 0.0f && glm::dot(flatNormalA, flatNormalB) < -0.777f)
						disp -= glm::dot(disp, flatNormalB) * flatNormalB;
				}
			}
			e_t.position += disp;

			float outT = CharacterStepCastShanyung(shanyung);
			if (outT <= (shanyungVerticalSpeed != 0.0f ? shanyungCapsuleSeparation : (shanyungCapsuleSeparation + 0.3f)))
			{
				e_t.position.y += shanyungCapsuleSeparation - outT;
				shanyungVerticalSpeed = 0.0f;
			}
			else
			{
				shanyungVerticalSpeed += 9.8f * deltaTime;
				e_t.position.y -= shanyungVerticalSpeed * deltaTime;
			}

			UpdateCamera(deltaTime, shanyung, GIMBAL_OFFSET_SHANYUNG);
		}
		else if (fox.IsEnabled())
		{
			glm::vec2 inputVector = { 0.0f, 0.0f };
			if (Input::Key(Input::KeyCode::A))
				inputVector.x = -1.0f;
			if (Input::Key(Input::KeyCode::D))
				inputVector.x = 1.0f;
			if (Input::Key(Input::KeyCode::S))
				inputVector.y = -1.0f;
			if (Input::Key(Input::KeyCode::W))
				inputVector.y = 1.0f;
			glm::normalize(inputVector);
			float targetBlendSpaceX = 0.0f;
			if (Input::Key(Input::KeyCode::A) || Input::Key(Input::KeyCode::D) || Input::Key(Input::KeyCode::W) || Input::Key(Input::KeyCode::S))
				targetBlendSpaceX += 0.5;
			if (Input::Key(Input::KeyCode::LeftShift))
				targetBlendSpaceX *= 2.0f;

			foxBlendSpaceCurrentX = glm::mix(foxBlendSpaceCurrentX, targetBlendSpaceX, deltaTime * 7.0f);
			foxSkeleton->SetBlendSpace1DPosition(foxBlendSpace, foxBlendSpaceCurrentX);
			foxSkeleton->UpdateAnimation(deltaTime);
			Transform& e_t = fox.GetComponent<Transform>();

			float targetSpeed;
			Math::WeightedBlend(foxSpeedPerAnimation.data(), foxWeights.data(), foxWeights.size(), targetSpeed);
			glm::vec3 camForwardFlat = cameraObject.GetComponent<Transform>().Forward();
			camForwardFlat.y = 0;
			glm::normalize(camForwardFlat);
			glm::vec3 camRightFlat = glm::cross(camForwardFlat, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec3 disp = e_t.Forward() * targetSpeed * deltaTime;

			bool collidedWithWall = false;
			uint32_t outNCount;
			Geometry::ContactData outCd[4];
			for (int i = 0; i < houses.size(); i++)
			{
				if (EntityIntersect::SphereMesh(foxCol, houses[i].entity, 4, &outNCount, outCd))
				{
					for (uint32_t i = 0; i < outNCount; i++)
					{
						glm::vec3 flatSphereNormal = glm::normalize(glm::vec3(outCd[i].normalA.x, 0.0f, outCd[i].normalA.z));
						glm::vec3 flatTriNormal = glm::normalize(glm::vec3(outCd[i].normalB.x, 0.0f, outCd[i].normalB.z));

						Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.3f, 0.0f), e_t.position + glm::vec3(0.0f, 0.3f, 0.0f) + flatTriNormal, glm::vec3(0.0f, 0.0f, 1.0f));
						Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.4f, 0.0f), e_t.position + glm::vec3(0.0f, 0.4f, 0.0f) + flatSphereNormal, glm::vec3(1.0f, 0.0f, 0.0f));
						if (glm::dot(disp, flatTriNormal) < 0.0f && glm::dot(flatSphereNormal, flatTriNormal) < -0.777f)
						{
							disp -= glm::dot(disp, flatTriNormal) * flatTriNormal;
							collidedWithWall = true;
						}
					}
				}
			}
			for (int i = 0; i < houseObjects.size(); i++)
			{
				if (EntityIntersect::SphereMesh(foxCol, houseObjects[i], 4, &outNCount, outCd))
				{
					for (uint32_t i = 0; i < outNCount; i++)
					{
						glm::vec3 flatCapsuleNormal = glm::normalize(glm::vec3(outCd[i].normalA.x, 0.0f, outCd[i].normalA.z));
						glm::vec3 flatTriNormal = glm::normalize(glm::vec3(outCd[i].normalB.x, 0.0f, outCd[i].normalB.z));

						Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.3f, 0.0f), e_t.position + glm::vec3(0.0f, 0.3f, 0.0f) + flatTriNormal, glm::vec3(0.0f, 0.0f, 1.0f));
						Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.4f, 0.0f), e_t.position + glm::vec3(0.0f, 0.4f, 0.0f) + flatCapsuleNormal, glm::vec3(1.0f, 0.0f, 0.0f));
						if (glm::dot(disp, flatTriNormal) < 0.0f && glm::dot(flatCapsuleNormal, flatTriNormal) < -0.777f)
							disp -= glm::dot(disp, flatTriNormal) * flatTriNormal;
					}
				}
			}
			for (int i = 0; i < TREE_COUNT; i++)
			{
				if (EntityIntersect::SphereCapsule(foxCol, trees[i], outCd))
				{
					glm::vec3 flatNormalA = glm::normalize(glm::vec3(outCd[0].normalA.x, 0.0f, outCd[0].normalA.z));
					glm::vec3 flatNormalB = glm::normalize(glm::vec3(outCd[0].normalB.x, 0.0f, outCd[0].normalB.z));

					Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.3f, 0.0f), e_t.position + glm::vec3(0.0f, 0.3f, 0.0f) + flatNormalA, glm::vec3(0.0f, 0.0f, 1.0f));
					Renderer::Debug::AddLine(e_t.position + glm::vec3(0.0f, 0.4f, 0.0f), e_t.position + glm::vec3(0.0f, 0.4f, 0.0f) + flatNormalB, glm::vec3(1.0f, 0.0f, 0.0f));
					if (glm::dot(disp, flatNormalB) < 0.0f && glm::dot(flatNormalA, flatNormalB) < -0.777f)
						disp -= glm::dot(disp, flatNormalB) * flatNormalB;
				}
			}
			e_t.position += disp;

			Transform& col_t = foxCol.GetComponent<Transform>();
			col_t.position = e_t.position;
			float outT = CharacterStepCastFox(foxCol);
			if (outT <= (foxVerticalSpeed != 0.0f ? foxSphereSeparation : (foxSphereSeparation + 0.3f)))
			{
				e_t.position.y += foxSphereSeparation - outT;
				foxVerticalSpeed = 0.0f;

				// sample again for inclination
				col_t.position = e_t.position + e_t.Forward() * 0.001f;
				outT = CharacterStepCastFox(foxCol);
				col_t.position.y += foxSphereSeparation - outT;

				glm::vec3 deltaForward = col_t.position - e_t.position;
				if (collidedWithWall)
					deltaForward.y = 0.0f;

				if (targetBlendSpaceX > 0.00001f)
					e_t.rotation = glm::slerp(e_t.rotation,
						glm::quatLookAt(camForwardFlat * inputVector.y + camRightFlat * inputVector.x, glm::vec3(0.0f, 1.0f, 0.0f)) *
						glm::quatLookAt(glm::normalize(glm::vec3(0.0f, deltaForward.y, -0.001f)),
							glm::vec3(0.0f, 1.0f, 0.0f)), deltaTime * 5.0f);

			}
			else
			{
				foxVerticalSpeed += 9.8f * deltaTime;
				e_t.position.y -= foxVerticalSpeed * deltaTime;
			}

			UpdateCamera(deltaTime, fox, GIMBAL_OFFSET_FOX);
		}
	}

	void Game::ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Character"))
			{
				if (ImGui::MenuItem("Shanyung", "Right/Left arrow", shanyung.IsEnabled()))
					SwitchCharacter();
				if (ImGui::MenuItem("Fox", "Right/Left arrow", fox.IsEnabled()))
					SwitchCharacter();
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
}