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
	struct Pipeline
	{
		DataLayout vertexAttributeDataLayout;
		VkPipelineLayout layout;
		VkPipeline pipeline;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet[MAX_FRAMES_IN_FLIGHT];
		VkDescriptorPool descriptorPool;
		std::string vertexShaderPath;
		std::string fragmentShaderPath;
	};

	// common across pipelines
	VulkanShaderBuffer commonUniformBuffer[MAX_FRAMES_IN_FLIGHT];

	const Window* window;
	VulkanDisplay vkdd;

	std::vector<Pipeline> pipelines;
	std::unordered_map<int, std::unordered_map<int, int>> meshToPipeline;
	int currentPipeline = -1;

	float aspectRatio = 16.0f / 9.0f;
	Entity activeCameraEntity;
	bool drawSkybox = false;

	glm::mat4 cameraView;
	glm::mat4 cameraProjection;


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
	};
	struct PerFrameData
	{
		glm::mat4 cameraMatrix;
		glm::mat4 screenSpaceMatrix;
		glm::mat4 skyboxMatrix;
		glm::vec3 cameraPosition;
	};

	PerObjectData perObjectData;
	PerFrameData perFrameUniformData;

	void DestroyPipelines()
	{
		for (int i = 0; i < pipelines.size(); i++)
		{
			vkdd.disp.destroyPipeline(pipelines[i].pipeline, nullptr);
			vkdd.disp.destroyPipelineLayout(pipelines[i].layout, nullptr);
		}
	}
	void DestroyMeshBuffers()
	{
		for (int i = 0; i < ARRAY_LEN(commonUniformBuffer); i++)
			commonUniformBuffer[i].Destroy();

		for (Pipeline& p : pipelines)
		{
			vkDestroyDescriptorPool(VulkanDisplay::Device(), p.descriptorPool, nullptr);
			vkDestroyDescriptorSetLayout(VulkanDisplay::Device(), p.descriptorSetLayout, nullptr);
		}
		for (auto pair : meshGpuData)
		{
			vkDestroyBuffer(VulkanDisplay::Device(), pair.second.vertexBuffer, nullptr);
			vkFreeMemory(VulkanDisplay::Device(), pair.second.vertexBufferMemory, nullptr);
			vkDestroyBuffer(VulkanDisplay::Device(), pair.second.indexBuffer, nullptr);
			vkFreeMemory(VulkanDisplay::Device(), pair.second.indexBufferMemory, nullptr);
		}
	}

	bool CreatePipeline(Pipeline& newPipeline)
	{
		std::cout << "[Renderer] Creating pipeline for shaders: ";
		std::cout << newPipeline.vertexShaderPath << ", " << newPipeline.fragmentShaderPath << std::endl;

		VkShaderModule vertexShaderModule, fragmentShaderModule;
		assert(VulkanUtils::CreateShaderModule(newPipeline.vertexShaderPath + ".spv", vertexShaderModule));
		assert(VulkanUtils::CreateShaderModule(newPipeline.fragmentShaderPath + ".spv", fragmentShaderModule));

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

		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		VulkanUtils::VertexAttributeDescriptionsFromDataLayout(newPipeline.vertexAttributeDataLayout, bindingDescription, attributeDescriptions);

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
		multisampling.rasterizationSamples = (VkSampleCountFlagBits) vkdd.sampleCount;

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
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;
			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = 1;
			poolInfo.pPoolSizes = &poolSize;
			poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
			if (vkCreateDescriptorPool(VulkanDisplay::Device(), &poolInfo, nullptr, &newPipeline.descriptorPool) != VK_SUCCESS)
			{
				std::cout << "[Renderer] Failed to create descriptor pool\n";
				return false;
			}

			std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
			VulkanUtils::DescriptorSetBindingsFromShader(newPipeline.vertexShaderPath.c_str(), newPipeline.fragmentShaderPath.c_str(), descriptorSetLayoutBindings);

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = descriptorSetLayoutBindings.size();
			layoutInfo.pBindings = descriptorSetLayoutBindings.data();

			if (vkCreateDescriptorSetLayout(VulkanDisplay::Device(), &layoutInfo, nullptr, &newPipeline.descriptorSetLayout) != VK_SUCCESS)
			{
				std::cout << "[Renderer] Failed to create descriptor set layout\n";
				return false;
			}
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = newPipeline.descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &newPipeline.descriptorSetLayout;

			for (int i = 0; i < ARRAY_LEN(newPipeline.descriptorSet); i++)
			{
				if (vkAllocateDescriptorSets(VulkanDisplay::Device(), &allocInfo, &(newPipeline.descriptorSet[i])) != VK_SUCCESS)
				{
					std::cout << "[Renderer] Failed to allocate descriptor sets\n";
					return false;
				}

				// set desriptor set to point to our common buffer
				commonUniformBuffer[i].Write(newPipeline.descriptorSet[i], 0);
			}
		}

		VkPushConstantRange push_constant;
		push_constant.offset = 0;
		push_constant.size = sizeof(PerObjectData);
		push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo pipeline_layout_info = {};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 1;
		pipeline_layout_info.pSetLayouts = &newPipeline.descriptorSetLayout;
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant;

		if (vkdd.disp.createPipelineLayout(&pipeline_layout_info, nullptr, &newPipeline.layout) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create pipeline layout\n";
			return false;
		}

		// Need dynamic viewport and scissor for window resize
		VkPipelineViewportStateCreateInfo viewport_state = {};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.scissorCount = 1;
		std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamic_info = {};
		dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_info.dynamicStateCount = dynamic_states.size();
		dynamic_info.pDynamicStates = dynamic_states.data();

		VkGraphicsPipelineCreateInfo pipeline_info = {};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.layout = newPipeline.layout;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pColorBlendState = &color_blending;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pDepthStencilState = &depthStencil;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pDynamicState = &dynamic_info;
		pipeline_info.pStages = shader_stages;
		pipeline_info.stageCount = ARRAY_LEN(shader_stages);
		pipeline_info.pVertexInputState = &vertex_input_info;
		VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
		pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingCreateInfo.colorAttachmentCount = 1;
		pipelineRenderingCreateInfo.pColorAttachmentFormats = &vkdd.swapchain.image_format;
		pipelineRenderingCreateInfo.depthAttachmentFormat = vkdd.depthFormat;
		pipeline_info.pNext = &pipelineRenderingCreateInfo;

		if (vkdd.disp.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &newPipeline.pipeline) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create pipline\n";
			return false;
		}

		vkdd.disp.destroyShaderModule(fragmentShaderModule, nullptr);
		vkdd.disp.destroyShaderModule(vertexShaderModule, nullptr);
	}

	void CreateMeshGpuData(const sf::MeshData* mesh)
	{
		size_t vertexBufferSize = mesh->vertexCount * mesh->vertexLayout.GetSize();
		meshGpuData[mesh] = MeshGpuData();

		VulkanUtils::CreateVertexBuffer(vertexBufferSize,
			mesh->vertexBuffer, meshGpuData[mesh].vertexBuffer, meshGpuData[mesh].vertexBufferMemory);
		VulkanUtils::CreateIndexBuffer(mesh->indexVector.size() * sizeof(mesh->indexVector[0]),
			mesh->indexVector.data(), meshGpuData[mesh].indexBuffer, meshGpuData[mesh].indexBufferMemory);
	}
}

bool sf::Renderer::Initialize(const Window& windowArg)
{
	window = &windowArg;
	std::cout << "[Renderer] Initializing vulkan renderer" << std::endl;

	vkdd.Initialize(windowArg);

	for (int i = 0; i < ARRAY_LEN(commonUniformBuffer); i++)
		commonUniformBuffer[i].Create(VulkanShaderBufferType::UniformBuffer, sizeof(PerFrameData));

	for (int i = 0; i < pipelines.size(); i++)
		CreatePipeline(pipelines[i]);

	window->AddOnResizeCallback(OnResize);
	return true;
}

void sf::Renderer::Terminate()
{
	vkdd.Terminate(DestroyMeshBuffers, DestroyPipelines);
}

void sf::Renderer::OnResize()
{
	vkDeviceWaitIdle(VulkanDisplay::Device());
	vkdd.RecreateSwapchain();
	aspectRatio = (float)window->GetWidth() / (float)window->GetHeight();
}

uint32_t sf::Renderer::CreateMaterial(const Material& material)
{
	system(std::string("python assets/vulkanCompileShaders.py " + material.vertexShaderFilePath + ".glsl " + material.fragmentShaderFilePath + ".glsl").c_str());

	pipelines.emplace_back();
	pipelines.back().vertexShaderPath = material.vertexShaderFilePath;
	pipelines.back().fragmentShaderPath = material.fragmentShaderFilePath;
	pipelines.back().vertexAttributeDataLayout = DataLayout(pipelines.back().vertexShaderPath.c_str());
	return pipelines.size() - 1;
}

void sf::Renderer::SetMeshMaterial(const Mesh& mesh, uint32_t materialId, int piece)
{
	assert(mesh.meshData->vertexLayout == pipelines[materialId].vertexAttributeDataLayout);
	if (piece < 0) // set for all pieces by default
	{
		for (int i = 0; i < mesh.meshData->pieces.size(); i++)
			meshToPipeline[mesh.id][i] = materialId;
	}
	else
	{
		assert(piece < mesh.meshData->pieces.size());
		meshToPipeline[mesh.id][piece] = materialId;
	}
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

	commonUniformBuffer[VulkanDisplay::CurrentFrameInFlight()].Update(perFrameUniformData);

	vkdd.Predraw(Config::GetClearColor());
	currentPipeline = -1;
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

	vkCmdPushConstants(vkdd.CommandBuffer(), pipelines.back().layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PerObjectData), &perObjectData);

	VkBuffer vertexBuffers[] = { meshGpuData[mesh.meshData].vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(vkdd.CommandBuffer(), 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(vkdd.CommandBuffer(), meshGpuData[mesh.meshData].indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	for (uint32_t i = 0; i < mesh.meshData->pieces.size(); i++)
	{
		int pipelineToUse;
		if (meshToPipeline.find(mesh.id) != meshToPipeline.end() && meshToPipeline[mesh.id].find(i) != meshToPipeline[mesh.id].end())
			pipelineToUse = meshToPipeline[mesh.id][i];
		else
			pipelineToUse = pipelines.size() - 1; // last one as default for now

		if (pipelineToUse != currentPipeline)
		{
			vkCmdBindPipeline(vkdd.CommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[pipelineToUse].pipeline);
			vkCmdBindDescriptorSets(vkdd.CommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[pipelineToUse].layout, 0, 1,
				&pipelines[pipelineToUse].descriptorSet[VulkanDisplay::CurrentFrameInFlight()], 0, nullptr);
			currentPipeline = pipelineToUse;
		}

		uint32_t drawEnd, drawStart;
		drawStart = mesh.meshData->pieces[i];
		drawEnd = mesh.meshData->pieces.size() > i + 1 ? mesh.meshData->pieces[i + 1] : mesh.meshData->indexVector.size();

		vkCmdDrawIndexed(vkdd.CommandBuffer(), drawEnd - drawStart, 1, drawStart, 0, 0);
	}
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