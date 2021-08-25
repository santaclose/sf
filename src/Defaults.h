#pragma once

#include <Renderer/Shader.h>
#include <Renderer/Material.h>
#include <Components/Mesh.h>

namespace sf::Defaults {

	extern Shader shader;
	extern Material material;
	extern Mesh cubeMesh;

	void Initialize();
}