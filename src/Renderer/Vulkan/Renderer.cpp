#ifdef SF_USE_VULKAN

#include "../Renderer.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

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

#include "VulkanState.h"

namespace sf::Renderer
{
	float aspectRatio = 1.7777777777;
	Entity activeCameraEntity;
	bool drawSkybox = false;

	glm::mat4 cameraView;
	glm::mat4 cameraProjection;

	VkInstance instance;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;

	//VkPipelineLayout pipelineLayout;
	//VkPipeline pipeline;
	//VkDescriptorSetLayout descriptorSetLayout;
	//VkDescriptorSet descriptorSet;

	//VkSemaphore presentCompleteSemaphore;
	//VkSemaphore renderCompleteSemaphore;

	//std::vector<VkFence> queueCompleteFences;

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
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
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

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR targetSurface)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, targetSurface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, targetSurface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, targetSurface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, targetSurface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, targetSurface, &presentModeCount, details.presentModes.data());
		}

		return details;
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
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			std::cout << "[Renderer] Failed to record command buffer\n";
	}
}

bool sf::Renderer::Initialize(void* process)
{
	std::cout << "[Renderer] Initializing vulkan renderer" << std::endl;

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
	{
		GLFWwindow* window = Config::GetWindow();
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = glfwGetWin32Window(window);
		createInfo.hinstance = GetModuleHandle(nullptr);

		VkResult surfaceCreationResult = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
		if (surfaceCreationResult != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create win32 surface\n";
			return false;
		}

		surfaceCreationResult = glfwCreateWindowSurface(instance, window, nullptr, &surface);
		if (surfaceCreationResult != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create surface on glfw window\n";
			return false;
		}
	}

	// Pick gpu
	VkPhysicalDevice physicalDeviceToUse;
	if (!PickPhysicalDevice(physicalDeviceToUse))
		return false;

	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;
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
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDeviceToUse, surface);
		if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
		{
			std::cout << "[Renderer] No swap chain support for device\n";
			return false;
		}

		// Choose format
		VkSurfaceFormatKHR formatToUse = swapChainSupport.formats[0]; // in case none meet condition below
		for (const auto& availableFormat : swapChainSupport.formats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				formatToUse = availableFormat;
				break;
			}
		}

		// Choose presentation mode
		VkPresentModeKHR presentModeToUse = VK_PRESENT_MODE_FIFO_KHR;

		// Choose swap extent
		VkExtent2D swapExtentToUse;
		uint32_t maxint = (std::numeric_limits<uint32_t>::max)();
		if (swapChainSupport.capabilities.currentExtent.width != maxint)
			swapExtentToUse = swapChainSupport.capabilities.currentExtent;
		else
		{
			int width, height;
			glfwGetFramebufferSize(Config::GetWindow(), &width, &height);
			swapExtentToUse = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
			swapExtentToUse.width = std::clamp(swapExtentToUse.width, swapChainSupport.capabilities.minImageExtent.width, swapChainSupport.capabilities.maxImageExtent.width);
			swapExtentToUse.height = std::clamp(swapExtentToUse.height, swapChainSupport.capabilities.minImageExtent.height, swapChainSupport.capabilities.maxImageExtent.height);
		}

		// Choose image count
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = formatToUse.format;
		createInfo.imageColorSpace = formatToUse.colorSpace;
		createInfo.imageExtent = swapExtentToUse;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
		if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentModeToUse;
		createInfo.clipped = VK_TRUE; // don't care about pixels behind other windows
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkResult swapchainCreationResult = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
		if (swapchainCreationResult != VK_SUCCESS)
		{
			// can fail if glfwCreateWindowSurface is called multiple times
			std::cout << "[Renderer] Failed to create swap chain\n";
			return false;
		}
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
		swapChainImageFormat = formatToUse.format;
		swapChainExtent = swapExtentToUse;
	}

	// Create image views
	{
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VkResult imageViewCreationResult = vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]);
			if (imageViewCreationResult != VK_SUCCESS)
			{
				std::cout << "[Renderer] Failed to create image view\n";
				return false;
			}
		}
	}

	// Create render pass
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create render pass\n";
			return false;
		}
	}

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

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

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
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
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

	// Create framebuffers
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
			{
				std::cout << "[Renderer] Failed to create framebuffer\n";
				return false;
			}
		}
	}

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

	// Create command buffer
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create command buffer\n";
			return false;
		}
	}

	// Create sync objects
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
		{
			std::cout << "[Renderer] Failed to create sync objects\n";
			return false;
		}
	}

	return true;
}

void sf::Renderer::OnResize()
{

}

void sf::Renderer::Predraw()
{
	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFence);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
	RecordCommandBuffer(commandBuffer, imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS)
		std::cout << "[Renderer] Failed to submit draw command buffer" << std::endl;

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(presentQueue, &presentInfo);
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
	vkDestroyCommandPool(device, commandPool, nullptr);
	for (auto framebuffer : swapChainFramebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	for (auto imageView : swapChainImageViews)
		vkDestroyImageView(device, imageView, nullptr);
	vkDestroySwapchainKHR(device, swapChain, nullptr);
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