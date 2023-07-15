#pragma once

#include <vulkan/vulkan.h>
#include <backends/imgui_impl_vulkan.h>

namespace sf::Renderer {

	struct VulkanState
	{
		VkAllocationCallbacks*   allocator = NULL;
		VkInstance               instance = VK_NULL_HANDLE;
		VkPhysicalDevice         physicalDevice = VK_NULL_HANDLE;
		VkDevice                 device = VK_NULL_HANDLE;
		uint32_t                 queueFamily = (uint32_t)-1;
		VkQueue                  queue = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
		VkPipelineCache          pipelineCache = VK_NULL_HANDLE;
		VkDescriptorPool         descriptorPool = VK_NULL_HANDLE;

		ImGui_ImplVulkanH_Window mainWindowData;
		int                      minImageCount = 2;
		bool                     swapChainRebuild = false;
	};
}