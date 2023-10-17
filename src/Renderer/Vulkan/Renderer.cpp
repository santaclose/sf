#ifdef SF_USE_VULKAN

#include "../Renderer.h"

#include <vulkan/vulkan.h>

#include <limits>
#include <assert.h>
#include <cstdint>
#include <iostream>
#include <algorithm> 
#include <unordered_map>
#include <unordered_set>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Config.h>
#include <Bitmap.h>

#include <Scene/Entity.h>
#include <Components/Camera.h>

#include <Material.h>
#include <Defaults.h>
#include <FileUtils.h>

#include "VulkanDisplay.h"
#include "VulkanUtils.h"

namespace sf::Renderer
{
	const Window* window;
	VulkanDisplay vkDisplayData;

	float aspectRatio = 16.0f / 9.0f;
	Entity activeCameraEntity;
	bool drawSkybox = false;

	glm::mat4 cameraView;
	glm::mat4 cameraProjection;

	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}
		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);
			return attributeDescriptions;
		}
	};

	const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};
	const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
	};
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	void DestroyMeshBuffers()
	{
		vkDestroyBuffer(vkDisplayData.disp.device, indexBuffer, nullptr);
		vkFreeMemory(vkDisplayData.disp.device, indexBufferMemory, nullptr);
		vkDestroyBuffer(vkDisplayData.disp.device, vertexBuffer, nullptr);
		vkFreeMemory(vkDisplayData.disp.device, vertexBufferMemory, nullptr);
	}

	bool CreatePipeline(VulkanDisplay& vkdd)
	{
		VkShaderModule vertexShaderModule, fragmentShaderModule;
		assert(VulkanUtils::CreateShaderModule(vkDisplayData, "assets/shaders/vulkan/testV.spv", vertexShaderModule));
		assert(VulkanUtils::CreateShaderModule(vkDisplayData, "assets/shaders/vulkan/testF.spv", fragmentShaderModule));

		VkPipelineShaderStageCreateInfo vert_stage_info = {};
		vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_stage_info.module = vertexShaderModule;
		vert_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_stage_info = {};
		frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_stage_info.module = fragmentShaderModule;
		frag_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] = { vert_stage_info, frag_stage_info };

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount = 1;
		vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertex_input_info.pVertexBindingDescriptions = &bindingDescription;
		vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)vkdd.swapchain.extent.width;
		viewport.height = (float)vkdd.swapchain.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = vkdd.swapchain.extent;

		VkPipelineViewportStateCreateInfo viewport_state = {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports = &viewport;
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo color_blending = {};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.logicOp = VK_LOGIC_OP_COPY;
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &colorBlendAttachment;
		color_blending.blendConstants[0] = 0.0f;
		color_blending.blendConstants[1] = 0.0f;
		color_blending.blendConstants[2] = 0.0f;
		color_blending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipeline_layout_info = {};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 0;
		pipeline_layout_info.pushConstantRangeCount = 0;

		if (vkdd.disp.createPipelineLayout(&pipeline_layout_info, nullptr, &vkdd.pipeline_layout) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create pipeline layout\n";
			return false;
		}

		std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamic_info = {};
		dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_info.pDynamicStates = dynamic_states.data();

		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.layout = vkdd.pipeline_layout;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pColorBlendState = &color_blending;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pDynamicState = &dynamic_info;
		pipeline_info.pStages = shader_stages;
		pipeline_info.stageCount = 2;
		pipeline_info.pVertexInputState = &vertex_input_info;
		VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
		pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingCreateInfo.colorAttachmentCount = 1;
		pipelineRenderingCreateInfo.pColorAttachmentFormats = &vkDisplayData.swapchain.image_format;
		pipeline_info.pNext = &pipelineRenderingCreateInfo;

		if (vkdd.disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &vkdd.graphics_pipeline) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create pipline\n";
			return false;
		}

		vkdd.disp.destroyShaderModule(fragmentShaderModule, nullptr);
		vkdd.disp.destroyShaderModule(vertexShaderModule, nullptr);
	}
}

bool sf::Renderer::Initialize(const Window& windowArg)
{
	window = &windowArg;
	std::cout << "[Renderer] Initializing vulkan renderer" << std::endl;

#ifdef SF_DEBUG
	system("python assets/compileShaders.py");
#endif
	vkDisplayData.Initialize(windowArg, CreatePipeline);
	window->AddOnResizeCallback(OnResize);

	VulkanUtils::CreateVertexBuffer(vkDisplayData, vertices.size() * sizeof(vertices[0]), vertices.data(), vertexBuffer, vertexBufferMemory);
	VulkanUtils::CreateIndexBuffer(vkDisplayData, indices.size() * sizeof(indices[0]), indices.data(), indexBuffer, indexBufferMemory);

	return true;
}

void sf::Renderer::OnResize()
{
	vkDeviceWaitIdle(vkDisplayData.device.device);
	vkDisplayData.RecreateSwapchain();
	aspectRatio = (float)window->GetWidth() / (float)window->GetHeight();
}

void sf::Renderer::Predraw()
{
	vkDisplayData.PreDraw();

	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(vkDisplayData.GetCurrentCommandBuffer(), 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(vkDisplayData.GetCurrentCommandBuffer(), indexBuffer, 0, VK_INDEX_TYPE_UINT16);
	vkCmdDrawIndexed(vkDisplayData.GetCurrentCommandBuffer(), static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	vkDisplayData.PostDraw();
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
	vkDisplayData.Terminate(DestroyMeshBuffers);
}

#endif