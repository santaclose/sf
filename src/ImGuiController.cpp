#include "ImGuiController.h"

#include <string>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <Window.h>
#include <Game.h>
#include <Input.h>

namespace sf::ImGuiController
{
	bool statsEnabled;
	Window* window;
}

void sf::ImGuiController::Initialize(Window& window)
{
	ImGuiController::window = &window;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Platform/Renderer bindings
	window.ImGuiInitForOpenGL(ImGui_ImplGlfw_InitForOpenGL);
	ImGui_ImplOpenGL3_Init();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	auto f = io.Fonts->AddFontFromFileTTF("assets/fonts/FiraCode/FiraCode-Regular.ttf", 15.0f);
}

void sf::ImGuiController::Terminate()
{
}

bool sf::ImGuiController::HasControl()
{
	return ImGui::GetIO().WantCaptureMouse;
}

void sf::ImGuiController::Tick(float deltaTime)
{
	if (Input::KeyDown(Input::F1))
		window->SetToolBarEnabled(!window->GetToolBarEnabled());
	if (Input::KeyDown(Input::F2))
		window->SetCursorEnabled(!window->GetCursorEnabled());
	if (Input::KeyDown(Input::F3))
		window->SetVsyncEnabled(!window->GetVsyncEnabled());
	if (Input::KeyDown(Input::Enter) && (Input::Key(Input::LeftAlt) || Input::Key(Input::RightAlt)))
		window->SetFullScreenEnabled(!window->GetFullScreenEnabled());

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	if (window->GetToolBarEnabled())
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("sf"))
			{
				ImGui::MenuItem("Stats", NULL, &statsEnabled);
				if (ImGui::MenuItem("Menu bar", "F1", window->GetToolBarEnabled()))
					window->SetToolBarEnabled(false);
				if (ImGui::MenuItem("Cursor enabled", "F2", window->GetCursorEnabled()))
					window->SetCursorEnabled(!window->GetCursorEnabled());
				if (ImGui::MenuItem("Vsync enabled", "F3", window->GetVsyncEnabled()))
					window->SetVsyncEnabled(!window->GetVsyncEnabled());
				if (ImGui::MenuItem("Fullscreen", "Alt+Enter", window->GetFullScreenEnabled()))
					window->SetFullScreenEnabled(!window->GetFullScreenEnabled());
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
	if (window->GetToolBarEnabled())
		sf::Game::ImGuiCall();
	//-------------------//

	// Render dear imgui into screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		window->HandleImGuiViewports(ImGui::UpdatePlatformWindows, ImGui::RenderPlatformWindowsDefault);
}
