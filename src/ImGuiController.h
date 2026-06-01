#pragma once

#include <Window.h>

namespace sf::ImGuiController
{
	void Initialize(Window& window, const Game::InitData& gameInitData);
	uint32_t CreateDisplayPanel(const char* name, uint32_t sampleCount = 4);
	Renderer::Framebuffer GetDisplayPanelFramebufferToDraw(uint32_t displayPanelId);
	uint32_t GetActiveDisplayPanel();
	void LockActiveDisplayPanel(uint32_t displayPanelId);
	void UnlockActiveDisplayPanel();

	void Terminate();
	void Tick(float deltaTime);
}
