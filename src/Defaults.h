#pragma once

#include <MeshData.h>
#include <DataLayout.h>

namespace sf::Defaults {

	extern MeshData cubeMeshData;
	extern MeshData planeMeshData;
	extern MeshData monkeMeshData;
	extern DataLayout defaultVertexLayout;
	extern DataLayout defaultSkinningVertexLayout;

	void Initialize();
}