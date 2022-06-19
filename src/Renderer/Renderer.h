#pragma once

#include <Scene/Entity.h>
#include <Components/Mesh.h>
#include <Components/VoxelBox.h>
#include <Components/Transform.h>

#include <Renderer/GlMaterial.h>
#include <Material.h>

#include <Bitmap.h>

namespace sf::Renderer {

	extern Entity activeCameraEntity;
	extern bool drawSkybox;

	bool Initialize(void* process);
	void OnResize();

	void ClearBuffers();
	void ComputeCameraMatrices();

	uint32_t CreateMaterial(const Material& material);

	void SetMeshMaterial(Mesh mesh, GlMaterial* material, int piece = -1);
	void SetMeshMaterial(Mesh mesh, uint32_t materialId, int piece = -1);

	void SetEnvironment(const std::string& hdrFilePath);

	void DrawSkybox();
	void DrawMesh(Mesh& mesh, Transform& transform);
	void DrawVoxelBox(VoxelBox& voxelBox, Transform& transform);

	void Terminate();
}