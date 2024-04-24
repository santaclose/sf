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

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
#include "VulkanShaderBuffer.h"

namespace sf::Renderer
{
	const Window* window;
	VulkanDisplay vkdd;

	float aspectRatio = 16.0f / 9.0f;
	Entity activeCameraEntity;
	bool drawSkybox = false;

	glm::mat4 cameraView;
	glm::mat4 cameraProjection;

	inline VkRect2D rect2D(
		int32_t width,
		int32_t height,
		int32_t offsetX,
		int32_t offsetY)
	{
		VkRect2D rect2D{};
		rect2D.extent.width = width;
		rect2D.extent.height = height;
		rect2D.offset.x = offsetX;
		rect2D.offset.y = offsetY;
		return rect2D;
	}

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec3 tan;
		glm::vec3 bitan;
		glm::vec3 color;
		glm::vec2 uv;
		float ao;

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
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, normal);
			return attributeDescriptions;
		}
	};

	struct MeshGpuData
	{
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
	};
	std::unordered_map<const sf::MeshData*, MeshGpuData> meshGpuData;


	struct PerObjectData
	{
		glm::mat4 modelMatrix;
		int vertexShaderId;
		int fragmentShaderId;
	};
	struct PerFrameData
	{
		glm::mat4 cameraMatrix;
		glm::mat4 screenSpaceMatrix;
		glm::mat4 skyboxMatrix;
		glm::vec3 cameraPosition;
	};
	//VkViewport viewport[2];
	//VkRect2D scissor[2];
	PerObjectData perObjectData;
	VulkanShaderBuffer perFrameUniformBuffer;
	VulkanShaderBuffer userUniformBuffer;
	VulkanShaderBuffer particleMatrixBuffer;
	PerFrameData perFrameUniformData;
	float time = 0.0f;
	std::vector<glm::mat4> particleMatrices;


	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	void DestroyMeshBuffers()
	{
		perFrameUniformBuffer.Destroy();
		userUniformBuffer.Destroy();
		particleMatrixBuffer.Destroy();
		vkDestroyDescriptorPool(VulkanDisplay::Device(), descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(VulkanDisplay::Device(), descriptorSetLayout, nullptr);
		for (auto pair : meshGpuData)
		{
			vkDestroyBuffer(VulkanDisplay::Device(), pair.second.vertexBuffer, nullptr);
			vkFreeMemory(VulkanDisplay::Device(), pair.second.vertexBufferMemory, nullptr);
			vkDestroyBuffer(VulkanDisplay::Device(), pair.second.indexBuffer, nullptr);
			vkFreeMemory(VulkanDisplay::Device(), pair.second.indexBufferMemory, nullptr);
		}
	}

	bool CreatePipeline()
	{
		VkShaderModule vertexShaderModule, fragmentShaderModule;
		assert(VulkanUtils::CreateShaderModule("assets/vulkan/testV.spv", vertexShaderModule));
		assert(VulkanUtils::CreateShaderModule("assets/vulkan/testF.spv", fragmentShaderModule));

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

		//viewport[0].x = 0.0f;
		//viewport[0].y = 0.0f;
		//viewport[0].width = ((float)vkdd.swapchain.extent.width) / 2.0f;
		//viewport[0].height = (float)vkdd.swapchain.extent.height;
		//viewport[0].minDepth = 0.0f;
		//viewport[0].maxDepth = 1.0f;
		//viewport[1].x = ((float)vkdd.swapchain.extent.width) / 2.0f;
		//viewport[1].y = 0.0f;
		//viewport[1].width = ((float)vkdd.swapchain.extent.width) / 2.0f;
		//viewport[1].height = (float)vkdd.swapchain.extent.height;
		//viewport[1].minDepth = 0.0f;
		//viewport[1].maxDepth = 1.0f;

		//scissor[0].offset = { 0, 0 };
		//scissor[0].extent = vkdd.swapchain.extent;
		//scissor[1].offset = { 0, 0 };
		//scissor[1].extent = vkdd.swapchain.extent;

		VkPipelineViewportStateCreateInfo viewport_state = {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports = nullptr;
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

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

		// Descriptor set
		{
			perFrameUniformBuffer.Create(VulkanShaderBufferType::UniformBuffer, true, sizeof(PerFrameData));
			userUniformBuffer.Create(VulkanShaderBufferType::UniformBuffer, true, sizeof(float));
			particleMatrixBuffer.Create(VulkanShaderBufferType::StorageBuffer, false, sizeof(glm::mat4) * 1000);

			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = static_cast<uint32_t>(VulkanDisplay::MaxFramesInFlight());
			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = 1;
			poolInfo.pPoolSizes = &poolSize;
			poolInfo.maxSets = static_cast<uint32_t>(VulkanDisplay::MaxFramesInFlight());
			if (vkCreateDescriptorPool(VulkanDisplay::Device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
			{
				std::cout << "[Renderer] Failed to create descriptor pool\n";
				return false;
			}

			VkDescriptorSetLayoutBinding uboLayoutBinding[] = {
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
			};
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = 3;
			layoutInfo.pBindings = uboLayoutBinding;

			if (vkCreateDescriptorSetLayout(VulkanDisplay::Device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			{
				std::cout << "[Renderer] Failed to create descriptor set layout\n";
				return false;
			}
			std::vector<VkDescriptorSetLayout> layouts(VulkanDisplay::MaxFramesInFlight(), descriptorSetLayout);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanDisplay::MaxFramesInFlight());
			allocInfo.pSetLayouts = layouts.data();
			descriptorSets.resize(VulkanDisplay::MaxFramesInFlight());
			if (vkAllocateDescriptorSets(VulkanDisplay::Device(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
			{
				std::cout << "[Renderer] Failed to allocate descriptor sets\n";
				return false;
			}

			perFrameUniformBuffer.Write(descriptorSets.data(), 0);
			userUniformBuffer.Write(descriptorSets.data(), 1);
			particleMatrixBuffer.Write(descriptorSets.data(), 2);
		}

		VkPushConstantRange push_constant;
		push_constant.offset = 0;
		push_constant.size = sizeof(PerObjectData);
		push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo pipeline_layout_info = {};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 1;
		pipeline_layout_info.pSetLayouts = &descriptorSetLayout;
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant;

		if (vkdd.disp.createPipelineLayout(&pipeline_layout_info, nullptr, &vkdd.pipelineLayout) != VK_SUCCESS)
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
		pipeline_info.layout = vkdd.pipelineLayout;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pColorBlendState = &color_blending;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pDepthStencilState = &depthStencil;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pDynamicState = &dynamic_info;
		pipeline_info.pStages = shader_stages;
		pipeline_info.stageCount = 2;
		pipeline_info.pVertexInputState = &vertex_input_info;
		VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
		pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingCreateInfo.colorAttachmentCount = 1;
		pipelineRenderingCreateInfo.pColorAttachmentFormats = &vkdd.swapchain.image_format;
		pipelineRenderingCreateInfo.depthAttachmentFormat = vkdd.depthFormat;
		pipeline_info.pNext = &pipelineRenderingCreateInfo;

		if (vkdd.disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &vkdd.graphicsPipeline) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create pipline\n";
			return false;
		}

		vkdd.disp.destroyShaderModule(fragmentShaderModule, nullptr);
		vkdd.disp.destroyShaderModule(vertexShaderModule, nullptr);
	}

	void CreateMeshGpuData(const sf::MeshData* mesh)
	{
		meshGpuData[mesh] = MeshGpuData();
		VulkanUtils::CreateVertexBuffer(mesh->vertexCount * mesh->vertexLayout.GetSize(),
			mesh->vertexBuffer, meshGpuData[mesh].vertexBuffer, meshGpuData[mesh].vertexBufferMemory);
		VulkanUtils::CreateIndexBuffer(mesh->indexVector.size() * sizeof(mesh->indexVector[0]),
			mesh->indexVector.data(), meshGpuData[mesh].indexBuffer, meshGpuData[mesh].indexBufferMemory);
	}
}

bool sf::Renderer::Initialize(const Window& windowArg)
{
	window = &windowArg;
	std::cout << "[Renderer] Initializing vulkan renderer" << std::endl;

#ifdef SF_DEBUG
	system("python assets/vulkanCombineShaders.py");
	system("python assets/vulkanCompileShaders.py");
#endif
	vkdd.Initialize(windowArg, CreatePipeline);
	window->AddOnResizeCallback(OnResize);

	particleMatrices.reserve(1000);
	particleMatrices.push_back(glm::mat4(1.0f));
	for (int i = 1; i < 1000; i++, particleMatrices.push_back(glm::translate(glm::mat4(1.0f), glm::vec3((i%20) * 1.0f, 0.0f, (i/20) * 1.0f)) * glm::rotate(glm::mat4(1.0f), i * 1.0f, glm::vec3(0.0f, 1.0f, 0.0f))));
	particleMatrixBuffer.Update(particleMatrices.data(), sizeof(glm::mat4) * 1000);
	return true;
}

void sf::Renderer::Terminate()
{
	vkdd.Terminate(DestroyMeshBuffers);
}

void sf::Renderer::OnResize()
{
	vkDeviceWaitIdle(VulkanDisplay::Device());
	vkdd.RecreateSwapchain();
	aspectRatio = (float)window->GetWidth() / (float)window->GetHeight();
}

uint32_t sf::Renderer::CreateMaterial(const Material& material)
{
	return 0;
}

void sf::Renderer::SetMeshMaterial(const Mesh& mesh, uint32_t materialId, int piece)
{
}

void sf::Renderer::SetEnvironment(const std::string& hdrFilePath, DataType hdrDataType)
{
}

void sf::Renderer::Predraw()
{
	// screen space matrix
	perFrameUniformData.screenSpaceMatrix = glm::ortho(0.0f, (float)sf::Config::GetWindowSize().x, (float)sf::Config::GetWindowSize().y, 0.0f);

	// camera matrices
	assert(activeCameraEntity);

	const Transform& transformComponent = activeCameraEntity.GetComponent<Transform>();
	const Camera& cameraComponent = activeCameraEntity.GetComponent<Camera>();
	if (cameraComponent.perspective)
	{
		if (aspectRatio == aspectRatio) // only if aspectRatio is not nan, it is nan when fullscreen and not visible
			cameraProjection = glm::perspective(
				cameraComponent.fieldOfView,
				aspectRatio,
				cameraComponent.nearClippingPlane,
				cameraComponent.farClippingPlane);
	}
	else
	{
		if (aspectRatio >= 1.0)
			cameraProjection = glm::ortho(
				-aspectRatio / 2.0f * cameraComponent.orthographicScale,
				aspectRatio / 2.0f * cameraComponent.orthographicScale,
				-0.5f * cameraComponent.orthographicScale,
				0.5f * cameraComponent.orthographicScale,
				cameraComponent.nearClippingPlane,
				cameraComponent.farClippingPlane);
		else
			cameraProjection = glm::ortho(
				-0.5f * cameraComponent.orthographicScale,
				0.5f * cameraComponent.orthographicScale,
				-1.0f / aspectRatio / 2.0f * cameraComponent.orthographicScale,
				1.0f / aspectRatio / 2.0f * cameraComponent.orthographicScale,
				cameraComponent.nearClippingPlane,
				cameraComponent.farClippingPlane);
	}

	cameraView = (glm::mat4)glm::conjugate(transformComponent.rotation);
	cameraView = glm::translate(cameraView, -transformComponent.position);

	perFrameUniformData.cameraMatrix = cameraProjection * cameraView;
	Transform& cameraTransform = activeCameraEntity.GetComponent<Transform>();
	perFrameUniformData.cameraPosition = cameraTransform.position;
	perFrameUniformData.skyboxMatrix = cameraProjection * glm::mat4(glm::mat3(cameraView));
	perFrameUniformBuffer.Update(perFrameUniformData);
	time += 1.0f / 60.0f;
	userUniformBuffer.Update(time);

	vkdd.Predraw(Config::GetClearColor());

	vkCmdBindDescriptorSets(vkdd.CommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vkdd.pipelineLayout, 0, 1, &descriptorSets[vkdd.currentFrameInFlight], 0, nullptr);
}

void sf::Renderer::Postdraw()
{
	vkdd.Postdraw();
}

void sf::Renderer::DrawSkybox()
{
}

void sf::Renderer::DrawMesh(Mesh& mesh, Transform& transform)
{
	if (mesh.meshData->vertexCount == 0)
		return;

	assert(activeCameraEntity);

	if (meshGpuData.find(mesh.meshData) == meshGpuData.end()) // create mesh data if not there
		CreateMeshGpuData(mesh.meshData);

	perObjectData.modelMatrix = transform.ComputeMatrix();
	vkCmdPushConstants(vkdd.CommandBuffer(), vkdd.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PerObjectData), &perObjectData);

	VkBuffer vertexBuffers[] = { meshGpuData[mesh.meshData].vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(vkdd.CommandBuffer(), 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(vkdd.CommandBuffer(), meshGpuData[mesh.meshData].indexBuffer, 0, VK_INDEX_TYPE_UINT32);


	VkViewport viewports[2] = {
		{ 0,										 (float)vkdd.swapchain.extent.height, (float)vkdd.swapchain.extent.width / 2.0f, -(float)vkdd.swapchain.extent.height, 0.0, 1.0f },
		{ (float)vkdd.swapchain.extent.width / 2.0f, (float)vkdd.swapchain.extent.height, (float)vkdd.swapchain.extent.width / 2.0f, -(float)vkdd.swapchain.extent.height, 0.0, 1.0f },
	};
	vkCmdSetViewport(vkdd.CommandBuffer(), 1, 1, viewports);

	VkRect2D scissorRects[2] = {
		rect2D(vkdd.swapchain.extent.width / 2, vkdd.swapchain.extent.height, 0, 0),
		rect2D(vkdd.swapchain.extent.width / 2, vkdd.swapchain.extent.height, vkdd.swapchain.extent.width / 2, 0),
	};
	vkCmdSetScissor(vkdd.CommandBuffer(), 1, 1, scissorRects);
	//vkCmdSetViewport(vkdd.CommandBuffer(), 0, 2, viewport);
	//vkCmdSetScissor(vkdd.CommandBuffer(), 0, 2, scissor);
	vkCmdDrawIndexed(vkdd.CommandBuffer(), static_cast<uint32_t>(mesh.meshData->indexVector.size()), 1000, 0, 0, 0);
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

#endif