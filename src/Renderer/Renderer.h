#pragma once

#include <Scene/Entity.h>
#include <Components/Mesh.h>
#include <Components/VoxelBox.h>
#include <Components/Transform.h>

namespace sf::Renderer {

	extern Entity activeCameraEntity;

	bool Initialize(void* process);
	void OnResize();

	void ClearBuffers();
	void ComputeCameraMatrices();

	void DrawSkybox();
	void DrawMesh(Mesh& mesh, Transform& transform);
	void DrawVoxelBox(VoxelBox& voxelBox, Transform& transform);

	void Terminate();
}