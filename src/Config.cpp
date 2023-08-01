#include "Config.h"

#include <iostream>
#include <fstream>
#include <json.hpp>

namespace sf::Config
{
	std::string name = "sfgame";
	glm::uvec2 windowSize = { 1280, 720 };
	uint32_t msaaCount = 4;
	glm::vec4 clearColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool fullscreen = false;
	bool imguiBarEnabled = true;
	bool cursorEnabled = true;
	bool vsyncEnabled = true;
}

void sf::Config::LoadFromFile(const std::string& filePath)
{
	std::ifstream i(filePath);
	if (!i.good())
	{
		std::cout << "[Config] Config file not found\n";
		return;
	}
	nlohmann::json j;
	i >> j;
	if (j.find("name") != j.end())
		name = j["name"];
	if (j.find("windowWidth") != j.end() && j.find("windowHeight") != j.end())
		windowSize = { j["windowWidth"], j["windowHeight"] };
	if (j.find("msaaCount") != j.end())
		msaaCount = j["msaaCount"];
	if (j.find("fullscreen") != j.end())
		fullscreen = j["fullscreen"];
	if (j.find("clearColorR") != j.end())
		clearColor.r = j["clearColorR"];
	if (j.find("clearColorG") != j.end())
		clearColor.g = j["clearColorG"];
	if (j.find("clearColorB") != j.end())
		clearColor.b = j["clearColorB"];
	if (j.find("imguiBarEnabled") != j.end())
		imguiBarEnabled = j["imguiBarEnabled"];
	if (j.find("cursorEnabled") != j.end())
		cursorEnabled = j["cursorEnabled"];
	if (j.find("vsyncEnabled") != j.end())
		vsyncEnabled = j["vsyncEnabled"];
	std::cout << "[Config] Config file loaded\n";
}

const std::string& sf::Config::GetName() { return name; }
const glm::uvec2& sf::Config::GetWindowSize() { return windowSize; }
const uint32_t sf::Config::GetMsaaCount() { return msaaCount; }
const glm::vec4& sf::Config::GetClearColor() { return clearColor; }
const bool& sf::Config::GetFullscreen() { return fullscreen; }
const bool& sf::Config::GetImGuiBarEnabled() { return imguiBarEnabled; }
const bool& sf::Config::GetCursorEnabled() { return cursorEnabled; }
const bool& sf::Config::GetVsyncEnabled() { return vsyncEnabled; }
