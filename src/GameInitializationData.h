#pragma once

#include <string>
#include <glm/glm.hpp>

namespace sf
{
	struct GameInitializationData
	{
		GameInitializationData() = default;
		GameInitializationData(const std::string& filePath);
		std::string windowTitle = "sfgame";
		glm::uvec2 windowSize = { 1280, 720 };
		bool windowCursorEnabled = true;
		bool windowFullscreenEnabled = false;
		uint32_t msaaCount = 4;
		glm::vec3 clearColor = { 1.0f, 1.0f, 1.0f };
		bool toolBarEnabled = true;
		bool vsyncEnabled = true;
	};
}