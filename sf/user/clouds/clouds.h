#pragma once
#include "../../Camera.h"
#include "../../Model.h"

namespace User
{
	class Game
	{
	public:
		static void Initialize();
		static void Terminate();
		static void OnUpdate(float deltaTime, float time);
		static void OnKey(int key, int action);
		static void OnMouseScroll(double xoffset, double yoffset);
		static void OnMouseMove(double xpos, double ypos, const glm::vec2& mousePosDelta);
	};
}