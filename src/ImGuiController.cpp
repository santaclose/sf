#include "ImGuiController.h"

#include <string>
#include <imgui.h>

#include <Game.h>
#include <Config.h>
#include <Input.h>

#include <Renderer/ImGuiBind.h>

namespace sf::ImGuiController
{
	bool statsEnabled;
	Renderer::ImGuiBind imGuiBind;
}

void sf::ImGuiController::Initialize(GLFWwindow* window)
{
	imGuiBind.Initialize(window);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	auto f = io.Fonts->AddFontFromFileTTF("assets/fonts/FiraCode/FiraCode-Regular.ttf", 15.0f);

	imGuiBind.AfterConfigure(window);
}

void sf::ImGuiController::Terminate()
{
	imGuiBind.Terminate();
}

bool sf::ImGuiController::HasControl()
{
	return ImGui::GetIO().WantCaptureMouse;
}

void sf::ImGuiController::Tick(float deltaTime)
{
	if (Input::KeyDown(Input::F1))
		Config::SetImGuiBarEnabled(!Config::GetImGuiBarEnabled());
	if (Input::KeyDown(Input::F2))
		Config::SetCursorEnabled(!Config::GetCursorEnabled());
	if (Input::KeyDown(Input::F3))
		Config::SetVsyncEnabled(!Config::GetVsyncEnabled());
	if (Input::KeyDown(Input::Enter) && Input::Key(Input::RightAlt))
		Config::SetFullscreen(!Config::GetFullscreen());

	imGuiBind.NewFrame();
	ImGui::NewFrame();

	if (Config::GetImGuiBarEnabled())
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("sf"))
			{
				ImGui::MenuItem("Stats", NULL, &statsEnabled);
				if (ImGui::MenuItem("Fullscreen", "Alt+Enter", Config::GetFullscreen()))
					Config::SetFullscreen(!Config::GetFullscreen());
				if (ImGui::MenuItem("Menu bar", "F1", Config::GetImGuiBarEnabled()))
					Config::SetImGuiBarEnabled(false);
				if (ImGui::MenuItem("Cursor enabled", "F2", Config::GetCursorEnabled()))
					Config::SetCursorEnabled(!Config::GetCursorEnabled());
				if (ImGui::MenuItem("Vsync enabled", "F3", Config::GetVsyncEnabled()))
					Config::SetVsyncEnabled(!Config::GetVsyncEnabled());
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
	if (statsEnabled)
	{
		ImGui::Begin("Stats");
		ImGui::Text("Frame time: %.3f ms", 1000.0f * deltaTime);
		ImGui::Text("FPS: %.1f", 1.0f / deltaTime);
		ImGui::End();
	}

	//-------------------//
	sf::Game::ImGuiCall();
	//-------------------//

	// Render dear imgui into screen
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
	if (!is_minimized)
		imGuiBind.FrameRender(draw_data);

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}

void sf::ImGuiController::OnResize()
{
	imGuiBind.OnResize();
}
