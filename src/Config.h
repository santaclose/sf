#pragma once

#include <string>

namespace sf::Config
{
	extern std::string name;
	extern uint32_t windowWidth;
	extern uint32_t windowHeight;
	extern int msaaCount;
	extern bool fullscreen;
	extern float clearColor[3];
};