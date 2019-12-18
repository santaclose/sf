#pragma once
#include "..//Camera.h"

namespace User
{
	class Game
	{
	private:
		static float shipSpeed;
		static glm::fquat targetShipRotation;
		static Camera theCamera;
		static Camera lookBackCamera;

	public:
		static void Initialize();
		static void OnUpdate(float deltaTime, float time);
		static void OnKey(int key, int action);
		static void OnScroll(double xoffset, double yoffset);
		static void OnMouseMove(double xpos, double ypos, const glm::vec2& mousePosDelta);
	};
}