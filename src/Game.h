#pragma once
#include <glm/glm.hpp>

namespace User
{
	class Game
	{
	private:

	public:
		static void Initialize(int argc, char** argv);
		static void Terminate();
		static void OnUpdate(float deltaTime, float time);
	};
}