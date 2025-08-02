#pragma once

#include <Scene/Entity.h>
#include <Components/Mesh.h>
#include <Components/SkinnedMesh.h>
#include <Components/VoxelVolume.h>
#include <Components/Transform.h>
#include <Components/ScreenCoordinates.h>
#include <Components/Sprite.h>
#include <Components/Text.h>
#include <Components/ParticleSystem.h>

#include <Components/SphereCollider.h>
#include <Components/CapsuleCollider.h>
#include <Components/BoxCollider.h>

#include <Renderer/GlMaterial.h>
#include <Window.h>
#include <Material.h>

#include <Bitmap.h>

namespace sf::Renderer {

	bool Initialize(const Window& window, const glm::vec3& clearColorArg);
	void OnResize();

	void Predraw();
	void Postdraw();

	void SetClearColor(const glm::vec3& clearColorArg);
	const glm::vec3& GetClearColor();

	uint32_t CreateMaterial(const Material& material, const BufferLayout& vertexBufferLayout);

	void SetActiveCameraEntity(Entity cameraEntity);
	Entity GetActiveCameraEntity();

	void SetEnvironment(const std::string& hdrFilePath, DataType hdrDataType = DataType::f16);

	void DrawSkybox();
	void DrawMesh(Mesh& mesh, Transform& transform);
	void DrawSkinnedMesh(SkinnedMesh& mesh, Transform& transform);
	void DrawParticleSystem(ParticleSystem& particleSystem, Transform& transform, float deltaTime);

	void DrawSprite(Sprite& sprite, ScreenCoordinates& screenCoordinates);
	void DrawText(Text& text, ScreenCoordinates& screenCoordinates);

	void AddLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);


	void SetDebugDrawEnabled(bool value);
	bool IsDebugDrawEnabled();
	void DebugDrawSkeleton(SkinnedMesh& mesh, Transform& transform);

	void DrawVoxelVolume(VoxelVolume& voxelVolume, Transform& transform);
	void DrawSphereCollider(const SphereCollider& sc, const glm::vec3& color);
	void DrawCapsuleCollider(const CapsuleCollider& cc, const glm::vec3& color);
	void DrawBoxCollider(const BoxCollider& bc, const glm::vec3& color);
	void DrawLines();

	void Terminate();
}