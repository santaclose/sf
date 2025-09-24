#pragma once

#include <string>
#include <glm/glm.hpp>

namespace sf::Game
{
	struct InitData
	{
		const char* windowTitle = "";
		glm::uvec2 windowSize = { 1280, 720 };
		bool windowCursorEnabled = true;
		bool windowFullscreenEnabled = false;
		uint32_t msaaCount = 4;
		glm::vec3 clearColor = { 1.0f, 1.0f, 1.0f };
		bool toolBarEnabled = true;
		bool vsyncEnabled = true;
	};

	InitData GetInitData();
	void Initialize(int argc, char** argv);
	void Terminate();
	void OnUpdate(float deltaTime, float time);
	void ImGuiCall();
}