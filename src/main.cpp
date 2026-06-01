#include <glm/glm.hpp>
#include <filesystem>
#include <iostream>
#include <vector>

#include <Window.h>
#include <Input.h>
#include <Game.h>
#include <Defaults.h>

#include <Components/Base.h>
#include <Components/Camera.h>
#include <Components/Transform.h>
#include <Components/Mesh.h>
#include <Components/SkinnedMesh.h>
#include <Components/ScreenCoordinates.h>
#include <Components/Sprite.h>
#include <Components/SphereCollider.h>
#include <Components/CapsuleCollider.h>
#include <Components/BoxCollider.h>

#include <ImGuiController.h>

#define MAX_DELTA_TIME 1.0f

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
		std::cout << "Adjusting working directory\n";
	}

	sf::Game::InitData gameInitData = sf::Game::GetInitData();
	sf::Window window = sf::Window(gameInitData);
	sf::ImGuiController::Initialize(window, gameInitData);

	//-------------------//
	sf::Game::Initialize(argc, argv);
	//-------------------//

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

		sf::ImGuiController::Tick(deltaTime);
		window.SwapBuffers();

		sf::Input::FrameEnd();
		window.PollEvents();

		lastFrameTime = currentFrameTime;
	}

	//-------------------//
	sf::Game::Terminate();
	//-------------------//

	sf::ImGuiController::Terminate();
	sf::Window::Terminate();
	return 0;
}