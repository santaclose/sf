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

#include "VulkanSwapchain.h"

namespace sf::Renderer
{
	const Window* window;

	float aspectRatio = 1.7777777777;
	Entity activeCameraEntity;
	bool drawSkybox = false;

	glm::mat4 cameraView;
	glm::mat4 cameraProjection;

	VulkanSwapchain swapChain;

	VkInstance instance;
	VkPhysicalDevice physicalDeviceToUse;
	VkPhysicalDeviceProperties physicalDeviceToUseProperties;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	const int MAX_FRAMES_IN_FLIGHT = 2;
	uint32_t currentFrame = 0;

	VkDebugUtilsMessengerEXT debugMessenger;

	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> deviceExtensions = {	VK_KHR_SWAPCHAIN_EXTENSION_NAME	};


#ifdef SF_DEBUG
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cout << "[DebugCallback] Validation layer callback: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}
#endif
	void GetRequiredExtensionsForInstance(std::vector<const char*>& out)
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = window->GetVulkanExtensions(glfwExtensionCount);
		out = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef SF_DEBUG
		out.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
	}

	bool EnableInstanceExtensions(VkInstanceCreateInfo& createInfo, const std::vector<const char*>& extensionsToEnable)
	{
		uint32_t availableExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());
		std::unordered_set<std::string> availableExtensionSet;
		for (const auto& extensionProperties : availableExtensions)
			availableExtensionSet.insert(extensionProperties.extensionName);
		std::vector<std::string> extensionsToEnableNotAvailable;
		for (const char* wantedExtension : extensionsToEnable)
		{
			if (availableExtensionSet.find(wantedExtension) == availableExtensionSet.end())
				extensionsToEnableNotAvailable.push_back(wantedExtension);
		}
		if (extensionsToEnableNotAvailable.size() == 0)
			std::cout << "[Renderer] All wanted instance extensions are available\n";
		else
		{
			std::cout << "[Renderer] Some wanted instance extensions are not available:\n";
			for (const std::string& extension : extensionsToEnableNotAvailable)
				std::cout << '\t' << extension << '\n';
			return false;
		}
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsToEnable.size());
		createInfo.ppEnabledExtensionNames = extensionsToEnable.data();
		return true;
	}

	bool EnableInstanceLayers(VkInstanceCreateInfo& createInfo, const std::vector<const char*>& layersToEnable)
	{
		if (layersToEnable.size() == 0)
		{
			createInfo.enabledLayerCount = 0;
			return true;
		}

		uint32_t availableLayerCount;
		vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(availableLayerCount);
		vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());
		std::unordered_set<std::string> availableLayerSet;
		for (const auto& layerProperties : availableLayers)
			availableLayerSet.insert(layerProperties.layerName);
		std::vector<std::string> layersToEnableNotAvailable;
		for (const char* wantedLayer : layersToEnable)
		{
			if (availableLayerSet.find(wantedLayer) == availableLayerSet.end())
				layersToEnableNotAvailable.push_back(wantedLayer);
		}
		if (layersToEnableNotAvailable.size() == 0)
			std::cout << "[Renderer] All wanted instance layers are available\n";
		else
		{
			std::cout << "[Renderer] Some wanted instance layers are not available\n";
			for (const std::string& layer : layersToEnableNotAvailable)
				std::cout << '\t' << layer << '\n';
			return false;
		}
		createInfo.enabledLayerCount = static_cast<uint32_t>(layersToEnable.size());
		createInfo.ppEnabledLayerNames = layersToEnable.data();
		return true;
	}

	bool EnableDeviceExtensions(VkDeviceCreateInfo& createInfo, VkPhysicalDevice device, const std::vector<const char*>& extensionsToEnable)
	{
		uint32_t availableExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data());
		std::unordered_set<std::string> availableExtensionSet;
		for (const auto& extensionProperties : availableExtensions)
			availableExtensionSet.insert(extensionProperties.extensionName);
		std::vector<std::string> extensionsToEnableNotAvailable;
		for (const char* wantedExtension : extensionsToEnable)
		{
			if (availableExtensionSet.find(wantedExtension) == availableExtensionSet.end())
				extensionsToEnableNotAvailable.push_back(wantedExtension);
		}
		if (extensionsToEnableNotAvailable.size() == 0)
			std::cout << "[Renderer] All wanted device extensions are available\n";
		else
		{
			std::cout << "[Renderer] Some wanted device extensions are not available:\n";
			for (const std::string& extension : extensionsToEnableNotAvailable)
				std::cout << '\t' << extension << '\n';
			return false;
		}
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsToEnable.size());
		createInfo.ppEnabledExtensionNames = extensionsToEnable.data();
		return true;
	}

	bool EnableDeviceLayers(VkDeviceCreateInfo& createInfo, VkPhysicalDevice device, const std::vector<const char*>& layersToEnable)
	{
		if (layersToEnable.size() == 0)
		{
			createInfo.enabledLayerCount = 0;
			return true;
		}

		uint32_t availableLayerCount;
		vkEnumerateDeviceLayerProperties(device, &availableLayerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(availableLayerCount);
		vkEnumerateDeviceLayerProperties(device , &availableLayerCount, availableLayers.data());
		std::unordered_set<std::string> availableLayerSet;
		for (const auto& layerProperties : availableLayers)
			availableLayerSet.insert(layerProperties.layerName);
		std::vector<std::string> layersToEnableNotAvailable;
		for (const char* wantedLayer : layersToEnable)
		{
			if (availableLayerSet.find(wantedLayer) == availableLayerSet.end())
				layersToEnableNotAvailable.push_back(wantedLayer);
		}
		if (layersToEnableNotAvailable.size() == 0)
			std::cout << "[Renderer] All wanted device layers are available\n";
		else
		{
			std::cout << "[Renderer] Some wanted device layers are not available\n";
			for (const std::string& layer : layersToEnableNotAvailable)
				std::cout << '\t' << layer << '\n';
			return false;
		}
		createInfo.enabledLayerCount = static_cast<uint32_t>(layersToEnable.size());
		createInfo.ppEnabledLayerNames = layersToEnable.data();
		return true;
	}

	bool PickPhysicalDevice(VkPhysicalDevice& out) // pick discrete or last gpu in the list
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			std::cout << "[Renderer] No card with vulkan support available\n";
			return false;
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		for (int i = 0; i < devices.size(); i++)
		{
			const auto& device = devices[i];
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			// assume it has the extensions we need, we try to enable later and fail if not
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || i == devices.size() - 1)
			{
				out = device;
				std::cout << "[Renderer] Picking GPU: " << deviceProperties.deviceName << "\n";
				physicalDeviceToUseProperties = deviceProperties;
				return true;
			}
		}
	}

	bool FindQueueFamily(VkPhysicalDevice device, VkQueueFlagBits queueFlagBits, uint32_t& outQueueFamilyIndex)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
		for (int i = 0; i < queueFamilies.size(); i++)
		{
			const auto& queueFamily = queueFamilies[i];
			if (queueFamily.queueFlags & queueFlagBits)
			{
				outQueueFamilyIndex = i;
				return true;
			}
		}
		return false;
	}

	bool FindPresentQueueFamily(VkPhysicalDevice device, VkSurfaceKHR targetSurface, uint32_t& outQueueFamilyIndex)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
		for (int i = 0; i < queueFamilies.size(); i++)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, targetSurface, &presentSupport);
			if (presentSupport)
			{
				outQueueFamilyIndex = i;
				return true;
			}
		}
		return false;
	}

	bool CreateShaderModule(const std::vector<char>& shaderCode, VkShaderModule& outShaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
		return vkCreateShaderModule(device, &createInfo, nullptr, &outShaderModule) == VK_SUCCESS;
	}

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			std::cout << "[Renderer] Failed to begin recording command buffer";

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChain.GetFramebuffer(imageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain.GetExtent();

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkViewport viewport = swapChain.CreateViewport();
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChain.GetExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			std::cout << "[Renderer] Failed to record command buffer\n";
	}
}

bool sf::Renderer::Initialize(const Window& windowArg)
{
	window = &windowArg;
	std::cout << "[Renderer] Initializing vulkan renderer" << std::endl;

#ifdef SF_DEBUG
	system("python assets/compileShaders.py");
#endif

	// Create instance
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "sf renderer";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "sf";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.flags = 0;
#ifdef SF_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif
		std::vector<const char*> extensions;
		GetRequiredExtensionsForInstance(extensions);
		if (!EnableInstanceExtensions(createInfo, extensions)) return false;
#ifdef SF_DEBUG
		if (!EnableInstanceLayers(createInfo, validationLayers)) return false;
#else
		if (!EnableInstanceLayers(createInfo, {})) return false;
#endif

		VkResult instanceCreationResult = vkCreateInstance(&createInfo, nullptr, &instance);
		if (instanceCreationResult != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create vulkan instance\n";
			return false;
		}
	}

	// Setup debug messenger
#ifdef SF_DEBUG
	{
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to set up debug messenger\n";
			return false;
		}
	}
#endif

	// Create surface
	window->CreateVulkanSurface(instance, surface);

	// Pick gpu
	if (!PickPhysicalDevice(physicalDeviceToUse))
		return false;


	// Create logical device
	{
		// Find graphics queue that can be used to render to our target surface
		if (!FindQueueFamily(physicalDeviceToUse, VK_QUEUE_GRAPHICS_BIT, graphicsQueueFamilyIndex))
		{
			std::cout << "[Renderer] Failed to find graphics queue family\n";
			return false;
		}
		if (!FindPresentQueueFamily(physicalDeviceToUse, surface, presentQueueFamilyIndex))
		{
			std::cout << "[Renderer] Failed to find present queue family\n";
			return false;
		}

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::unordered_set<uint32_t> uniqueQueueFamilies = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };


		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		if (!EnableDeviceExtensions(createInfo, physicalDeviceToUse, deviceExtensions))
		{
			std::cout << "[Renderer] Failed to enable extensions for device\n";
			return false;
		}

#ifdef SF_DEBUG
		if (!EnableDeviceLayers(createInfo, physicalDeviceToUse, validationLayers))
		{
			std::cout << "[Renderer] Failed to enable layers for device\n";
			return false;
		}
#else
		if (!EnableDeviceLayers(createInfo, physicalDeviceToUse, {}))
		{
			std::cout << "[Renderer] Failed to enable layers for device\n";
			return false;
		}
#endif

		VkResult deviceCreationResult = vkCreateDevice(physicalDeviceToUse, &createInfo, nullptr, &device);
		if (deviceCreationResult != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create logical device\n";
			return false;
		}
		vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
		vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue);
	}
	
	// Create swap chain
	swapChain.Create(window, physicalDeviceToUse, device, surface, graphicsQueueFamilyIndex, presentQueueFamilyIndex);
	//CreateSwapChain();

	// Create image views
	//CreateSwapChainImageViews();
	swapChain.CreateImageViews(device);

	// Create render pass
	swapChain.CreateRenderPass(device, renderPass);

	// Create pipeline
	{
		// ---- Shader modules ---- //
		std::string vertexShaderPath = "assets/shaders/vulkan/testV.spv";
		std::string fragmentShaderPath = "assets/shaders/vulkan/testF.spv";
		std::vector<char> vertexShaderCode, fragmentShaderCode;
		if (!FileUtils::ReadFileAsBytes(vertexShaderPath, vertexShaderCode))
		{
			std::cout << "[Renderer] Could not read vertex shader file: " << vertexShaderPath << std::endl;
			return false;
		}
		if (!FileUtils::ReadFileAsBytes(fragmentShaderPath, fragmentShaderCode))
		{
			std::cout << "[Renderer] Could not read fragment shader file: " << fragmentShaderPath << std::endl;
			return false;
		}
		VkShaderModule vertexShaderModule, fragmentShaderModule;
		if (!CreateShaderModule(vertexShaderCode, vertexShaderModule))
		{
			std::cout << "[Renderer] Could not create shader module for shader: " << vertexShaderPath << std::endl;
			return false;
		}
		if (!CreateShaderModule(fragmentShaderCode, fragmentShaderModule))
		{
			std::cout << "[Renderer] Could not create shader module for shader: " << fragmentShaderPath << std::endl;
			return false;
		}

		VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
		vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShaderStageInfo.module = vertexShaderModule;
		vertexShaderStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
		fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShaderStageInfo.module = fragmentShaderModule;
		fragmentShaderStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

		// ---- Fixed functions ---- //
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = swapChain.CreateViewport();

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChain.GetExtent();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = swapChain.GetMsaaSamples();
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create pipeline layout\n";
			return false;
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create pipeline\n";
			return false;
		}

		vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
		vkDestroyShaderModule(device, vertexShaderModule, nullptr);
	}

	swapChain.CreateMultisampleResources(device, physicalDeviceToUse);
	swapChain.CreateFrameBuffers(device, renderPass);

	// Create command pool
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create command pool\n";
			return false;
		}
	}

	// Create command buffers
	{
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create command buffer\n";
			return false;
		}
	}

	// Create sync objects
	{
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			{
				std::cout << "[Renderer] Failed to create sync objects\n";
				return false;
			}
		}
	}

	window->AddOnResizeCallback(OnResize);
	return true;
}

void sf::Renderer::OnResize()
{
	vkDeviceWaitIdle(device);
	swapChain.Recreate(window, physicalDeviceToUse, device, surface, graphicsQueueFamilyIndex, presentQueueFamilyIndex, renderPass);
}

void sf::Renderer::Predraw()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swapChain.Get(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
	RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		std::cout << "[Renderer] Failed to submit draw command buffer" << std::endl;

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain.Get()};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(presentQueue, &presentInfo);
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyDevice(device, nullptr);
#ifdef SF_DEBUG
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void* sf::Renderer::GetWindowSurface()
{
	return &surface;
}

#endif