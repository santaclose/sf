#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>
#include <fstream>

#include <Defaults.h>
#include <Config.h>
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
	std::string Game::ConfigFilePath = "examples/animation/config.json";

	Scene scene;
	Entity gimbal, cameraObject;
	std::vector<Entity> galleryObjects;

	glm::vec3 targetGimbalRotation = glm::vec3(0.0, glm::radians(180.0f), 0.0);

	glm::quat modelRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	float modelRotationY = 0.0f;

	float cameraDistance = 3.0;
	bool rotationEnabled = false;

	SkeletonData* skeletons;
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
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/Fox/glTF-Binary/Fox.glb",
			"https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/BrainStem/glTF-Binary/BrainStem.glb"
			}, "examples/animation/");

		gimbal = scene.CreateEntity();
		cameraObject = scene.CreateEntity();
		Renderer::activeCameraEntity = cameraObject;

		gimbal.AddComponent<Transform>();

		cameraObject.AddComponent<Camera>();
		cameraObject.AddComponent<Transform>();

		int gltfid;
		skeletons = new SkeletonData[2];
		meshes = new MeshData[2];
		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& e_t = galleryObjects.back().AddComponent<Transform>();
			e_t.scale = 0.025f;
			e_t.position.y -= 1.0f;

			gltfid = GltfImporter::Load("examples/animation/Fox.glb");
			GltfImporter::GenerateSkeleton(gltfid, skeletons[0]);
			meshes[0].ChangeVertexLayout(Defaults::defaultSkinningVertexLayout);
			GltfImporter::GenerateMeshData(gltfid, meshes[0]);
			MeshProcessor::ComputeNormals(meshes[0]);
			Mesh& objectMesh = galleryObjects.back().AddComponent<SkinnedMesh>(&(meshes[0]), &(skeletons[0]));
		}
		{
			galleryObjects.push_back(scene.CreateEntity());
			Transform& e_t = galleryObjects.back().AddComponent<Transform>();
			e_t.rotation = glm::quat(glm::vec3(glm::radians(-90.0f), 0.0f, 0.0f));
			e_t.scale = 1.7f;
			e_t.position.y -= 1.5f;

			gltfid = GltfImporter::Load("examples/animation/BrainStem.glb");
			GltfImporter::GenerateSkeleton(gltfid, skeletons[1]);
			meshes[1].ChangeVertexLayout(Defaults::defaultSkinningVertexLayout);
			GltfImporter::GenerateMeshData(gltfid, meshes[1]);
			Mesh& objectMesh = galleryObjects.back().AddComponent<SkinnedMesh>(&(meshes[1]), &(skeletons[1]));
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

	void GalleryChange(bool next)
	{
		int prevModel = selectedModel;
		selectedModel += next ? 1 : -1;
		selectedModel = Math::Mod(selectedModel, (int)galleryObjects.size());
		galleryObjects[prevModel].SetEnabled(false);
		galleryObjects[selectedModel].SetEnabled(true);
	}

	void AnimationChange(bool next)
	{
		skeletons[selectedModel].animationIndex += next ? 1 : -1;
		skeletons[selectedModel].animationIndex = Math::Mod(skeletons[selectedModel].animationIndex, skeletons[selectedModel].animations.size());
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
		if (Input::KeyDown(Input::KeyCode::Space))
			rotationEnabled = !rotationEnabled;
		else if (Input::KeyDown(Input::KeyCode::Right))
			GalleryChange(true);
		else if (Input::KeyDown(Input::KeyCode::Left))
			GalleryChange(false);
		else if (Input::KeyDown(Input::KeyCode::KP5))
			skeletons[selectedModel].animate = !skeletons[selectedModel].animate;
		else if (Input::KeyDown(Input::KeyCode::KP6))
			AnimationChange(true);
		else if (Input::KeyDown(Input::KeyCode::KP4))
			AnimationChange(false);

		if (skeletons[selectedModel].animate)
		{
			skeletons[selectedModel].animationTime += deltaTime;
			skeletons[selectedModel].ClampAnimationTime();
			skeletons[selectedModel].UpdateAnimation();
		}

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
		if (Config::GetImGuiBarEnabled())
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
				if (ImGui::BeginMenu("Animation"))
				{
					if (ImGui::MenuItem("Animate", "NumPad5")) { skeletons[selectedModel].animate = !skeletons[selectedModel].animate; }
					if (ImGui::MenuItem("Previous", "NumPad4")) { AnimationChange(false); }
					if (ImGui::MenuItem("Next", "NumPad6")) { AnimationChange(true); }
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
		}
	}
	void Game::Terminate()
	{
		delete[] meshes;
	}
}