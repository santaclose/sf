#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <iostream>
#include <vector>

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

#include <ImGuiController.h>

#define BACKGROUND_COLOR 0.1

float gameTime = 0.0;
double lastFrameTime = 0.0;
double currentFrameTime = 0.0;
double deltaTime = 0.0;
bool deltaTimeLock = true;

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	sf::Input::UpdateMousePosition(xpos, ypos);
}
void mouse_button_callback(GLFWwindow*, int button, int action, int mods)
{
	sf::Input::UpdateMouseButtons(button, action);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	sf::Input::UpdateKeyboard(key, action);
}
void character_callback(GLFWwindow* window, uint32_t codepoint)
{
	sf::Input::UpdateCharacter(codepoint);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	sf::Input::UpdateMouseScroll(xoffset, yoffset);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	sf::Config::UpdateWindowSize(width, height);
	sf::ImGuiController::OnResize();
	sf::Renderer::OnResize();
}

int main(int argc, char** argv)
{
	if (!std::filesystem::is_directory("assets"))
	{
		std::filesystem::current_path("../../../");
		std::cout << "Adjusting working directory\n";
	}

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	sf::Config::LoadFromFile(sf::Game::ConfigFilePath);
#ifdef SF_USE_VULKAN
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
#ifdef SF_USE_OPENGL
	glfwWindowHint(GLFW_SAMPLES, sf::Config::GetMsaaCount());
#ifdef SF_DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
#endif

	window = glfwCreateWindow(sf::Config::GetWindowSize().x, sf::Config::GetWindowSize().y, sf::Config::GetName().c_str(), sf::Config::GetFullscreen() ? glfwGetPrimaryMonitor() : NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	sf::Config::UpdateWindow(window);

#if SF_USE_VULKAN
	if (!glfwVulkanSupported())
	{
		printf("GLFW: Vulkan Not Supported\n");
		return 1;
	}
#endif

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, sf::Config::GetCursorEnabled() ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

	/* INPUT BINDINGS */
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCharCallback(window, character_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(sf::Config::GetVsyncEnabled());

	sf::ImGuiController::Initialize(window);

	if (!sf::Renderer::Initialize(glfwGetProcAddress))
		std::cout << "Failed to initialize renderer\n";
	sf::Defaults::Initialize();
	//-------------------//
	sf::Game::Initialize(argc, argv);
	//-------------------//

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		if (deltaTimeLock)
		{
			currentFrameTime = lastFrameTime = glfwGetTime();
			deltaTimeLock = false;
		}
		else
			currentFrameTime = glfwGetTime();
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

		sf::ImGuiController::Tick(deltaTime);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		sf::Input::FrameEnd();
		/* Poll for and process events */
		glfwPollEvents();

		lastFrameTime = currentFrameTime;
	}

	//-------------------//
	sf::Game::Terminate();
	//-------------------//

	sf::ImGuiController::Terminate();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}