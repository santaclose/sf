#ifdef SF_USE_VULKAN

#include "../Renderer.h"

#include <assert.h>
#include <iostream>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Config.h>
#include <Bitmap.h>

#include <Scene/Entity.h>
#include <Components/Camera.h>

#include <Material.h>
#include <Defaults.h>

#include "VulkanState.h"

namespace sf::Renderer
{
	float aspectRatio = 1.7777777777;
	Entity activeCameraEntity;
	bool drawSkybox = false;

	glm::mat4 cameraView;
	glm::mat4 cameraProjection;
}


bool sf::Renderer::Initialize(void* process)
{
	std::cout << "[Renderer] Initializing vulkan renderer" << std::endl;
	return true;
}

void sf::Renderer::OnResize()
{

}

void sf::Renderer::Predraw()
{
}

void sf::Renderer::SetMeshMaterial(const Mesh& mesh, uint32_t materialId, int piece)
{
}

uint32_t sf::Renderer::CreateMaterial(const Material& material)
{
	return 0;
}

void sf::Renderer::SetEnvironment(const std::string& hdrFilePath, DataType hdrDataType)
{
}

void sf::Renderer::DrawSkybox()
{
}

void sf::Renderer::DrawMesh(Mesh& mesh, Transform& transform)
{
}

void sf::Renderer::DrawSkinnedMesh(SkinnedMesh& mesh, Transform& transform)
{
}

void sf::Renderer::DrawVoxelBox(VoxelBox& voxelBox, Transform& transform)
{
}

void sf::Renderer::DrawSkeleton(Skeleton& skeleton, Transform& transform)
{
}

void sf::Renderer::DrawSprite(Sprite& sprite, ScreenCoordinates& screenCoordinates)
{
}

void sf::Renderer::Terminate()
{
}

#endif