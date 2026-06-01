#pragma once

#include <Renderer/RenderTarget.h>

#include <Game.h>

#include <Scene/Scene.h>
#include <Scene/Entity.h>

#include <Components/Base.h>
#include <Components/Mesh.h>
#include <Components/SkinnedMesh.h>
#include <Components/Transform.h>
#include <Components/ScreenCoordinates.h>
#include <Components/Sprite.h>
#include <Components/Text.h>
#include <Components/ParticleSystem.h>
#include <Components/Camera.h>

#include <Components/SphereCollider.h>
#include <Components/CapsuleCollider.h>
#include <Components/BoxCollider.h>

#include <Renderer/GlMaterial.h>
#include <Renderer/RenderTarget.h>
#include <Window.h>
#include <Material.h>

#include <Bitmap.h>

namespace sf::Renderer {

	std::vector<RenderTarget>& GetRenderTargets();

	bool Initialize(const Window& window, const Game::InitData& gameInitData);

	void BindRenderTarget(uint32_t renderTargetId);
	void Clear(bool clearDepth);
	void BindCamera(Camera& camera, Transform& cameraTransform);
	void FrameEnd();

	void SetEnvironment(const std::string& hdrFilePath, DataType hdrDataType = DataType::f16);

	void DrawMesh(Mesh& mesh, Transform& transform);
	void DrawSkinnedMesh(SkinnedMesh& mesh, Transform& transform);
	void DrawParticleSystem(ParticleSystem& particleSystem, Transform& transform, float deltaTime);

	void DrawSprite(Sprite& sprite, ScreenCoordinates& screenCoordinates);
	void DrawText(Text& text, ScreenCoordinates& screenCoordinates);

	void AddLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color);

	void SetDebugDrawEnabled(bool value);
	bool IsDebugDrawEnabled();
	void DebugDrawSkeleton(SkinnedMesh& mesh, Transform& transform);

	void DrawSphereCollider(const SphereCollider& sc, const glm::vec3& color);
	void DrawCapsuleCollider(const CapsuleCollider& cc, const glm::vec3& color);
	void DrawBoxCollider(const BoxCollider& bc, const glm::vec3& color);
	void DrawLines();

	// void DrawScene(Scene* scene, float deltaTime);
	void DrawFramebuffer(const Framebuffer& framebuffer, float deltaTime);

	void Terminate();
}