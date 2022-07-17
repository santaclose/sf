#pragma once

#include <string>

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
		static void ImGuiCall();
	};
}