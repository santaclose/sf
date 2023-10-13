#pragma once

#include <Scene/Entity.h>
#include <Components/Mesh.h>
#include <Components/SkinnedMesh.h>
#include <Components/VoxelBox.h>
#include <Components/Transform.h>
#include <Components/Skeleton.h>
#include <Components/ScreenCoordinates.h>
#include <Components/Sprite.h>

#include <Window.h>
#include <Material.h>

#include <Bitmap.h>

namespace sf::Renderer {

	extern Entity activeCameraEntity;
	extern bool drawSkybox;

	bool Initialize(const Window& window);
	void OnResize();

	void Predraw();

	uint32_t CreateMaterial(const Material& material);

	void SetMeshMaterial(const Mesh& mesh, uint32_t materialId, int piece = -1);

	void SetEnvironment(const std::string& hdrFilePath, DataType hdrDataType = DataType::f16);

	void DrawSkybox();
	void DrawMesh(Mesh& mesh, Transform& transform);
	void DrawSkinnedMesh(SkinnedMesh& mesh, Transform& transform);
	void DrawVoxelBox(VoxelBox& voxelBox, Transform& transform);
	void DrawSkeleton(Skeleton& skeleton, Transform& transform);
	void DrawSprite(Sprite& sprite, ScreenCoordinates& screenCoordinates);

	void Terminate();
}