#pragma once
#include "..//Camera.h"

namespace User
{
	class Game
	{
	private:

	public:
		static void Initialize();
		static void OnUpdate(float deltaTime, float time);
		static void OnKey(int key, int action);
		static void OnMouseScroll(double xoffset, double yoffset);
		static void OnMouseMove(double xpos, double ypos, const glm::vec2& mousePosDelta);
		static void Terminate();
	};
}