#include "ImGuiController.h"

#include <string>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imnodes.h>

#include <Window.h>
#include <Game.h>
#include <Input.h>
#include <Debug.h>

#include <Renderer/RenderTarget.h>
#include <Renderer/Renderer.h>
#include <Renderer/ImGuiDisplayPanel.h>

#include <Scene/Scene.h>
#include <Scene/Entity.h>

namespace sf::ImGuiController
{
	bool statsEnabled = false;
	bool logsEnabled = false;
	bool debugDrawEnabled = false;
	Window* window;
	std::vector<ImGuiDisplayPanel*> displayPanels;
	uint32_t lockedDisplayPanel = ~0;
	uint32_t hoveredDisplayPanel = ~0;

	void DrawDisplayPanel(uint32_t panelId, float deltaTime)
	{
		sf::Renderer::Framebuffer dpFramebuffer = displayPanels[panelId]->GetFramebufferToDraw();
		sf::Renderer::DrawFramebuffer(dpFramebuffer, deltaTime);
	}
}

void sf::ImGuiController::Initialize(Window& window, const Game::InitData& gameInitData)
{
	bool rendererInitializedSuccessfully = sf::Renderer::Initialize(window, gameInitData);
	assert(rendererInitializedSuccessfully);

	ImGuiController::window = &window;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImNodes::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
#ifdef SF_PLATFORM_WINDOWS
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
#endif

	// Setup Platform/Renderer bindings
	window.ImGuiInitForOpenGL(ImGui_ImplGlfw_InitForOpenGL);
	ImGui_ImplOpenGL3_Init();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	auto f = io.Fonts->AddFontFromFileTTF("assets/fonts/FiraCode/FiraCode-Regular.ttf", 15.0f);
}

uint32_t sf::ImGuiController::CreateDisplayPanel(const char* name, uint32_t sampleCount)
{
	displayPanels.push_back(new ImGuiDisplayPanel());
	displayPanels.back()->Create(name, DrawDisplayPanel, sampleCount, true);
	return (uint32_t) displayPanels.size() - 1;
}

sf::Renderer::Framebuffer sf::ImGuiController::GetDisplayPanelFramebufferToDraw(uint32_t displayPanelId)
{
	assert(displayPanelId < displayPanels.size());
	return displayPanels[displayPanelId]->GetFramebufferToDraw();
}

uint32_t sf::ImGuiController::GetActiveDisplayPanel()
{
	if (lockedDisplayPanel != ~0)
		return lockedDisplayPanel;
	return hoveredDisplayPanel;
}

void sf::ImGuiController::LockActiveDisplayPanel(uint32_t displayPanelId)
{
	lockedDisplayPanel = displayPanelId;
}

void sf::ImGuiController::UnlockActiveDisplayPanel()
{
	lockedDisplayPanel = ~0;
}

void sf::ImGuiController::Terminate()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImNodes::DestroyContext();
	ImGui::DestroyContext();
	sf::Renderer::Terminate();
}

void sf::ImGuiController::Tick(float deltaTime)
{
	/* Draw scenes */
	sf::Renderer::DrawFramebuffer(window->GetFramebufferToDraw(), deltaTime);

	if (Input::KeyDown(Input::Escape))
		window->SetToolBarEnabled(!window->GetToolBarEnabled());
	if (Input::KeyDown(Input::F2))
		window->SetCursorRequired(!window->GetCursorRequired());
	if (Input::KeyDown(Input::F3))
		window->SetVsyncEnabled(!window->GetVsyncEnabled());
	if (Input::KeyDown(Input::Enter) && (Input::Key(Input::LeftAlt) || Input::Key(Input::RightAlt)))
		window->SetFullScreenEnabled(!window->GetFullScreenEnabled());

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	{
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
		static ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", nullptr, window_flags);
		ImGui::PopStyleVar(3);
		ImGuiID dockspace_id = ImGui::GetID("DockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		ImGui::End();
	}

	hoveredDisplayPanel = ~0;
	bool cursorHover;
	for (uint32_t i = 0; i < displayPanels.size(); i++)
	{
		displayPanels[i]->ImGuiCall(ImGui::GetIO(), cursorHover, i, deltaTime);
		if (cursorHover)
			hoveredDisplayPanel = i;
	}
	Input::SetEnabled(!ImGui::GetIO().WantCaptureMouse || GetActiveDisplayPanel() != ~0);
	sf::Renderer::FrameEnd();

	if (window->GetToolBarEnabled())
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("sf"))
			{
				ImGui::MenuItem("Stats", NULL, &statsEnabled);
				ImGui::MenuItem("Logs", NULL, &logsEnabled);
				if (ImGui::MenuItem("Debug Draw", NULL, &debugDrawEnabled))
					Renderer::SetDebugDrawEnabled(debugDrawEnabled);
				if (ImGui::MenuItem("Menu bar", "Esc", window->GetToolBarEnabled()))
					window->SetToolBarEnabled(false);
				if (ImGui::MenuItem("Cursor required", "F2", window->GetCursorRequired()))
					window->SetCursorRequired(!window->GetCursorRequired());
				if (ImGui::MenuItem("Vsync enabled", "F3", window->GetVsyncEnabled()))
					window->SetVsyncEnabled(!window->GetVsyncEnabled());
				if (ImGui::MenuItem("Fullscreen", "Alt+Enter", window->GetFullScreenEnabled()))
					window->SetFullScreenEnabled(!window->GetFullScreenEnabled());
				if (ImGui::MenuItem("Restart"))
				{
					Game::Terminate();
					Game::Initialize(0, nullptr);
				}
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
	if (logsEnabled)
	{
		ImGui::Begin("Logs");

		Debug::LogSeekBegin();
		const Debug::LogInfo* currentLog;
		while (Debug::LogGetNext(currentLog))
		{
			uint32_t imguiColor = (currentLog->color >> 24) | ((currentLog->color & 0x00ff0000) >> 8) | ((currentLog->color & 0x0000ff00) << 8) | ((currentLog->color & 0x000000ff) << 24);
			ImGui::PushStyleColor(ImGuiCol_Text, imguiColor);
			ImGui::TextUnformatted(currentLog->text);
			ImGui::PopStyleColor();
		}
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);
		ImGui::End();
	}

	//-------------------//
	if (window->GetToolBarEnabled())
		sf::Game::ImGuiCall();
	//-------------------//

	// Render dear imgui into screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#ifdef SF_PLATFORM_WINDOWS
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		window->HandleImGuiViewports(ImGui::UpdatePlatformWindows, ImGui::RenderPlatformWindowsDefault);
#endif
}
