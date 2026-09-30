#pragma once

#include <Renderer/RenderTarget.h>
#include <Renderer/DebugDraw.h>

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

	struct Stats
	{
		uint32_t meshTriangles = 0u;
		uint32_t meshQuads = 0u;
		uint32_t skinnedMeshTriangles = 0u;
	};

	std::vector<RenderTarget>& GetRenderTargets();

	bool Initialize(const Window& window, const Game::InitData& gameInitData);

	void BindRenderTarget(uint32_t renderTargetId);
	void Clear(bool clearDepth);
	void BindCamera(const Camera& camera, const Transform& cameraTransform);
	Stats FrameEnd();

	void SetEnvironment(const std::string& hdrFilePath, DataType hdrDataType = DataType::f16);

	void DrawMeshShading(const Mesh& mesh, const Transform& transform);
	void DrawMesh(const Mesh& mesh, const Transform& transform);
	void DrawSkinnedMesh(const SkinnedMesh& mesh, const Transform& transform);
	void DrawParticleSystem(const ParticleSystem& particleSystem, const Transform& transform, float deltaTime);

	void DrawSprite(const Sprite& sprite, const ScreenCoordinates& screenCoordinates);
	void DrawText(const Text& text, const ScreenCoordinates& screenCoordinates);

	Debug::Config& AccessDebugConfig();

	SphereCollider GetFrustumCullingMeshSphere(const Mesh& mesh, const Transform& transform);
	SphereCollider GetFrustumCullingSkinnedMeshSphere(const SkinnedMesh& mesh, const Transform& transform);

	// void DrawScene(Scene* scene, float deltaTime);
	void DrawFramebuffer(const Framebuffer& framebuffer, float deltaTime);

	void InvalidateMeshData(const MeshData& meshData);
	void InvalidateSkeletonData(const SkeletonData& skeletonData);
	void InvalidateLODGroup(const LODGroup& lodGroup);
	void InvalidateParticleSystem(const ParticleSystem& particleSystem);
	void InvalidateMaterial(const Material& material);

	void Terminate();
}