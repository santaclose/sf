#pragma once

#include <imgui.h>

struct GLFWwindow;

namespace sf::Renderer {

	struct ImGuiBind
	{
		void Initialize(GLFWwindow* window);
		void AfterConfigure(GLFWwindow* window);

		void NewFrame();
		void FrameRender(ImDrawData* draw_data);
		void OnResize();

		void Terminate();
	};
}