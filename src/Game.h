#pragma once

#include <glm/glm.hpp>

namespace sf
{
	class Game
	{
	private:

	public:
		static void Initialize(int argc, char** argv);
		static void Terminate();
		static void OnUpdate(float deltaTime, float time);
		static void ImGuiCall();
	};
}