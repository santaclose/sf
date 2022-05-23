#pragma once

#include <Renderer/Shader.h>
#include <Renderer/Material.h>
#include <MeshData.h>

namespace sf::Defaults {

	extern Shader shader;
	extern Material material;
	extern MeshData cubeMeshData;

	void Initialize();
}