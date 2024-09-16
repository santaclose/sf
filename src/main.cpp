#include <glm/glm.hpp>
#include <filesystem>
#include <iostream>
#include <vector>

#include <Window.h>
#include <Input.h>
#include <Config.h>
#include <Game.h>
#include <Defaults.h>
#include <Renderer/Renderer.h>

#include <Scene/Scene.h>
#include <Scene/Entity.h>

#include <Components/Base.h>
#include <Components/Camera.h>
#include <Components/Transform.h>
#include <Components/Mesh.h>
#include <Components/SkinnedMesh.h>
#include <Components/ScreenCoordinates.h>
#include <Components/Sprite.h>
#include <Components/Skeleton.h>

//#include <ImGuiController.h>

#define BACKGROUND_COLOR 0.1

float gameTime = 0.0;
double lastFrameTime = 0.0;
double currentFrameTime = 0.0;
double deltaTime = 0.0;
bool deltaTimeLock = true;

int main(int argc, char** argv)
{
	if (!std::filesystem::is_directory("assets"))
	{
		std::filesystem::current_path("../../../");
		std::cout << "[main] Adjusting working directory\n";
	}

	sf::Config::LoadFromFile(sf::Game::ConfigFilePath);
	sf::Window window = sf::Window(sf::Config::GetName().c_str(), sf::Config::GetWindowSize(), sf::Config::GetFullscreen(), sf::Config::GetCursorEnabled(), sf::Config::GetVsyncEnabled());

	sf::Defaults::Initialize();
	//-------------------//
	sf::Game::Initialize(argc, argv);
	//-------------------//

	if (!sf::Renderer::Initialize(window))
		std::cout << "[main] Failed to initialize renderer\n";

	//sf::ImGuiController::Initialize(window);

	/* Loop until the user closes the window */
	while (!window.ShouldClose())
	{
		if (deltaTimeLock)
		{
			currentFrameTime = lastFrameTime = window.GetTime();
			deltaTimeLock = false;
		}
		else
			currentFrameTime = window.GetTime();
		deltaTime = currentFrameTime - lastFrameTime;

		//-------------------//
		sf::Game::OnUpdate(deltaTime, gameTime);
		//-------------------//

		gameTime += deltaTime;

		/* Draw scene */
		sf::Renderer::Predraw();
		sf::Renderer::DrawSkybox();
		auto meshRenderView = sf::Scene::activeScene->GetRegistry().view<sf::Base, sf::Mesh, sf::Transform>();
		for (auto entity : meshRenderView)
		{
			auto [base, mesh, transform] = meshRenderView.get<sf::Base, sf::Mesh, sf::Transform>(entity);
			if (base.isEntityEnabled)
				sf::Renderer::DrawMesh(mesh, transform);
		}
		auto skinnedMeshRenderView = sf::Scene::activeScene->GetRegistry().view<sf::Base, sf::SkinnedMesh, sf::Transform>();
		for (auto entity : skinnedMeshRenderView)
		{
			auto [base, mesh, transform] = skinnedMeshRenderView.get<sf::Base, sf::SkinnedMesh, sf::Transform>(entity);
			if (base.isEntityEnabled)
				sf::Renderer::DrawSkinnedMesh(mesh, transform);
		}
		auto voxelBoxRenderView = sf::Scene::activeScene->GetRegistry().view<sf::Base, sf::VoxelBox, sf::Transform>();
		for (auto entity : voxelBoxRenderView)
		{
			auto [base, voxelBox, transform] = voxelBoxRenderView.get<sf::Base, sf::VoxelBox, sf::Transform>(entity);
			if (base.isEntityEnabled)
				sf::Renderer::DrawVoxelBox(voxelBox, transform);
		}
		auto skeletonRenderView = sf::Scene::activeScene->GetRegistry().view<sf::Base, sf::Skeleton, sf::Transform>();
		for (auto entity : skeletonRenderView)
		{
			auto [base, skeleton, transform] = skeletonRenderView.get<sf::Base, sf::Skeleton, sf::Transform>(entity);
			if (base.isEntityEnabled)
				sf::Renderer::DrawSkeleton(skeleton, transform);
		}
		auto spriteRenderView = sf::Scene::activeScene->GetRegistry().view<sf::Base, sf::Sprite, sf::ScreenCoordinates>();
		for (auto entity : spriteRenderView)
		{
			auto [base, sprite, screenCooordinates] = spriteRenderView.get<sf::Base, sf::Sprite, sf::ScreenCoordinates>(entity);
			if (base.isEntityEnabled)
				sf::Renderer::DrawSprite(sprite, screenCooordinates);
		}
		sf::Renderer::Postdraw();

		//sf::ImGuiController::Tick(deltaTime);
		window.SwapBuffers();

		sf::Input::FrameEnd();
		window.PollEvents();

		lastFrameTime = currentFrameTime;
	}

	//-------------------//
	sf::Game::Terminate();
	//-------------------//

	//sf::ImGuiController::Terminate();
	sf::Renderer::Terminate();
	return 0;
}