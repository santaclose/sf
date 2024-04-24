#pragma once

#include <Window.h>

namespace sf::ImGuiController
{
	void Initialize(Window& window);
	void Terminate();
	bool HasControl();
	void Tick(float deltaTime);
}
