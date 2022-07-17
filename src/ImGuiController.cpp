#include "ImGuiController.h"

#include <string>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <Game.h>
#include <Config.h>
#include <Input.h>

namespace sf::ImGuiController
{
	bool statsEnabled;
}

void sf::ImGuiController::Setup(GLFWwindow* window)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	auto f = io.Fonts->AddFontFromFileTTF("assets/fonts/FiraCode/FiraCode-Regular.ttf", 15.0f);
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
	if (Input::KeyDown(Input::Enter) && Input::Key(Input::RightAlt))
		Config::SetFullscreen(!Config::GetFullscreen());

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	if (Config::GetImGuiBarEnabled())
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("sf"))
			{
				ImGui::MenuItem("Stats", NULL, &statsEnabled);
				if (ImGui::MenuItem("Fullscreen", "Alt+Enter"))
					Config::SetFullscreen(!Config::GetFullscreen());
				if (ImGui::MenuItem("Cursor enabled", "F2"))
					Config::SetCursorEnabled(!Config::GetCursorEnabled());
				if (ImGui::MenuItem("Menu bar", "F1"))
					Config::SetImGuiBarEnabled(false);
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
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}
