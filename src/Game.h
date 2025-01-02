#pragma once

#include <string>
#include <Scene/Entity.h>

namespace sf
{
	class Game
	{
	private:

	public:
		static std::string ConfigFilePath;
		static void Initialize(int argc, char** argv);
		static void Terminate();
		static void OnUpdate(float deltaTime, float time);
		static void OnCollision(Entity entity);
		static void ImGuiCall();
	};
}